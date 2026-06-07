/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#include "IISUsageGraphImporter.h"

#include "IISEmbeddingJobQueue.h"
#include "IISChunkCatalog.h"
#include "IISStoragePaths.h"
#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "SQLiteDatabase.h"
#include "SQLitePreparedStatement.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace IISUsageGraph
{
	static FString NormalizePath(const FString& Path)
	{
		if (Path.IsEmpty())
		{
			return FString();
		}
		FString Normalized = FPaths::ConvertRelativePathToFull(Path);
		FPaths::NormalizeFilename(Normalized);
		return Normalized;
	}

	static FString DefaultEvidenceOutputRoot()
	{
		return NormalizePath(FPaths::ProjectSavedDir() / TEXT("InternalIndexService") / TEXT("evidence"));
	}

	static bool LoadJsonFile(const FString& FilePath, TSharedPtr<FJsonObject>& OutObject)
	{
		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *FilePath))
		{
			return false;
		}
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		return FJsonSerializer::Deserialize(Reader, OutObject) && OutObject.IsValid();
	}

	static FString GetStringField(const TSharedPtr<FJsonObject>& Object, const FString& FieldName)
	{
		FString Value;
		if (Object.IsValid())
		{
			Object->TryGetStringField(FieldName, Value);
		}
		return Value;
	}

	static int32 GetIntField(const TSharedPtr<FJsonObject>& Object, const FString& FieldName)
	{
		double Value = 0.0;
		if (Object.IsValid() && Object->TryGetNumberField(FieldName, Value))
		{
			return static_cast<int32>(Value);
		}
		return 0;
	}

	static FIISSourceReference MakeSourceRef(
		const FString& RelativePath,
		const FString& JsonPointer,
		int32 LineNumber = 0)
	{
		FIISSourceReference Ref;
		Ref.RelativePath = RelativePath;
		Ref.JsonPointer = JsonPointer;
		if (LineNumber > 0)
		{
			Ref.Fingerprint = FString::Printf(TEXT("line:%d"), LineNumber);
		}
		return Ref;
	}

	static EIISUsageKind ParseUsageKindToken(const FString& Token)
	{
		const FString Lower = Token.ToLower();
		if (Lower == TEXT("declaration"))
		{
			return EIISUsageKind::Declaration;
		}
		if (Lower == TEXT("reference"))
		{
			return EIISUsageKind::Reference;
		}
		if (Lower == TEXT("call"))
		{
			return EIISUsageKind::Call;
		}
		return EIISUsageKind::Unknown;
	}

	static FString UsageKindToDbString(const EIISUsageKind Kind)
	{
		switch (Kind)
		{
		case EIISUsageKind::Declaration:
			return TEXT("declaration");
		case EIISUsageKind::Reference:
			return TEXT("reference");
		case EIISUsageKind::Call:
			return TEXT("call");
		default:
			return TEXT("unknown");
		}
	}

	static EIISUsageKind UsageKindFromDbString(const FString& Value)
	{
		return ParseUsageKindToken(Value);
	}

	static bool ExecuteStatement(FSQLiteDatabase& Database, const TCHAR* Statement, TArray<FString>& OutErrors)
	{
		if (!Database.Execute(Statement))
		{
			OutErrors.Add(FString::Printf(TEXT("SQLite error: %s"), *Database.GetLastError()));
			return false;
		}
		return true;
	}

	static bool OpenCatalogDatabase(FSQLiteDatabase& Database, TArray<FString>& OutErrors)
	{
		const FString CatalogPath = FIISChunkCatalog::GetCatalogPath();
		IFileManager::Get().MakeDirectory(*FPaths::GetPath(CatalogPath), true);
		if (!Database.Open(*CatalogPath, ESQLiteDatabaseOpenMode::ReadWriteCreate))
		{
			OutErrors.Add(FString::Printf(TEXT("Failed to open IIS catalog: %s"), *Database.GetLastError()));
			return false;
		}
		return true;
	}

	static bool EnsureUsageGraphSchema(FSQLiteDatabase& Database, TArray<FString>& OutErrors)
	{
		bool bOk = true;
		bOk &= ExecuteStatement(Database,
			TEXT("CREATE TABLE IF NOT EXISTS symbols (")
			TEXT("symbol_id TEXT PRIMARY KEY,")
			TEXT("name TEXT,")
			TEXT("qualified_name TEXT,")
			TEXT("kind TEXT,")
			TEXT("module_name TEXT,")
			TEXT("lifecycle_state TEXT DEFAULT 'active'")
			TEXT(");"),
			OutErrors);
		bOk &= ExecuteStatement(Database,
			TEXT("CREATE TABLE IF NOT EXISTS usages (")
			TEXT("symbol_id TEXT,")
			TEXT("usage_kind TEXT,")
			TEXT("rel_path TEXT,")
			TEXT("json_pointer TEXT,")
			TEXT("fingerprint TEXT")
			TEXT(");"),
			OutErrors);
		bOk &= ExecuteStatement(Database,
			TEXT("CREATE TABLE IF NOT EXISTS call_edges (")
			TEXT("caller_id TEXT,")
			TEXT("callee_id TEXT,")
			TEXT("rel_path TEXT,")
			TEXT("json_pointer TEXT")
			TEXT(");"),
			OutErrors);
		bOk &= ExecuteStatement(Database,
			TEXT("CREATE TABLE IF NOT EXISTS asset_refs (")
			TEXT("asset_path TEXT,")
			TEXT("referencing_symbol_id TEXT,")
			TEXT("ref_kind TEXT")
			TEXT(");"),
			OutErrors);
		bOk &= ExecuteStatement(Database,
			TEXT("CREATE TABLE IF NOT EXISTS blueprint_refs (")
			TEXT("blueprint_name TEXT,")
			TEXT("graph_or_node_ref TEXT,")
			TEXT("referenced_symbol_id TEXT")
			TEXT(");"),
			OutErrors);
		bOk &= ExecuteStatement(Database, TEXT("CREATE INDEX IF NOT EXISTS idx_symbols_name ON symbols(name);"), OutErrors);
		bOk &= ExecuteStatement(Database, TEXT("CREATE INDEX IF NOT EXISTS idx_symbols_qualified_name ON symbols(qualified_name);"), OutErrors);
		return bOk;
	}

	static bool TrySetEvidenceRootFromProbe(const FString& CandidateRoot, FResolvedUsageEvidencePaths& OutPaths)
	{
		const FString NormalizedRoot = NormalizePath(CandidateRoot);
		const FString CppIndex = FPaths::Combine(NormalizedRoot, TEXT("cpp"), TEXT("cpp_symbol_index.json"));
		if (!FPaths::FileExists(CppIndex))
		{
			return false;
		}

		OutPaths.EvidenceRoot = NormalizedRoot;
		OutPaths.CppSymbolIndexPath = CppIndex;
		OutPaths.ReflectionSurfaceIndexPath =
			FPaths::Combine(NormalizedRoot, TEXT("reflection"), TEXT("reflection_surface_index.json"));
		OutPaths.NetworkSurfaceIndexPath =
			FPaths::Combine(NormalizedRoot, TEXT("network"), TEXT("network_surface_index.json"));
		OutPaths.AssetRegistryExportPath =
			FPaths::Combine(NormalizedRoot, TEXT("assets"), TEXT("asset_registry_export.json"));
		OutPaths.ModuleGraphPath =
			FPaths::Combine(NormalizedRoot, TEXT("modules"), TEXT("module_graph.json"));
		OutPaths.BlueprintsDirectory = FPaths::Combine(NormalizedRoot, TEXT("blueprints"));
		return true;
	}

	static void CollectEvidenceRootCandidates(const FString& ContractPath, TArray<FString>& OutCandidates)
	{
		const FString ContractDir = FPaths::GetPath(NormalizePath(ContractPath));
		OutCandidates.Add(DefaultEvidenceOutputRoot());

		FString WalkDir = ContractDir;
		for (int32 Depth = 0; Depth < 8; ++Depth)
		{
			OutCandidates.AddUnique(WalkDir);
			OutCandidates.AddUnique(FPaths::Combine(WalkDir, TEXT("evidence")));
			const FString Parent = FPaths::GetPath(WalkDir);
			if (Parent == WalkDir)
			{
				break;
			}
			WalkDir = Parent;
		}

		const FString RunsDir = FPaths::Combine(DefaultEvidenceOutputRoot(), TEXT("runs"));
		TArray<FString> RunDirs;
		IFileManager::Get().FindFiles(RunDirs, *(RunsDir / TEXT("*")), false, true);
		RunDirs.Sort();
		for (int32 Index = RunDirs.Num() - 1; Index >= 0; --Index)
		{
			OutCandidates.Add(FPaths::Combine(RunsDir, RunDirs[Index]));
		}
	}

	static bool QueryMatches(const FString& Haystack, const FString& Needle)
	{
		return !Haystack.IsEmpty() && Haystack.Contains(Needle, ESearchCase::IgnoreCase, ESearchDir::FromStart);
	}

	static bool PersistGraphPayload(
		const TArray<FIISSymbolRecord>& Symbols,
		const TArray<FIISUsageRecord>& Usages,
		const TArray<FIISCallEdge>& CallEdges,
		const TArray<FIISAssetReference>& AssetRefs,
		const TArray<FIISBlueprintReference>& BlueprintRefs,
		TArray<FString>& OutWarnings)
	{
		TArray<FString> Errors;
		FSQLiteDatabase Database;
		if (!OpenCatalogDatabase(Database, Errors))
		{
			OutWarnings.Append(Errors);
			return false;
		}
		if (!EnsureUsageGraphSchema(Database, Errors))
		{
			OutWarnings.Append(Errors);
			Database.Close();
			return false;
		}

		if (!Database.Execute(TEXT("BEGIN IMMEDIATE;")))
		{
			OutWarnings.Add(FString::Printf(TEXT("Usage graph import transaction failed: %s"), *Database.GetLastError()));
			Database.Close();
			return false;
		}

		auto Rollback = [&Database, &OutWarnings]()
		{
			Database.Execute(TEXT("ROLLBACK;"));
			OutWarnings.Add(TEXT("Usage graph import rolled back due to errors."));
		};

		if (!ExecuteStatement(Database, TEXT("DELETE FROM usages;"), Errors)
			|| !ExecuteStatement(Database, TEXT("DELETE FROM call_edges;"), Errors)
			|| !ExecuteStatement(Database, TEXT("DELETE FROM asset_refs;"), Errors)
			|| !ExecuteStatement(Database, TEXT("DELETE FROM blueprint_refs;"), Errors))
		{
			Rollback();
			OutWarnings.Append(Errors);
			Database.Close();
			return false;
		}

		TSet<FString> SeenSymbolIds;
		for (const FIISSymbolRecord& Symbol : Symbols)
		{
			if (Symbol.SymbolId.IsEmpty())
			{
				continue;
			}
			SeenSymbolIds.Add(Symbol.SymbolId);

			FString SymbolLifecycle = Symbol.LifecycleState;
			if (SymbolLifecycle.IsEmpty())
			{
				SymbolLifecycle = TEXT("active");
			}

			FSQLitePreparedStatement Statement = Database.PrepareStatement(
				TEXT("INSERT INTO symbols(symbol_id, name, qualified_name, kind, module_name, lifecycle_state) ")
				TEXT("VALUES(?, ?, ?, ?, ?, ?) ")
				TEXT("ON CONFLICT(symbol_id) DO UPDATE SET ")
				TEXT("name=excluded.name, qualified_name=excluded.qualified_name, kind=excluded.kind, ")
				TEXT("module_name=excluded.module_name, lifecycle_state=excluded.lifecycle_state;"),
				ESQLitePreparedStatementFlags::Persistent);
			if (!Statement.IsValid())
			{
				Errors.Add(Database.GetLastError());
				Rollback();
				OutWarnings.Append(Errors);
				Database.Close();
				return false;
			}

			Statement.SetBindingValueByIndex(1, Symbol.SymbolId);
			Statement.SetBindingValueByIndex(2, Symbol.Name);
			Statement.SetBindingValueByIndex(3, Symbol.QualifiedName);
			Statement.SetBindingValueByIndex(4, Symbol.Kind);
			Statement.SetBindingValueByIndex(5, Symbol.ModuleName);
			Statement.SetBindingValueByIndex(6, SymbolLifecycle);
			Statement.Execute();
			Statement.Reset();
		}

		for (const FIISUsageRecord& Usage : Usages)
		{
			FSQLitePreparedStatement Statement = Database.PrepareStatement(
				TEXT("INSERT INTO usages(symbol_id, usage_kind, rel_path, json_pointer, fingerprint) VALUES(?, ?, ?, ?, ?);"),
				ESQLitePreparedStatementFlags::Persistent);
			if (!Statement.IsValid())
			{
				continue;
			}
			Statement.SetBindingValueByIndex(1, Usage.SymbolId);
			Statement.SetBindingValueByIndex(2, UsageKindToDbString(Usage.UsageKind));
			Statement.SetBindingValueByIndex(3, Usage.Location.RelativePath);
			Statement.SetBindingValueByIndex(4, Usage.Location.JsonPointer);
			Statement.SetBindingValueByIndex(5, Usage.Location.Fingerprint);
			Statement.Execute();
			Statement.Reset();
		}

		for (const FIISCallEdge& Edge : CallEdges)
		{
			FSQLitePreparedStatement Statement = Database.PrepareStatement(
				TEXT("INSERT INTO call_edges(caller_id, callee_id, rel_path, json_pointer) VALUES(?, ?, ?, ?);"),
				ESQLitePreparedStatementFlags::Persistent);
			if (!Statement.IsValid())
			{
				continue;
			}
			Statement.SetBindingValueByIndex(1, Edge.CallerSymbolId);
			Statement.SetBindingValueByIndex(2, Edge.CalleeSymbolId);
			Statement.SetBindingValueByIndex(3, Edge.Evidence.RelativePath);
			Statement.SetBindingValueByIndex(4, Edge.Evidence.JsonPointer);
			Statement.Execute();
			Statement.Reset();
		}

		for (const FIISAssetReference& AssetRef : AssetRefs)
		{
			FSQLitePreparedStatement Statement = Database.PrepareStatement(
				TEXT("INSERT INTO asset_refs(asset_path, referencing_symbol_id, ref_kind) VALUES(?, ?, ?);"),
				ESQLitePreparedStatementFlags::Persistent);
			if (!Statement.IsValid())
			{
				continue;
			}
			Statement.SetBindingValueByIndex(1, AssetRef.AssetPath);
			Statement.SetBindingValueByIndex(2, AssetRef.ReferencingSymbolId);
			Statement.SetBindingValueByIndex(3, AssetRef.RefKind);
			Statement.Execute();
			Statement.Reset();
		}

		for (const FIISBlueprintReference& BlueprintRef : BlueprintRefs)
		{
			FSQLitePreparedStatement Statement = Database.PrepareStatement(
				TEXT("INSERT INTO blueprint_refs(blueprint_name, graph_or_node_ref, referenced_symbol_id) VALUES(?, ?, ?);"),
				ESQLitePreparedStatementFlags::Persistent);
			if (!Statement.IsValid())
			{
				continue;
			}
			Statement.SetBindingValueByIndex(1, BlueprintRef.BlueprintName);
			Statement.SetBindingValueByIndex(2, BlueprintRef.GraphOrNodeRef);
			Statement.SetBindingValueByIndex(3, BlueprintRef.ReferencedSymbolId);
			Statement.Execute();
			Statement.Reset();
		}

		if (SeenSymbolIds.Num() > 0)
		{
			FString InClause;
			for (const FString& SymbolId : SeenSymbolIds)
			{
				if (!InClause.IsEmpty())
				{
					InClause += TEXT(",");
				}
				InClause += FString::Printf(TEXT("'%s'"), *SymbolId.Replace(TEXT("'"), TEXT("''")));
			}
			const FString StaleSql = FString::Printf(
				TEXT("UPDATE symbols SET lifecycle_state='stale' WHERE symbol_id NOT IN (%s) ")
				TEXT("AND (lifecycle_state IS NULL OR lifecycle_state = '' OR lifecycle_state = 'active');"),
				*InClause);
			if (!Database.Execute(*StaleSql))
			{
				Errors.Add(Database.GetLastError());
			}
		}

		if (!Database.Execute(TEXT("COMMIT;")))
		{
			Rollback();
			OutWarnings.Add(FString::Printf(TEXT("Usage graph import commit failed: %s"), *Database.GetLastError()));
			Database.Close();
			return false;
		}

		Database.Close();
		if (Errors.Num() > 0)
		{
			OutWarnings.Append(Errors);
		}
		return true;
	}
}

FString FIISUsageGraphImporter::DeriveSymbolId(
	const FString& QualifiedName,
	const FString& Name,
	const FString& ModuleName,
	int32 LineNumber)
{
	if (!QualifiedName.IsEmpty())
	{
		return FIISEmbeddingJobQueue::ComputeSHA256(QualifiedName);
	}
	const FString Fallback = FString::Printf(TEXT("%s|%s|%d"), *Name, *ModuleName, LineNumber);
	return FIISEmbeddingJobQueue::ComputeSHA256(Fallback);
}

bool FIISUsageGraphImporter::ResolveEvidencePathsFromHandoffContract(
	const FString& HandoffContractPath,
	FResolvedUsageEvidencePaths& OutPaths,
	TArray<FString>& OutWarnings)
{
	OutPaths = FResolvedUsageEvidencePaths();
	TSharedPtr<FJsonObject> ContractObject;
	if (IISUsageGraph::LoadJsonFile(IISUsageGraph::NormalizePath(HandoffContractPath), ContractObject))
	{
		const FString ContractDir = FPaths::GetPath(IISUsageGraph::NormalizePath(HandoffContractPath));
		auto ResolveRelative = [&ContractDir](const FString& PathValue) -> FString
		{
			if (PathValue.IsEmpty())
			{
				return FString();
			}
			return FPaths::IsRelative(PathValue)
				? IISUsageGraph::NormalizePath(FPaths::Combine(ContractDir, PathValue))
				: IISUsageGraph::NormalizePath(PathValue);
		};

		const FString RagPackagePath = ResolveRelative(IISUsageGraph::GetStringField(ContractObject, TEXT("rag_export_package_path")));
		if (!RagPackagePath.IsEmpty())
		{
			TSharedPtr<FJsonObject> RagObject;
			if (IISUsageGraph::LoadJsonFile(RagPackagePath, RagObject))
			{
				const FString RagPackageDir = FPaths::GetPath(RagPackagePath);
				const TArray<TSharedPtr<FJsonValue>>* SourceRefs = nullptr;
				if (RagObject->TryGetArrayField(TEXT("source_references"), SourceRefs))
				{
					for (const TSharedPtr<FJsonValue>& Value : *SourceRefs)
					{
						const TSharedPtr<FJsonObject>* RefObject = nullptr;
						if (!Value.IsValid() || !Value->TryGetObject(RefObject) || !RefObject)
						{
							continue;
						}
						const FString RelativePath = IISUsageGraph::GetStringField(*RefObject, TEXT("relative_path"));
						if (RelativePath.EndsWith(TEXT("cpp/cpp_symbol_index.json")))
						{
							const FString FullPath = IISUsageGraph::NormalizePath(FPaths::Combine(RagPackageDir, RelativePath));
							const FString Root = FPaths::GetPath(FPaths::GetPath(FullPath));
							IISUsageGraph::TrySetEvidenceRootFromProbe(Root, OutPaths);
							break;
						}
					}
				}
			}
		}
	}

	if (OutPaths.EvidenceRoot.IsEmpty())
	{
		TArray<FString> Candidates;
		IISUsageGraph::CollectEvidenceRootCandidates(HandoffContractPath, Candidates);
		for (const FString& Candidate : Candidates)
		{
			if (IISUsageGraph::TrySetEvidenceRootFromProbe(Candidate, OutPaths))
			{
				break;
			}
		}
	}

	if (OutPaths.EvidenceRoot.IsEmpty())
	{
		OutWarnings.Add(TEXT("Could not resolve evidence root for usage graph import."));
		return false;
	}

	OutWarnings.Add(FString::Printf(TEXT("Usage graph evidence root: %s"), *OutPaths.EvidenceRoot));
	return true;
}

bool FIISUsageGraphImporter::ParseCppSymbolIndexJson(
	const FString& Json,
	TArray<FIISSymbolRecord>& OutSymbols,
	TArray<FIISUsageRecord>& OutUsages,
	TArray<FString>& OutWarnings)
{
	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		OutWarnings.Add(TEXT("cpp_symbol_index JSON parse failed."));
		return false;
	}

	const TArray<TSharedPtr<FJsonValue>>* Files = nullptr;
	if (!Root->TryGetArrayField(TEXT("files"), Files))
	{
		OutWarnings.Add(TEXT("cpp_symbol_index missing files[] array."));
		return false;
	}

	for (int32 FileIndex = 0; FileIndex < Files->Num(); ++FileIndex)
	{
		const TSharedPtr<FJsonObject>* FileObject = nullptr;
		if (!(*Files)[FileIndex].IsValid() || !(*Files)[FileIndex]->TryGetObject(FileObject) || !FileObject)
		{
			continue;
		}

		const FString RelativePath = IISUsageGraph::GetStringField(*FileObject, TEXT("relative_path"));
		const FString ModuleName = IISUsageGraph::GetStringField(*FileObject, TEXT("module_name"));
		const TArray<TSharedPtr<FJsonValue>>* Symbols = nullptr;
		if (!(*FileObject)->TryGetArrayField(TEXT("symbols"), Symbols))
		{
			continue;
		}

		for (int32 SymbolIndex = 0; SymbolIndex < Symbols->Num(); ++SymbolIndex)
		{
			const TSharedPtr<FJsonObject>* SymbolObject = nullptr;
			if (!(*Symbols)[SymbolIndex].IsValid() || !(*Symbols)[SymbolIndex]->TryGetObject(SymbolObject) || !SymbolObject)
			{
				continue;
			}

			const FString Name = IISUsageGraph::GetStringField(*SymbolObject, TEXT("name"));
			const FString QualifiedName = IISUsageGraph::GetStringField(*SymbolObject, TEXT("qualified_name"));
			const FString Kind = IISUsageGraph::GetStringField(*SymbolObject, TEXT("symbol_kind"));
			const int32 LineNumber = IISUsageGraph::GetIntField(*SymbolObject, TEXT("line_number"));
			const FString SymbolId = DeriveSymbolId(QualifiedName, Name, ModuleName, LineNumber);

			FIISSymbolRecord Symbol;
			Symbol.SymbolId = SymbolId;
			Symbol.Name = Name;
			Symbol.QualifiedName = QualifiedName;
			Symbol.Kind = Kind;
			Symbol.ModuleName = ModuleName;
			Symbol.SourceReferences.Add(IISUsageGraph::MakeSourceRef(
				RelativePath,
				FString::Printf(TEXT("/files/%d/symbols/%d"), FileIndex, SymbolIndex),
				LineNumber));
			OutSymbols.Add(Symbol);

			FIISUsageRecord Usage;
			Usage.SymbolId = SymbolId;
			Usage.UsageKind = EIISUsageKind::Declaration;
			Usage.Location = Symbol.SourceReferences[0];
			OutUsages.Add(Usage);
		}
	}

	return OutSymbols.Num() > 0;
}

bool FIISUsageGraphImporter::ParseReflectionSurfaceIndexJson(
	const FString& Json,
	TArray<FIISSymbolRecord>& OutSymbols,
	TArray<FIISUsageRecord>& OutUsages,
	TArray<FString>& OutWarnings)
{
	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		OutWarnings.Add(TEXT("reflection_surface_index JSON parse failed."));
		return false;
	}

	const TArray<TSharedPtr<FJsonValue>>* Types = nullptr;
	if (!Root->TryGetArrayField(TEXT("types"), Types))
	{
		OutWarnings.Add(TEXT("reflection_surface_index missing types[] array."));
		return false;
	}

	for (int32 TypeIndex = 0; TypeIndex < Types->Num(); ++TypeIndex)
	{
		const TSharedPtr<FJsonObject>* TypeObject = nullptr;
		if (!(*Types)[TypeIndex].IsValid() || !(*Types)[TypeIndex]->TryGetObject(TypeObject) || !TypeObject)
		{
			continue;
		}

		const FString TypeName = IISUsageGraph::GetStringField(*TypeObject, TEXT("name"));
		const FString QualifiedName = IISUsageGraph::GetStringField(*TypeObject, TEXT("qualified_name"));
		const FString ModuleName = IISUsageGraph::GetStringField(*TypeObject, TEXT("module_name"));
		const FString RelativePath = IISUsageGraph::GetStringField(*TypeObject, TEXT("relative_path"));
		const int32 TypeLine = IISUsageGraph::GetIntField(*TypeObject, TEXT("line_number"));
		const FString TypeSymbolId = DeriveSymbolId(QualifiedName, TypeName, ModuleName, TypeLine);

		FIISSymbolRecord TypeSymbol;
		TypeSymbol.SymbolId = TypeSymbolId;
		TypeSymbol.Name = TypeName;
		TypeSymbol.QualifiedName = QualifiedName;
		TypeSymbol.Kind = TEXT("type");
		TypeSymbol.ModuleName = ModuleName;
		TypeSymbol.SourceReferences.Add(IISUsageGraph::MakeSourceRef(
			RelativePath,
			FString::Printf(TEXT("/types/%d"), TypeIndex),
			TypeLine));
		OutSymbols.Add(TypeSymbol);

		FIISUsageRecord TypeUsage;
		TypeUsage.SymbolId = TypeSymbolId;
		TypeUsage.UsageKind = EIISUsageKind::Declaration;
		TypeUsage.Location = TypeSymbol.SourceReferences[0];
		OutUsages.Add(TypeUsage);

		const TArray<TSharedPtr<FJsonValue>>* Functions = nullptr;
		if (!(*TypeObject)->TryGetArrayField(TEXT("functions"), Functions))
		{
			continue;
		}

		for (int32 FunctionIndex = 0; FunctionIndex < Functions->Num(); ++FunctionIndex)
		{
			const TSharedPtr<FJsonObject>* FunctionObject = nullptr;
			if (!(*Functions)[FunctionIndex].IsValid()
				|| !(*Functions)[FunctionIndex]->TryGetObject(FunctionObject)
				|| !FunctionObject)
			{
				continue;
			}

			const FString FunctionName = IISUsageGraph::GetStringField(*FunctionObject, TEXT("name"));
			const int32 FunctionLine = IISUsageGraph::GetIntField(*FunctionObject, TEXT("line_number"));
			const FString FunctionQualified = QualifiedName.IsEmpty()
				? FunctionName
				: FString::Printf(TEXT("%s::%s"), *QualifiedName, *FunctionName);
			const FString FunctionSymbolId = DeriveSymbolId(FunctionQualified, FunctionName, ModuleName, FunctionLine);

			FIISSymbolRecord FunctionSymbol;
			FunctionSymbol.SymbolId = FunctionSymbolId;
			FunctionSymbol.Name = FunctionName;
			FunctionSymbol.QualifiedName = FunctionQualified;
			FunctionSymbol.Kind = TEXT("function");
			FunctionSymbol.ModuleName = ModuleName;
			FunctionSymbol.SourceReferences.Add(IISUsageGraph::MakeSourceRef(
				RelativePath,
				FString::Printf(TEXT("/types/%d/functions/%d"), TypeIndex, FunctionIndex),
				FunctionLine));
			OutSymbols.Add(FunctionSymbol);

			FIISUsageRecord FunctionUsage;
			FunctionUsage.SymbolId = FunctionSymbolId;
			FunctionUsage.UsageKind = EIISUsageKind::Declaration;
			FunctionUsage.Location = FunctionSymbol.SourceReferences[0];
			OutUsages.Add(FunctionUsage);
		}
	}

	return OutSymbols.Num() > 0;
}

bool FIISUsageGraphImporter::ParseNetworkSurfaceIndexJson(
	const FString& Json,
	TArray<FIISSymbolRecord>& OutSymbols,
	TArray<FIISCallEdge>& OutCallEdges,
	TArray<FString>& OutWarnings)
{
	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		OutWarnings.Add(TEXT("network_surface_index JSON parse failed."));
		return false;
	}

	const TArray<TSharedPtr<FJsonValue>>* Rpcs = nullptr;
	if (!Root->TryGetArrayField(TEXT("rpcs"), Rpcs))
	{
		OutWarnings.Add(TEXT("network_surface_index missing rpcs[] array."));
		return false;
	}

	for (int32 RpcIndex = 0; RpcIndex < Rpcs->Num(); ++RpcIndex)
	{
		const TSharedPtr<FJsonObject>* RpcObject = nullptr;
		if (!(*Rpcs)[RpcIndex].IsValid() || !(*Rpcs)[RpcIndex]->TryGetObject(RpcObject) || !RpcObject)
		{
			continue;
		}

		const FString OwnerTypeName = IISUsageGraph::GetStringField(*RpcObject, TEXT("owner_type_name"));
		const FString FunctionName = IISUsageGraph::GetStringField(*RpcObject, TEXT("function_name"));
		const TSharedPtr<FJsonObject>* SourceLocationObject = nullptr;
		FString RelativePath;
		FString ModuleName;
		int32 LineNumber = 0;
		if ((*RpcObject)->TryGetObjectField(TEXT("source_location"), SourceLocationObject) && SourceLocationObject)
		{
			RelativePath = IISUsageGraph::GetStringField(*SourceLocationObject, TEXT("relative_path"));
			ModuleName = IISUsageGraph::GetStringField(*SourceLocationObject, TEXT("module_name"));
			LineNumber = IISUsageGraph::GetIntField(*SourceLocationObject, TEXT("line_number"));
		}

		const FString CallerQualified = OwnerTypeName;
		const FString CalleeQualified = FString::Printf(TEXT("%s::%s"), *OwnerTypeName, *FunctionName);
		const FString CallerSymbolId = DeriveSymbolId(CallerQualified, OwnerTypeName, ModuleName, 0);
		const FString CalleeSymbolId = DeriveSymbolId(CalleeQualified, FunctionName, ModuleName, LineNumber);

		FIISSymbolRecord OwnerSymbol;
		OwnerSymbol.SymbolId = CallerSymbolId;
		OwnerSymbol.Name = OwnerTypeName;
		OwnerSymbol.QualifiedName = CallerQualified;
		OwnerSymbol.Kind = TEXT("type");
		OwnerSymbol.ModuleName = ModuleName;
		OutSymbols.Add(OwnerSymbol);

		FIISSymbolRecord RpcSymbol;
		RpcSymbol.SymbolId = CalleeSymbolId;
		RpcSymbol.Name = FunctionName;
		RpcSymbol.QualifiedName = CalleeQualified;
		RpcSymbol.Kind = TEXT("rpc");
		RpcSymbol.ModuleName = ModuleName;
		OutSymbols.Add(RpcSymbol);

		FIISCallEdge Edge;
		Edge.CallerSymbolId = CallerSymbolId;
		Edge.CalleeSymbolId = CalleeSymbolId;
		Edge.Evidence = IISUsageGraph::MakeSourceRef(
			RelativePath,
			FString::Printf(TEXT("/rpcs/%d"), RpcIndex),
			LineNumber);
		OutCallEdges.Add(Edge);
	}

	return OutCallEdges.Num() > 0;
}

bool FIISUsageGraphImporter::ParseAssetRegistryExportJson(
	const FString& Json,
	TArray<FIISAssetReference>& OutAssetRefs,
	TArray<FString>& OutWarnings)
{
	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		OutWarnings.Add(TEXT("asset_registry_export JSON parse failed."));
		return false;
	}

	const TArray<TSharedPtr<FJsonValue>>* Assets = nullptr;
	if (!Root->TryGetArrayField(TEXT("assets"), Assets))
	{
		OutWarnings.Add(TEXT("asset_registry_export missing assets[] array."));
		return false;
	}

	for (int32 AssetIndex = 0; AssetIndex < Assets->Num(); ++AssetIndex)
	{
		const TSharedPtr<FJsonObject>* AssetObject = nullptr;
		if (!(*Assets)[AssetIndex].IsValid() || !(*Assets)[AssetIndex]->TryGetObject(AssetObject) || !AssetObject)
		{
			continue;
		}

		const FString ObjectPath = IISUsageGraph::GetStringField(*AssetObject, TEXT("object_path"));
		const TArray<TSharedPtr<FJsonValue>>* Referencers = nullptr;
		if (!(*AssetObject)->TryGetArrayField(TEXT("referencers"), Referencers))
		{
			continue;
		}

		for (int32 ReferencerIndex = 0; ReferencerIndex < Referencers->Num(); ++ReferencerIndex)
		{
			const TSharedPtr<FJsonObject>* ReferencerObject = nullptr;
			if (!(*Referencers)[ReferencerIndex].IsValid()
				|| !(*Referencers)[ReferencerIndex]->TryGetObject(ReferencerObject)
				|| !ReferencerObject)
			{
				continue;
			}

			const FString PackagePath = IISUsageGraph::GetStringField(*ReferencerObject, TEXT("package_path"));
			const FString PackageName = IISUsageGraph::GetStringField(*ReferencerObject, TEXT("package_name"));
			const FString ReferencerKey = !PackagePath.IsEmpty() ? PackagePath : PackageName;
			if (ReferencerKey.IsEmpty())
			{
				continue;
			}

			FIISAssetReference AssetRef;
			AssetRef.AssetPath = ObjectPath;
			AssetRef.ReferencingSymbolId = DeriveSymbolId(ReferencerKey, PackageName, TEXT(""), 0);
			AssetRef.RefKind = TEXT("referencer");
			OutAssetRefs.Add(AssetRef);
		}
	}

	return OutAssetRefs.Num() > 0;
}

bool FIISUsageGraphImporter::ParseBlueprintIrJson(
	const FString& Json,
	const FString& BlueprintName,
	TArray<FIISBlueprintReference>& OutBlueprintRefs,
	TArray<FString>& OutWarnings)
{
	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		OutWarnings.Add(TEXT("blueprint_ir JSON parse failed."));
		return false;
	}

	const FString EffectiveBlueprintName = BlueprintName.IsEmpty()
		? IISUsageGraph::GetStringField(Root, TEXT("blueprint_name"))
		: BlueprintName;

	const TArray<TSharedPtr<FJsonValue>>* Graphs = nullptr;
	if (!Root->TryGetArrayField(TEXT("graphs"), Graphs))
	{
		OutWarnings.Add(TEXT("blueprint_ir missing graphs[] array."));
		return false;
	}

	for (int32 GraphIndex = 0; GraphIndex < Graphs->Num(); ++GraphIndex)
	{
		const TSharedPtr<FJsonObject>* GraphObject = nullptr;
		if (!(*Graphs)[GraphIndex].IsValid() || !(*Graphs)[GraphIndex]->TryGetObject(GraphObject) || !GraphObject)
		{
			continue;
		}

		const FString GraphName = IISUsageGraph::GetStringField(*GraphObject, TEXT("graph_name"));
		const TArray<TSharedPtr<FJsonValue>>* Nodes = nullptr;
		if (!(*GraphObject)->TryGetArrayField(TEXT("nodes"), Nodes))
		{
			continue;
		}

		for (int32 NodeIndex = 0; NodeIndex < Nodes->Num(); ++NodeIndex)
		{
			const TSharedPtr<FJsonObject>* NodeObject = nullptr;
			if (!(*Nodes)[NodeIndex].IsValid() || !(*Nodes)[NodeIndex]->TryGetObject(NodeObject) || !NodeObject)
			{
				continue;
			}

			const FString FunctionName = IISUsageGraph::GetStringField(*NodeObject, TEXT("function_name"));
			const FString FunctionOwner = IISUsageGraph::GetStringField(*NodeObject, TEXT("function_owner"));
			const FString NodeId = IISUsageGraph::GetStringField(*NodeObject, TEXT("node_id"));
			if (FunctionName.IsEmpty())
			{
				continue;
			}

			const FString Qualified = FunctionOwner.IsEmpty()
				? FunctionName
				: FString::Printf(TEXT("%s::%s"), *FunctionOwner, *FunctionName);

			FIISBlueprintReference BlueprintRef;
			BlueprintRef.BlueprintName = EffectiveBlueprintName;
			BlueprintRef.GraphOrNodeRef = FString::Printf(TEXT("%s/%s"), *GraphName, *NodeId);
			BlueprintRef.ReferencedSymbolId = DeriveSymbolId(Qualified, FunctionName, TEXT(""), 0);
			OutBlueprintRefs.Add(BlueprintRef);
		}
	}

	return OutBlueprintRefs.Num() > 0;
}

bool FIISUsageGraphImporter::ImportFromHandoff(
	const FString& HandoffContractPath,
	TArray<FString>& OutWarnings)
{
	FResolvedUsageEvidencePaths Paths;
	if (!ResolveEvidencePathsFromHandoffContract(HandoffContractPath, Paths, OutWarnings))
	{
		return false;
	}

	TArray<FIISSymbolRecord> Symbols;
	TArray<FIISUsageRecord> Usages;
	TArray<FIISCallEdge> CallEdges;
	TArray<FIISAssetReference> AssetRefs;
	TArray<FIISBlueprintReference> BlueprintRefs;

	auto LoadAndParse = [&OutWarnings](const FString& Path, auto Parser) -> bool
	{
		if (!FPaths::FileExists(Path))
		{
			return false;
		}
		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *Path))
		{
			OutWarnings.Add(FString::Printf(TEXT("Could not read evidence file: %s"), *Path));
			return false;
		}
		return Parser(JsonText);
	};

	LoadAndParse(Paths.CppSymbolIndexPath, [&](const FString& JsonText)
	{
		return ParseCppSymbolIndexJson(JsonText, Symbols, Usages, OutWarnings);
	});
	LoadAndParse(Paths.ReflectionSurfaceIndexPath, [&](const FString& JsonText)
	{
		return ParseReflectionSurfaceIndexJson(JsonText, Symbols, Usages, OutWarnings);
	});
	LoadAndParse(Paths.NetworkSurfaceIndexPath, [&](const FString& JsonText)
	{
		return ParseNetworkSurfaceIndexJson(JsonText, Symbols, CallEdges, OutWarnings);
	});
	LoadAndParse(Paths.AssetRegistryExportPath, [&](const FString& JsonText)
	{
		return ParseAssetRegistryExportJson(JsonText, AssetRefs, OutWarnings);
	});

	if (FPaths::DirectoryExists(Paths.BlueprintsDirectory))
	{
		TArray<FString> BlueprintFiles;
		IFileManager::Get().FindFilesRecursive(
			BlueprintFiles,
			*Paths.BlueprintsDirectory,
			TEXT("*.blueprint_ir.json"),
			true,
			false);
		for (const FString& BlueprintFile : BlueprintFiles)
		{
			FString JsonText;
			if (!FFileHelper::LoadFileToString(JsonText, *BlueprintFile))
			{
				continue;
			}
			const FString Stem = FPaths::GetBaseFilename(BlueprintFile).Replace(TEXT(".blueprint_ir"), TEXT(""));
			ParseBlueprintIrJson(JsonText, Stem, BlueprintRefs, OutWarnings);
		}
	}

	if (Symbols.Num() == 0)
	{
		OutWarnings.Add(TEXT("Usage graph import found no symbols."));
		return false;
	}

	return IISUsageGraph::PersistGraphPayload(Symbols, Usages, CallEdges, AssetRefs, BlueprintRefs, OutWarnings);
}

bool FIISUsageGraphImporter::ImportGraphPayloadForTest(
	const TArray<FIISSymbolRecord>& Symbols,
	const TArray<FIISUsageRecord>& Usages,
	const TArray<FIISCallEdge>& CallEdges,
	const TArray<FIISAssetReference>& AssetRefs,
	const TArray<FIISBlueprintReference>& BlueprintRefs,
	TArray<FString>& OutWarnings)
{
	FIISStoragePaths::EnsureDefaultFolders();
	return IISUsageGraph::PersistGraphPayload(Symbols, Usages, CallEdges, AssetRefs, BlueprintRefs, OutWarnings);
}

bool FIISUsageGraphImporter::QueryUsages(const FString& Query, FIISUsageQueryResult& OutResult)
{
	OutResult = FIISUsageQueryResult();
	const FString TrimmedQuery = Query.TrimStartAndEnd();
	if (TrimmedQuery.IsEmpty())
	{
		OutResult.Warnings.Add(TEXT("QueryUsages requires a non-empty query."));
		return false;
	}

	const FString CatalogPath = FIISChunkCatalog::GetCatalogPath();
	if (!FPaths::FileExists(CatalogPath))
	{
		OutResult.Warnings.Add(TEXT("IIS catalog does not exist."));
		return false;
	}

	FSQLiteDatabase Database;
	if (!Database.Open(*CatalogPath, ESQLiteDatabaseOpenMode::ReadOnly))
	{
		OutResult.Warnings.Add(FString::Printf(TEXT("Failed to open catalog: %s"), *Database.GetLastError()));
		return false;
	}

	const FString LikePattern = FString::Printf(TEXT("%%%s%%"), *TrimmedQuery);
	TSet<FString> MatchedSymbolIds;
	TSet<FString> MatchedModuleNames;

	auto SymbolMatchesQuery = [&TrimmedQuery](const FString& Name, const FString& QualifiedName) -> bool
	{
		return IISUsageGraph::QueryMatches(Name, TrimmedQuery) || IISUsageGraph::QueryMatches(QualifiedName, TrimmedQuery);
	};

	Database.Execute(
		TEXT("SELECT symbol_id, name, qualified_name, module_name FROM symbols ")
		TEXT("WHERE (lifecycle_state IS NULL OR lifecycle_state = '' OR lifecycle_state = 'active');"),
		[&](const FSQLitePreparedStatement& Row)
		{
			FString SymbolId;
			FString Name;
			FString QualifiedName;
			FString ModuleName;
			Row.GetColumnValueByName(TEXT("symbol_id"), SymbolId);
			Row.GetColumnValueByName(TEXT("name"), Name);
			Row.GetColumnValueByName(TEXT("qualified_name"), QualifiedName);
			Row.GetColumnValueByName(TEXT("module_name"), ModuleName);
			if (SymbolMatchesQuery(Name, QualifiedName))
			{
				MatchedSymbolIds.Add(SymbolId);
				if (!ModuleName.IsEmpty() && IISUsageGraph::QueryMatches(ModuleName, TrimmedQuery))
				{
					MatchedModuleNames.Add(ModuleName);
				}
			}
			return ESQLitePreparedStatementExecuteRowResult::Continue;
		});

	{
		FSQLitePreparedStatement AssetStatement = Database.PrepareStatement(
			TEXT("SELECT asset_path, referencing_symbol_id FROM asset_refs WHERE asset_path LIKE ?;"),
			ESQLitePreparedStatementFlags::None);
		if (AssetStatement.IsValid())
		{
			AssetStatement.SetBindingValueByIndex(1, LikePattern);
			AssetStatement.Execute([&](const FSQLitePreparedStatement& Row)
			{
				FString AssetPath;
				FString ReferencingSymbolId;
				Row.GetColumnValueByName(TEXT("asset_path"), AssetPath);
				Row.GetColumnValueByName(TEXT("referencing_symbol_id"), ReferencingSymbolId);
				if (!ReferencingSymbolId.IsEmpty())
				{
					MatchedSymbolIds.Add(ReferencingSymbolId);
				}
				FIISAssetReference AssetRef;
				AssetRef.AssetPath = AssetPath;
				AssetRef.ReferencingSymbolId = ReferencingSymbolId;
				AssetRef.RefKind = TEXT("asset_match");
				OutResult.AssetRefs.Add(AssetRef);
				return ESQLitePreparedStatementExecuteRowResult::Continue;
			});
		}

		FSQLitePreparedStatement BlueprintStatement = Database.PrepareStatement(
			TEXT("SELECT blueprint_name, graph_or_node_ref, referenced_symbol_id FROM blueprint_refs WHERE blueprint_name LIKE ?;"),
			ESQLitePreparedStatementFlags::None);
		if (BlueprintStatement.IsValid())
		{
			BlueprintStatement.SetBindingValueByIndex(1, LikePattern);
			BlueprintStatement.Execute([&](const FSQLitePreparedStatement& Row)
			{
				FString BlueprintName;
				FString GraphOrNodeRef;
				FString ReferencedSymbolId;
				Row.GetColumnValueByName(TEXT("blueprint_name"), BlueprintName);
				Row.GetColumnValueByName(TEXT("graph_or_node_ref"), GraphOrNodeRef);
				Row.GetColumnValueByName(TEXT("referenced_symbol_id"), ReferencedSymbolId);
				if (!ReferencedSymbolId.IsEmpty())
				{
					MatchedSymbolIds.Add(ReferencedSymbolId);
				}
				FIISBlueprintReference BlueprintRef;
				BlueprintRef.BlueprintName = BlueprintName;
				BlueprintRef.GraphOrNodeRef = GraphOrNodeRef;
				BlueprintRef.ReferencedSymbolId = ReferencedSymbolId;
				OutResult.BlueprintNodes.Add(BlueprintRef);
				return ESQLitePreparedStatementExecuteRowResult::Continue;
			});
		}
	}

	if (MatchedSymbolIds.Num() == 0)
	{
		Database.Close();
		return true;
	}

	FString InClause;
	for (const FString& SymbolId : MatchedSymbolIds)
	{
		if (!InClause.IsEmpty())
		{
			InClause += TEXT(",");
		}
		InClause += FString::Printf(TEXT("'%s'"), *SymbolId.Replace(TEXT("'"), TEXT("''")));
	}

	const FString UsageSql = FString::Printf(
		TEXT("SELECT symbol_id, usage_kind, rel_path, json_pointer, fingerprint FROM usages WHERE symbol_id IN (%s);"),
		*InClause);
	Database.Execute(*UsageSql, [&](const FSQLitePreparedStatement& Row)
	{
		FIISUsageRecord Usage;
		FString UsageKindStr;
		Row.GetColumnValueByName(TEXT("symbol_id"), Usage.SymbolId);
		Row.GetColumnValueByName(TEXT("usage_kind"), UsageKindStr);
		Row.GetColumnValueByName(TEXT("rel_path"), Usage.Location.RelativePath);
		Row.GetColumnValueByName(TEXT("json_pointer"), Usage.Location.JsonPointer);
		Row.GetColumnValueByName(TEXT("fingerprint"), Usage.Location.Fingerprint);
		Usage.UsageKind = IISUsageGraph::UsageKindFromDbString(UsageKindStr);
		switch (Usage.UsageKind)
		{
		case EIISUsageKind::Declaration:
			OutResult.Declarations.Add(Usage);
			break;
		case EIISUsageKind::Reference:
		case EIISUsageKind::Call:
		default:
			OutResult.References.Add(Usage);
			break;
		}
		return ESQLitePreparedStatementExecuteRowResult::Continue;
	});

	const FString CallSql = FString::Printf(
		TEXT("SELECT caller_id, callee_id, rel_path, json_pointer FROM call_edges ")
		TEXT("WHERE caller_id IN (%s) OR callee_id IN (%s);"),
		*InClause,
		*InClause);
	Database.Execute(*CallSql, [&](const FSQLitePreparedStatement& Row)
	{
		FIISCallEdge Edge;
		Row.GetColumnValueByName(TEXT("caller_id"), Edge.CallerSymbolId);
		Row.GetColumnValueByName(TEXT("callee_id"), Edge.CalleeSymbolId);
		Row.GetColumnValueByName(TEXT("rel_path"), Edge.Evidence.RelativePath);
		Row.GetColumnValueByName(TEXT("json_pointer"), Edge.Evidence.JsonPointer);
		OutResult.Calls.Add(Edge);
		return ESQLitePreparedStatementExecuteRowResult::Continue;
	});

	for (const FString& ModuleName : MatchedModuleNames)
	{
		OutResult.ModuleRefs.AddUnique(ModuleName);
	}

	Database.Close();

	OutResult.bGraphEvidenceAvailable =
		OutResult.Declarations.Num() > 0
		|| OutResult.References.Num() > 0
		|| OutResult.Calls.Num() > 0
		|| OutResult.BlueprintNodes.Num() > 0
		|| OutResult.AssetRefs.Num() > 0
		|| OutResult.ModuleRefs.Num() > 0;

	return true;
}
