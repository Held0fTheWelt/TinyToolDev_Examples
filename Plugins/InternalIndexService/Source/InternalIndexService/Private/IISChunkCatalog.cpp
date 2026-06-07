// SPDX-FileCopyrightText: Copyright (c) 2025-2026 Yves Tanas
// SPDX-License-Identifier: LicenseRef-Fab-Standard-EULA
//
// This file is part of the "Internal Index Service" Unreal Engine plugin.
// Use, reproduction, distribution, and modification are governed by the Fab Standard End User License Agreement,
// available at: https://www.fab.com/eula.

#include "IISChunkCatalog.h"

#include "IISEmbeddingTypes.h"
#include "IISEmbeddingJobQueue.h"
#include "IISStoragePaths.h"
#include "IISVectorIndexBackend.h"
#include "Algo/Sort.h"
#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"
#include "SQLiteDatabase.h"
#include "SQLitePreparedStatement.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
	const TCHAR* CatalogFileName = TEXT("iis_chunk_catalog.sqlite");
	const TCHAR* CatalogBuildReportJsonName = TEXT("catalog_build_report.json");
	const TCHAR* CatalogBuildReportMarkdownName = TEXT("catalog_build_report.md");
	const TCHAR* SearchReportJsonName = TEXT("search_report.json");
	const TCHAR* SearchReportMarkdownName = TEXT("search_report.md");
	const TCHAR* ContextPackReportJsonName = TEXT("latest_context_pack.json");
	const TCHAR* ContextPackReportMarkdownName = TEXT("latest_context_pack.md");

	constexpr float DefaultHybridLexicalWeight = 0.55f;
	constexpr float DefaultHybridVectorWeight = 0.45f;
	constexpr float PreferredGroupHybridBonus = 0.05f;
	constexpr float ExactTitleMatchHybridBonus = 0.05f;
	constexpr int32 MaxVectorCandidatesBeforeMerge = 50;
	constexpr int32 MaxLexicalCandidatesBeforeMerge = 50;

#if WITH_DEV_AUTOMATION_TESTS
	FString GForceUpsertFailureChunkId;
#endif

	FString ChunkKindToString(const EIISChunkKind ChunkKind)
	{
		switch (ChunkKind)
		{
		case EIISChunkKind::Code:
			return TEXT("Code");
		case EIISChunkKind::Blueprint:
			return TEXT("Blueprint");
		case EIISChunkKind::Asset:
			return TEXT("Asset");
		case EIISChunkKind::Module:
			return TEXT("Module");
		case EIISChunkKind::Reflection:
			return TEXT("Reflection");
		case EIISChunkKind::Network:
			return TEXT("Network");
		case EIISChunkKind::ArchitectureFinding:
			return TEXT("ArchitectureFinding");
		case EIISChunkKind::CandidateReview:
			return TEXT("CandidateReview");
		case EIISChunkKind::PlanningIntake:
			return TEXT("PlanningIntake");
		case EIISChunkKind::RagPreparedChunk:
			return TEXT("RagPreparedChunk");
		case EIISChunkKind::Documentation:
			return TEXT("Documentation");
		case EIISChunkKind::Guardrail:
			return TEXT("Guardrail");
		case EIISChunkKind::Unknown:
		default:
			return TEXT("Unknown");
		}
	}

	FString ChunkSensitivityToString(const EIISChunkSensitivity Sensitivity)
	{
		switch (Sensitivity)
		{
		case EIISChunkSensitivity::PublicProductSafe:
			return TEXT("PublicProductSafe");
		case EIISChunkSensitivity::ProjectLocal:
			return TEXT("ProjectLocal");
		case EIISChunkSensitivity::SourceEvidence:
			return TEXT("SourceEvidence");
		case EIISChunkSensitivity::PrivateReview:
			return TEXT("PrivateReview");
		case EIISChunkSensitivity::Restricted:
			return TEXT("Restricted");
		case EIISChunkSensitivity::Unknown:
		default:
			return TEXT("Unknown");
		}
	}

	FString CatalogStatusToString(const EIISCatalogStatus Status)
	{
		switch (Status)
		{
		case EIISCatalogStatus::Ready:
			return TEXT("Ready");
		case EIISCatalogStatus::Empty:
			return TEXT("Empty");
		case EIISCatalogStatus::Warning:
			return TEXT("Warning");
		case EIISCatalogStatus::Error:
			return TEXT("Error");
		case EIISCatalogStatus::Unknown:
		default:
			return TEXT("Unknown");
		}
	}

	FString SearchStatusToString(const EIISSearchStatus Status)
	{
		switch (Status)
		{
		case EIISSearchStatus::Ready:
			return TEXT("Ready");
		case EIISSearchStatus::Empty:
			return TEXT("Empty");
		case EIISSearchStatus::Warning:
			return TEXT("Warning");
		case EIISSearchStatus::Error:
			return TEXT("Error");
		case EIISSearchStatus::Unknown:
		default:
			return TEXT("Unknown");
		}
	}

	FString SearchModeToString(const EIISSearchMode SearchMode)
	{
		switch (SearchMode)
		{
		case EIISSearchMode::Lexical:
			return TEXT("Lexical");
		case EIISSearchMode::Vector:
			return TEXT("Vector");
		case EIISSearchMode::Hybrid:
			return TEXT("Hybrid");
		case EIISSearchMode::Unknown:
		default:
			return TEXT("Unknown");
		}
	}

	FString ContextPackStatusToString(const EIISContextPackStatus Status)
	{
		switch (Status)
		{
		case EIISContextPackStatus::Ready:
			return TEXT("Ready");
		case EIISContextPackStatus::Empty:
			return TEXT("Empty");
		case EIISContextPackStatus::Warning:
			return TEXT("Warning");
		case EIISContextPackStatus::Error:
			return TEXT("Error");
		case EIISContextPackStatus::Unknown:
		default:
			return TEXT("Unknown");
		}
	}

	FString NormalizeEnumToken(const FString& Value)
	{
		FString Normalized = Value;
		Normalized.ReplaceInline(TEXT("_"), TEXT(""));
		Normalized.ReplaceInline(TEXT("-"), TEXT(""));
		Normalized.ReplaceInline(TEXT(" "), TEXT(""));
		return Normalized.ToLower();
	}

	EIISChunkKind ParseChunkKind(const FString& Value)
	{
		const FString Token = NormalizeEnumToken(Value);
		if (Token == TEXT("code"))
		{
			return EIISChunkKind::Code;
		}
		if (Token == TEXT("blueprint"))
		{
			return EIISChunkKind::Blueprint;
		}
		if (Token == TEXT("asset"))
		{
			return EIISChunkKind::Asset;
		}
		if (Token == TEXT("module"))
		{
			return EIISChunkKind::Module;
		}
		if (Token == TEXT("reflection"))
		{
			return EIISChunkKind::Reflection;
		}
		if (Token == TEXT("network"))
		{
			return EIISChunkKind::Network;
		}
		if (Token == TEXT("architecturefinding"))
		{
			return EIISChunkKind::ArchitectureFinding;
		}
		if (Token == TEXT("candidatereview"))
		{
			return EIISChunkKind::CandidateReview;
		}
		if (Token == TEXT("planningintake") || Token == TEXT("planningintakeitem"))
		{
			return EIISChunkKind::PlanningIntake;
		}
		if (Token == TEXT("ragpreparedchunk"))
		{
			return EIISChunkKind::RagPreparedChunk;
		}
		if (Token == TEXT("documentation"))
		{
			return EIISChunkKind::Documentation;
		}
		if (Token == TEXT("guardrail") || Token == TEXT("guardrailnotice"))
		{
			return EIISChunkKind::Guardrail;
		}

		return EIISChunkKind::Unknown;
	}

	EIISChunkSensitivity ParseSensitivity(const FString& Value)
	{
		const FString Token = NormalizeEnumToken(Value);
		if (Token == TEXT("publicproductsafe"))
		{
			return EIISChunkSensitivity::PublicProductSafe;
		}
		if (Token == TEXT("projectlocal"))
		{
			return EIISChunkSensitivity::ProjectLocal;
		}
		if (Token == TEXT("sourceevidence"))
		{
			return EIISChunkSensitivity::SourceEvidence;
		}
		if (Token == TEXT("privatereview"))
		{
			return EIISChunkSensitivity::PrivateReview;
		}
		if (Token == TEXT("restricted"))
		{
			return EIISChunkSensitivity::Restricted;
		}

		return EIISChunkSensitivity::Unknown;
	}

	FString GetStringField(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName)
	{
		FString Value;
		if (Object.IsValid())
		{
			Object->TryGetStringField(FieldName, Value);
		}
		return Value;
	}

	bool GetBoolField(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName)
	{
		bool bValue = false;
		if (Object.IsValid())
		{
			Object->TryGetBoolField(FieldName, bValue);
		}
		return bValue;
	}

	TArray<TSharedPtr<FJsonValue>> MakeStringArray(const TArray<FString>& Values)
	{
		TArray<TSharedPtr<FJsonValue>> JsonValues;
		for (const FString& Value : Values)
		{
			JsonValues.Add(MakeShared<FJsonValueString>(Value));
		}
		return JsonValues;
	}

	void AddStringArrayFromJsonValueArray(const TArray<TSharedPtr<FJsonValue>>& Values, TArray<FString>& OutValues)
	{
		for (const TSharedPtr<FJsonValue>& Value : Values)
		{
			if (!Value.IsValid())
			{
				continue;
			}

			if (Value->Type == EJson::String)
			{
				OutValues.Add(Value->AsString());
				continue;
			}

			if (Value->Type == EJson::Object)
			{
				const TSharedPtr<FJsonObject> Object = Value->AsObject();
				if (!Object.IsValid())
				{
					continue;
				}

				FString TextValue;
				if (Object->TryGetStringField(TEXT("label_id"), TextValue)
					|| Object->TryGetStringField(TEXT("group_id"), TextValue)
					|| Object->TryGetStringField(TEXT("value"), TextValue)
					|| Object->TryGetStringField(TEXT("label_kind"), TextValue))
				{
					OutValues.Add(TextValue);
				}
			}
		}
	}

	void LoadStringArrayField(
		const TSharedPtr<FJsonObject>& Object,
		const TCHAR* FieldName,
		TArray<FString>& OutValues)
	{
		const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
		if (Object.IsValid() && Object->TryGetArrayField(FieldName, Values))
		{
			AddStringArrayFromJsonValueArray(*Values, OutValues);
		}
	}

	FIISSourceReference MakeSourceReferenceFromObject(const TSharedPtr<FJsonObject>& Object)
	{
		FIISSourceReference Reference;
		Reference.ArtifactKind = GetStringField(Object, TEXT("artifact_kind"));
		Reference.RelativePath = GetStringField(Object, TEXT("relative_path"));
		Reference.JsonPointer = GetStringField(Object, TEXT("json_pointer"));
		Reference.Fingerprint = GetStringField(Object, TEXT("fingerprint"));
		Reference.Explanation = GetStringField(Object, TEXT("explanation"));
		return Reference;
	}

	FIISSourceReference MakeSourceReferenceFromString(const FString& Value)
	{
		FIISSourceReference Reference;
		TArray<FString> Parts;
		Value.ParseIntoArray(Parts, TEXT("|"), false);
		for (FString& Part : Parts)
		{
			Part.TrimStartAndEndInline();
		}

		if (Parts.Num() >= 4)
		{
			Reference.JsonPointer = Parts[0];
			Reference.Fingerprint = Parts[1];
			Reference.ArtifactKind = Parts[2];
			Reference.RelativePath = Parts[3];
		}
		else
		{
			Reference.Explanation = Value;
		}

		return Reference;
	}

	void LoadSourceReferences(const TSharedPtr<FJsonObject>& Object, TArray<FIISSourceReference>& OutReferences)
	{
		const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
		if (!Object.IsValid() || !Object->TryGetArrayField(TEXT("source_references"), Values))
		{
			return;
		}

		for (const TSharedPtr<FJsonValue>& Value : *Values)
		{
			if (!Value.IsValid())
			{
				continue;
			}

			if (Value->Type == EJson::String)
			{
				OutReferences.Add(MakeSourceReferenceFromString(Value->AsString()));
			}
			else if (Value->Type == EJson::Object)
			{
				OutReferences.Add(MakeSourceReferenceFromObject(Value->AsObject()));
			}
		}
	}

	bool SaveJsonObjectToFile(const TSharedRef<FJsonObject>& Object, const FString& FilePath)
	{
		FString JsonOutput;
		const TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> Writer =
			TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&JsonOutput);
		if (!FJsonSerializer::Serialize(Object, Writer))
		{
			return false;
		}

		return FFileHelper::SaveStringToFile(JsonOutput, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}

	bool LoadChunkFromJsonObject(const TSharedPtr<FJsonObject>& Object, FIISIndexChunk& OutChunk, TArray<FString>& OutWarnings)
	{
		if (!Object.IsValid())
		{
			return false;
		}

		OutChunk = FIISIndexChunk();
		OutChunk.ChunkId = GetStringField(Object, TEXT("chunk_id"));
		OutChunk.SourceChunkId = GetStringField(Object, TEXT("source_chunk_id"));
		OutChunk.ChunkKind = ParseChunkKind(GetStringField(Object, TEXT("chunk_kind")));
		OutChunk.Sensitivity = ParseSensitivity(GetStringField(Object, TEXT("sensitivity")));
		OutChunk.Title = GetStringField(Object, TEXT("title"));
		OutChunk.Text = GetStringField(Object, TEXT("text"));
		OutChunk.ModuleName = GetStringField(Object, TEXT("module_name"));
		OutChunk.SourceId = GetStringField(Object, TEXT("source_id"));
		OutChunk.SourceRunId = GetStringField(Object, TEXT("source_run_id"));
		OutChunk.DestinationRunId = GetStringField(Object, TEXT("destination_run_id"));
		OutChunk.bIsAiGenerated = GetBoolField(Object, TEXT("is_ai_generated"));
		OutChunk.bAllowsMigrationDecision = GetBoolField(Object, TEXT("allows_migration_decision"));
		OutChunk.bAllowsPatchGeneration = GetBoolField(Object, TEXT("allows_patch_generation"));
		OutChunk.LifecycleState = GetStringField(Object, TEXT("lifecycle_state"));

		LoadStringArrayField(Object, TEXT("retrieval_labels"), OutChunk.RetrievalLabels);
		LoadStringArrayField(Object, TEXT("retrieval_groups"), OutChunk.RetrievalGroups);
		LoadStringArrayField(Object, TEXT("warnings"), OutChunk.Warnings);
		LoadSourceReferences(Object, OutChunk.SourceReferences);

		if (OutChunk.ChunkId.IsEmpty())
		{
			OutWarnings.Add(TEXT("Chunk store row has no chunk_id."));
			return false;
		}

		return true;
	}

	bool LoadChunksFromJsonl(
		const FString& ChunkStoreJsonlPath,
		TArray<FIISIndexChunk>& OutChunks,
		FIISCatalogBuildReport& Report)
	{
		if (ChunkStoreJsonlPath.IsEmpty())
		{
			Report.Errors.Add(TEXT("Chunk store JSONL path is empty."));
			return false;
		}

		if (!FPaths::FileExists(ChunkStoreJsonlPath))
		{
			Report.Errors.Add(FString::Printf(TEXT("Chunk store JSONL file does not exist: %s"), *ChunkStoreJsonlPath));
			return false;
		}

		TArray<FString> Lines;
		if (!FFileHelper::LoadFileToStringArray(Lines, *ChunkStoreJsonlPath))
		{
			Report.Errors.Add(FString::Printf(TEXT("Failed to read chunk store JSONL: %s"), *ChunkStoreJsonlPath));
			return false;
		}

		for (int32 LineIndex = 0; LineIndex < Lines.Num(); ++LineIndex)
		{
			FString Line = Lines[LineIndex];
			Line.TrimStartAndEndInline();
			if (Line.IsEmpty())
			{
				continue;
			}

			++Report.Summary.SourceChunkCount;

			TSharedPtr<FJsonObject> Object;
			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Line);
			if (!FJsonSerializer::Deserialize(Reader, Object) || !Object.IsValid())
			{
				++Report.Summary.SkippedChunkCount;
				Report.Errors.Add(FString::Printf(TEXT("Chunk store line %d is not a valid JSON object."), LineIndex + 1));
				continue;
			}

			FIISIndexChunk Chunk;
			if (!LoadChunkFromJsonObject(Object, Chunk, Report.Warnings))
			{
				++Report.Summary.SkippedChunkCount;
				continue;
			}

			OutChunks.Add(MoveTemp(Chunk));
		}

		return Report.Errors.Num() == 0 || OutChunks.Num() > 0;
	}

	bool OpenCatalogDatabase(FSQLiteDatabase& Database, TArray<FString>& OutErrors)
	{
		const FString CatalogPath = FIISChunkCatalog::GetCatalogPath();
		IFileManager::Get().MakeDirectory(*FIISStoragePaths::GetIndexesDir(), true);
		if (!Database.Open(*CatalogPath, ESQLiteDatabaseOpenMode::ReadWriteCreate))
		{
			OutErrors.Add(FString::Printf(TEXT("Failed to open SQLite catalog: %s"), *CatalogPath));
			OutErrors.Add(Database.GetLastError());
			return false;
		}

		return true;
	}

	bool ExecuteStatement(FSQLiteDatabase& Database, const TCHAR* Statement, TArray<FString>& OutErrors)
	{
		if (Database.Execute(Statement))
		{
			return true;
		}

		OutErrors.Add(Database.GetLastError());
		return false;
	}

	bool ChunksTableHasColumn(FSQLiteDatabase& Database, const TCHAR* ColumnName)
	{
		bool bFound = false;
		Database.Execute(TEXT("PRAGMA table_info(chunks);"), [&bFound, ColumnName](const FSQLitePreparedStatement& Row)
		{
			FString Name;
			Row.GetColumnValueByName(TEXT("name"), Name);
			if (Name.Equals(ColumnName, ESearchCase::IgnoreCase))
			{
				bFound = true;
			}
			return ESQLitePreparedStatementExecuteRowResult::Continue;
		});
		return bFound;
	}

	void MigrateChunksSchemaIfNeeded(FSQLiteDatabase& Database)
	{
		if (!ChunksTableHasColumn(Database, TEXT("text_sha256")))
		{
			Database.Execute(TEXT("ALTER TABLE chunks ADD COLUMN text_sha256 TEXT;"));
		}
		if (!ChunksTableHasColumn(Database, TEXT("lifecycle_state")))
		{
			Database.Execute(TEXT("ALTER TABLE chunks ADD COLUMN lifecycle_state TEXT DEFAULT 'active';"));
		}
		if (!ChunksTableHasColumn(Database, TEXT("source_refs_hash")))
		{
			Database.Execute(TEXT("ALTER TABLE chunks ADD COLUMN source_refs_hash TEXT;"));
		}
	}

	FString ComputeSourceRefsHash(const TArray<FIISSourceReference>& SourceReferences)
	{
		TArray<FString> Lines;
		Lines.Reserve(SourceReferences.Num());
		for (const FIISSourceReference& Ref : SourceReferences)
		{
			Lines.Add(Ref.RelativePath + TEXT("|") + Ref.Fingerprint);
		}
		Lines.Sort();
		return FIISEmbeddingJobQueue::ComputeSHA256(FString::Join(Lines, TEXT("\n")));
	}

	void PrepareChunkForCatalog(FIISIndexChunk& Chunk)
	{
		if (Chunk.TextSha256.IsEmpty())
		{
			Chunk.TextSha256 = FIISEmbeddingJobQueue::ComputeSHA256(Chunk.Text);
		}
		if (Chunk.LifecycleState.IsEmpty())
		{
			Chunk.LifecycleState = TEXT("active");
		}
		if (Chunk.SourceRefsHash.IsEmpty())
		{
			Chunk.SourceRefsHash = ComputeSourceRefsHash(Chunk.SourceReferences);
		}
	}

	bool EnsureCatalogSchema(FSQLiteDatabase& Database, FIISCatalogBuildReport& Report, bool& bOutFtsActive)
	{
		bool bSuccess = true;
		bOutFtsActive = false;

		bSuccess &= ExecuteStatement(Database, TEXT("PRAGMA user_version=1;"), Report.Errors);
		bSuccess &= ExecuteStatement(
			Database,
			TEXT("CREATE TABLE IF NOT EXISTS chunks (")
			TEXT("chunk_id TEXT PRIMARY KEY,")
			TEXT("source_chunk_id TEXT,")
			TEXT("chunk_kind TEXT,")
			TEXT("sensitivity TEXT,")
			TEXT("title TEXT,")
			TEXT("text TEXT,")
			TEXT("module_name TEXT,")
			TEXT("source_id TEXT,")
			TEXT("source_run_id TEXT,")
			TEXT("destination_run_id TEXT,")
			TEXT("is_ai_generated INTEGER,")
			TEXT("allows_migration_decision INTEGER,")
			TEXT("allows_patch_generation INTEGER,")
			TEXT("text_sha256 TEXT,")
			TEXT("lifecycle_state TEXT DEFAULT 'active',")
			TEXT("source_refs_hash TEXT,")
			TEXT("imported_at_utc TEXT")
			TEXT(");"),
			Report.Errors);
		if (bSuccess)
		{
			MigrateChunksSchemaIfNeeded(Database);
		}
		bSuccess &= ExecuteStatement(
			Database,
			TEXT("CREATE TABLE IF NOT EXISTS chunk_labels (")
			TEXT("chunk_id TEXT,")
			TEXT("label TEXT,")
			TEXT("PRIMARY KEY (chunk_id, label)")
			TEXT(");"),
			Report.Errors);
		bSuccess &= ExecuteStatement(
			Database,
			TEXT("CREATE TABLE IF NOT EXISTS chunk_groups (")
			TEXT("chunk_id TEXT,")
			TEXT("group_id TEXT,")
			TEXT("PRIMARY KEY (chunk_id, group_id)")
			TEXT(");"),
			Report.Errors);
		bSuccess &= ExecuteStatement(
			Database,
			TEXT("CREATE TABLE IF NOT EXISTS chunk_source_refs (")
			TEXT("chunk_id TEXT,")
			TEXT("artifact_kind TEXT,")
			TEXT("relative_path TEXT,")
			TEXT("json_pointer TEXT,")
			TEXT("fingerprint TEXT,")
			TEXT("explanation TEXT")
			TEXT(");"),
			Report.Errors);

		bSuccess &= ExecuteStatement(
			Database,
			TEXT("CREATE TABLE IF NOT EXISTS symbols (")
			TEXT("symbol_id TEXT PRIMARY KEY,")
			TEXT("name TEXT,")
			TEXT("qualified_name TEXT,")
			TEXT("kind TEXT,")
			TEXT("module_name TEXT,")
			TEXT("lifecycle_state TEXT DEFAULT 'active'")
			TEXT(");"),
			Report.Errors);
		bSuccess &= ExecuteStatement(
			Database,
			TEXT("CREATE TABLE IF NOT EXISTS usages (")
			TEXT("symbol_id TEXT,")
			TEXT("usage_kind TEXT,")
			TEXT("rel_path TEXT,")
			TEXT("json_pointer TEXT,")
			TEXT("fingerprint TEXT")
			TEXT(");"),
			Report.Errors);
		bSuccess &= ExecuteStatement(
			Database,
			TEXT("CREATE TABLE IF NOT EXISTS call_edges (")
			TEXT("caller_id TEXT,")
			TEXT("callee_id TEXT,")
			TEXT("rel_path TEXT,")
			TEXT("json_pointer TEXT")
			TEXT(");"),
			Report.Errors);
		bSuccess &= ExecuteStatement(
			Database,
			TEXT("CREATE TABLE IF NOT EXISTS asset_refs (")
			TEXT("asset_path TEXT,")
			TEXT("referencing_symbol_id TEXT,")
			TEXT("ref_kind TEXT")
			TEXT(");"),
			Report.Errors);
		bSuccess &= ExecuteStatement(
			Database,
			TEXT("CREATE TABLE IF NOT EXISTS blueprint_refs (")
			TEXT("blueprint_name TEXT,")
			TEXT("graph_or_node_ref TEXT,")
			TEXT("referenced_symbol_id TEXT")
			TEXT(");"),
			Report.Errors);
		bSuccess &= ExecuteStatement(
			Database,
			TEXT("CREATE INDEX IF NOT EXISTS idx_symbols_name ON symbols(name);"),
			Report.Errors);
		bSuccess &= ExecuteStatement(
			Database,
			TEXT("CREATE INDEX IF NOT EXISTS idx_symbols_qualified_name ON symbols(qualified_name);"),
			Report.Errors);

		if (Database.Execute(TEXT("DROP TABLE IF EXISTS chunks_fts;"))
			&& Database.Execute(TEXT("CREATE VIRTUAL TABLE chunks_fts USING fts5(chunk_id UNINDEXED, title, text, labels, groups);")))
		{
			bOutFtsActive = true;
		}
		else
		{
			Report.Warnings.Add(FString::Printf(TEXT("SQLite FTS5 is unavailable; lexical search will use deterministic fallback scoring. Last SQLite error: %s"), *Database.GetLastError()));
		}

		return bSuccess;
	}

	TSet<FString> LoadExistingChunkIds(FSQLiteDatabase& Database)
	{
		TSet<FString> ExistingChunkIds;
		Database.Execute(TEXT("SELECT chunk_id FROM chunks;"), [&ExistingChunkIds](const FSQLitePreparedStatement& Statement)
		{
			FString ChunkId;
			Statement.GetColumnValueByName(TEXT("chunk_id"), ChunkId);
			if (!ChunkId.IsEmpty())
			{
				ExistingChunkIds.Add(ChunkId);
			}
			return ESQLitePreparedStatementExecuteRowResult::Continue;
		});

		return ExistingChunkIds;
	}

	int32 QueryScalarInt(FSQLiteDatabase& Database, const TCHAR* StatementText)
	{
		int64 Value = 0;
		Database.Execute(StatementText, [&Value](const FSQLitePreparedStatement& Statement)
		{
			Statement.GetColumnValueByIndex(0, Value);
			return ESQLitePreparedStatementExecuteRowResult::Stop;
		});
		return static_cast<int32>(Value);
	}

	bool ResetCatalogTables(FSQLiteDatabase& Database, const bool bFtsActive, TArray<FString>& OutErrors)
	{
		bool bSuccess = true;
		bSuccess &= ExecuteStatement(Database, TEXT("DELETE FROM chunk_source_refs;"), OutErrors);
		bSuccess &= ExecuteStatement(Database, TEXT("DELETE FROM chunk_labels;"), OutErrors);
		bSuccess &= ExecuteStatement(Database, TEXT("DELETE FROM chunk_groups;"), OutErrors);
		bSuccess &= ExecuteStatement(Database, TEXT("DELETE FROM chunks;"), OutErrors);
		if (bFtsActive)
		{
			bSuccess &= ExecuteStatement(Database, TEXT("DELETE FROM chunks_fts;"), OutErrors);
		}
		return bSuccess;
	}

	bool InsertChunkRow(
		FSQLiteDatabase& Database,
		const FIISIndexChunk& Chunk,
		const FString& ImportedAtUtc,
		TArray<FString>& OutErrors)
	{
		FSQLitePreparedStatement Statement = Database.PrepareStatement(
			TEXT("INSERT OR REPLACE INTO chunks (")
			TEXT("chunk_id, source_chunk_id, chunk_kind, sensitivity, title, text, module_name, source_id, source_run_id, destination_run_id, ")
			TEXT("is_ai_generated, allows_migration_decision, allows_patch_generation, text_sha256, lifecycle_state, source_refs_hash, imported_at_utc")
			TEXT(") VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14, ?15, ?16, ?17);"),
			ESQLitePreparedStatementFlags::Persistent);
		if (!Statement.IsValid())
		{
			OutErrors.Add(FString::Printf(TEXT("Failed to prepare chunk upsert statement for %s."), *Chunk.ChunkId));
			OutErrors.Add(Database.GetLastError());
			return false;
		}

		Statement.SetBindingValueByIndex(1, Chunk.ChunkId);
		Statement.SetBindingValueByIndex(2, Chunk.SourceChunkId);
		Statement.SetBindingValueByIndex(3, ChunkKindToString(Chunk.ChunkKind));
		Statement.SetBindingValueByIndex(4, ChunkSensitivityToString(Chunk.Sensitivity));
		Statement.SetBindingValueByIndex(5, Chunk.Title);
		Statement.SetBindingValueByIndex(6, Chunk.Text);
		Statement.SetBindingValueByIndex(7, Chunk.ModuleName);
		Statement.SetBindingValueByIndex(8, Chunk.SourceId);
		Statement.SetBindingValueByIndex(9, Chunk.SourceRunId);
		Statement.SetBindingValueByIndex(10, Chunk.DestinationRunId);
		Statement.SetBindingValueByIndex(11, Chunk.bIsAiGenerated ? 1 : 0);
		Statement.SetBindingValueByIndex(12, Chunk.bAllowsMigrationDecision ? 1 : 0);
		Statement.SetBindingValueByIndex(13, Chunk.bAllowsPatchGeneration ? 1 : 0);
		Statement.SetBindingValueByIndex(14, Chunk.TextSha256);
		Statement.SetBindingValueByIndex(15, Chunk.LifecycleState);
		Statement.SetBindingValueByIndex(16, Chunk.SourceRefsHash);
		Statement.SetBindingValueByIndex(17, ImportedAtUtc);

		if (Statement.Execute())
		{
			return true;
		}

		OutErrors.Add(FString::Printf(TEXT("Failed to upsert chunk into catalog: %s"), *Chunk.ChunkId));
		OutErrors.Add(Database.GetLastError());
		return false;
	}

	bool InsertLabels(FSQLiteDatabase& Database, const FIISIndexChunk& Chunk, TArray<FString>& OutErrors)
	{
		FSQLitePreparedStatement Statement = Database.PrepareStatement(
			TEXT("INSERT OR IGNORE INTO chunk_labels (chunk_id, label) VALUES (?1, ?2);"),
			ESQLitePreparedStatementFlags::Persistent);
		if (!Statement.IsValid())
		{
			OutErrors.Add(Database.GetLastError());
			return false;
		}

		for (const FString& Label : Chunk.RetrievalLabels)
		{
			Statement.Reset();
			Statement.ClearBindings();
			Statement.SetBindingValueByIndex(1, Chunk.ChunkId);
			Statement.SetBindingValueByIndex(2, Label);
			if (!Statement.Execute())
			{
				OutErrors.Add(FString::Printf(TEXT("Failed to insert chunk label '%s' for %s."), *Label, *Chunk.ChunkId));
				OutErrors.Add(Database.GetLastError());
				return false;
			}
		}

		return true;
	}

	bool InsertGroups(FSQLiteDatabase& Database, const FIISIndexChunk& Chunk, TArray<FString>& OutErrors)
	{
		FSQLitePreparedStatement Statement = Database.PrepareStatement(
			TEXT("INSERT OR IGNORE INTO chunk_groups (chunk_id, group_id) VALUES (?1, ?2);"),
			ESQLitePreparedStatementFlags::Persistent);
		if (!Statement.IsValid())
		{
			OutErrors.Add(Database.GetLastError());
			return false;
		}

		for (const FString& Group : Chunk.RetrievalGroups)
		{
			Statement.Reset();
			Statement.ClearBindings();
			Statement.SetBindingValueByIndex(1, Chunk.ChunkId);
			Statement.SetBindingValueByIndex(2, Group);
			if (!Statement.Execute())
			{
				OutErrors.Add(FString::Printf(TEXT("Failed to insert chunk group '%s' for %s."), *Group, *Chunk.ChunkId));
				OutErrors.Add(Database.GetLastError());
				return false;
			}
		}

		return true;
	}

	bool InsertSourceRefs(FSQLiteDatabase& Database, const FIISIndexChunk& Chunk, TArray<FString>& OutErrors)
	{
		FSQLitePreparedStatement Statement = Database.PrepareStatement(
			TEXT("INSERT INTO chunk_source_refs (chunk_id, artifact_kind, relative_path, json_pointer, fingerprint, explanation) ")
			TEXT("VALUES (?1, ?2, ?3, ?4, ?5, ?6);"),
			ESQLitePreparedStatementFlags::Persistent);
		if (!Statement.IsValid())
		{
			OutErrors.Add(Database.GetLastError());
			return false;
		}

		for (const FIISSourceReference& Reference : Chunk.SourceReferences)
		{
			Statement.Reset();
			Statement.ClearBindings();
			Statement.SetBindingValueByIndex(1, Chunk.ChunkId);
			Statement.SetBindingValueByIndex(2, Reference.ArtifactKind);
			Statement.SetBindingValueByIndex(3, Reference.RelativePath);
			Statement.SetBindingValueByIndex(4, Reference.JsonPointer);
			Statement.SetBindingValueByIndex(5, Reference.Fingerprint);
			Statement.SetBindingValueByIndex(6, Reference.Explanation);
			if (!Statement.Execute())
			{
				OutErrors.Add(FString::Printf(TEXT("Failed to insert source reference for %s."), *Chunk.ChunkId));
				OutErrors.Add(Database.GetLastError());
				return false;
			}
		}

		return true;
	}

	FString JoinForFts(const TArray<FString>& Values)
	{
		return FString::Join(Values, TEXT(" "));
	}

	bool InsertFtsRow(FSQLiteDatabase& Database, const FIISIndexChunk& Chunk, TArray<FString>& OutWarnings)
	{
		FSQLitePreparedStatement Statement = Database.PrepareStatement(
			TEXT("INSERT INTO chunks_fts (chunk_id, title, text, labels, groups) VALUES (?1, ?2, ?3, ?4, ?5);"),
			ESQLitePreparedStatementFlags::Persistent);
		if (!Statement.IsValid())
		{
			OutWarnings.Add(Database.GetLastError());
			return false;
		}

		Statement.SetBindingValueByIndex(1, Chunk.ChunkId);
		Statement.SetBindingValueByIndex(2, Chunk.Title);
		Statement.SetBindingValueByIndex(3, Chunk.Text);
		Statement.SetBindingValueByIndex(4, JoinForFts(Chunk.RetrievalLabels));
		Statement.SetBindingValueByIndex(5, JoinForFts(Chunk.RetrievalGroups));
		if (!Statement.Execute())
		{
			OutWarnings.Add(FString::Printf(TEXT("Failed to insert FTS row for %s: %s"), *Chunk.ChunkId, *Database.GetLastError()));
			return false;
		}

		return true;
	}

	bool DeleteChunkChildRows(FSQLiteDatabase& Database, const FString& ChunkId, TArray<FString>& OutErrors)
	{
		bool bSuccess = true;
		FSQLitePreparedStatement Statement = Database.PrepareStatement(TEXT("DELETE FROM chunk_labels WHERE chunk_id = ?1;"));
		if (Statement.IsValid())
		{
			Statement.SetBindingValueByIndex(1, ChunkId);
			bSuccess &= Statement.Execute();
		}
		Statement = Database.PrepareStatement(TEXT("DELETE FROM chunk_groups WHERE chunk_id = ?1;"));
		if (Statement.IsValid())
		{
			Statement.Reset();
			Statement.SetBindingValueByIndex(1, ChunkId);
			bSuccess &= Statement.Execute();
		}
		Statement = Database.PrepareStatement(TEXT("DELETE FROM chunk_source_refs WHERE chunk_id = ?1;"));
		if (Statement.IsValid())
		{
			Statement.Reset();
			Statement.SetBindingValueByIndex(1, ChunkId);
			bSuccess &= Statement.Execute();
		}
		if (!bSuccess)
		{
			OutErrors.Add(FString::Printf(TEXT("Failed to delete child rows for chunk %s."), *ChunkId));
			OutErrors.Add(Database.GetLastError());
		}
		return bSuccess;
	}

	bool DeleteFtsRow(FSQLiteDatabase& Database, const FString& ChunkId, TArray<FString>& OutWarnings)
	{
		FSQLitePreparedStatement Statement = Database.PrepareStatement(TEXT("DELETE FROM chunks_fts WHERE chunk_id = ?1;"));
		if (!Statement.IsValid())
		{
			OutWarnings.Add(Database.GetLastError());
			return false;
		}
		Statement.SetBindingValueByIndex(1, ChunkId);
		if (!Statement.Execute())
		{
			OutWarnings.Add(FString::Printf(TEXT("Failed to delete FTS row for %s: %s"), *ChunkId, *Database.GetLastError()));
			return false;
		}
		return true;
	}

	bool QueryExistingChunkSha(
		FSQLiteDatabase& Database,
		const FString& ChunkId,
		bool& bOutExists,
		FString& OutSha,
		FString& OutLifecycleState)
	{
		bOutExists = false;
		OutSha.Reset();
		OutLifecycleState.Reset();
		FSQLitePreparedStatement Statement = Database.PrepareStatement(
			TEXT("SELECT text_sha256, lifecycle_state FROM chunks WHERE chunk_id = ?1;"));
		if (!Statement.IsValid())
		{
			return false;
		}
		Statement.SetBindingValueByIndex(1, ChunkId);
		bool bFound = false;
		Statement.Execute([&bFound, &bOutExists, &OutSha, &OutLifecycleState](const FSQLitePreparedStatement& Row)
		{
			bFound = true;
			bOutExists = true;
			Row.GetColumnValueByName(TEXT("text_sha256"), OutSha);
			Row.GetColumnValueByName(TEXT("lifecycle_state"), OutLifecycleState);
			return ESQLitePreparedStatementExecuteRowResult::Stop;
		});
		return bFound || !bOutExists;
	}

	bool ReactivateChunkLifecycle(FSQLiteDatabase& Database, const FString& ChunkId, TArray<FString>& OutErrors)
	{
		FSQLitePreparedStatement Statement = Database.PrepareStatement(
			TEXT("UPDATE chunks SET lifecycle_state = 'active' WHERE chunk_id = ?1;"));
		if (!Statement.IsValid())
		{
			OutErrors.Add(Database.GetLastError());
			return false;
		}
		Statement.SetBindingValueByIndex(1, ChunkId);
		if (!Statement.Execute())
		{
			OutErrors.Add(FString::Printf(TEXT("Failed to reactivate chunk lifecycle: %s"), *ChunkId));
			OutErrors.Add(Database.GetLastError());
			return false;
		}
		return true;
	}

	static bool IsStaleLifecycleState(const FString& LifecycleState)
	{
		return LifecycleState.Equals(TEXT("stale"), ESearchCase::IgnoreCase);
	}

	bool UpdateChunkRowOnConflict(
		FSQLiteDatabase& Database,
		const FIISIndexChunk& Chunk,
		const FString& ImportedAtUtc,
		TArray<FString>& OutErrors)
	{
		FSQLitePreparedStatement Statement = Database.PrepareStatement(
			TEXT("UPDATE chunks SET source_chunk_id = ?2, chunk_kind = ?3, sensitivity = ?4, title = ?5, text = ?6, ")
			TEXT("module_name = ?7, source_id = ?8, source_run_id = ?9, destination_run_id = ?10, ")
			TEXT("is_ai_generated = ?11, allows_migration_decision = ?12, allows_patch_generation = ?13, ")
			TEXT("text_sha256 = ?14, lifecycle_state = 'active', source_refs_hash = ?15, imported_at_utc = ?16 ")
			TEXT("WHERE chunk_id = ?1;"),
			ESQLitePreparedStatementFlags::Persistent);
		if (!Statement.IsValid())
		{
			OutErrors.Add(Database.GetLastError());
			return false;
		}

		Statement.SetBindingValueByIndex(1, Chunk.ChunkId);
		Statement.SetBindingValueByIndex(2, Chunk.SourceChunkId);
		Statement.SetBindingValueByIndex(3, ChunkKindToString(Chunk.ChunkKind));
		Statement.SetBindingValueByIndex(4, ChunkSensitivityToString(Chunk.Sensitivity));
		Statement.SetBindingValueByIndex(5, Chunk.Title);
		Statement.SetBindingValueByIndex(6, Chunk.Text);
		Statement.SetBindingValueByIndex(7, Chunk.ModuleName);
		Statement.SetBindingValueByIndex(8, Chunk.SourceId);
		Statement.SetBindingValueByIndex(9, Chunk.SourceRunId);
		Statement.SetBindingValueByIndex(10, Chunk.DestinationRunId);
		Statement.SetBindingValueByIndex(11, Chunk.bIsAiGenerated ? 1 : 0);
		Statement.SetBindingValueByIndex(12, Chunk.bAllowsMigrationDecision ? 1 : 0);
		Statement.SetBindingValueByIndex(13, Chunk.bAllowsPatchGeneration ? 1 : 0);
		Statement.SetBindingValueByIndex(14, Chunk.TextSha256);
		Statement.SetBindingValueByIndex(15, Chunk.SourceRefsHash);
		Statement.SetBindingValueByIndex(16, ImportedAtUtc);

		if (!Statement.Execute())
		{
			OutErrors.Add(FString::Printf(TEXT("Failed to update chunk on conflict: %s"), *Chunk.ChunkId));
			OutErrors.Add(Database.GetLastError());
			return false;
		}
		return true;
	}

	bool MarkStaleChunksForSource(
		FSQLiteDatabase& Database,
		const FString& SourceId,
		const TSet<FString>& SeenChunkIds,
		TArray<FString>& OutErrors)
	{
		if (SourceId.IsEmpty())
		{
			return true;
		}

		FString Sql;
		if (SeenChunkIds.Num() == 0)
		{
			Sql = TEXT("UPDATE chunks SET lifecycle_state = 'stale' WHERE source_id = ?1;");
		}
		else
		{
			TArray<FString> QuotedIds;
			QuotedIds.Reserve(SeenChunkIds.Num());
			for (const FString& ChunkId : SeenChunkIds)
			{
				const FString Escaped = ChunkId.Replace(TEXT("'"), TEXT("''"));
				QuotedIds.Add(FString::Printf(TEXT("'%s'"), *Escaped));
			}
			Sql = FString::Printf(
				TEXT("UPDATE chunks SET lifecycle_state = 'stale' WHERE source_id = ?1 AND chunk_id NOT IN (%s);"),
				*FString::Join(QuotedIds, TEXT(",")));
		}

		FSQLitePreparedStatement Statement = Database.PrepareStatement(*Sql);
		if (!Statement.IsValid())
		{
			OutErrors.Add(Database.GetLastError());
			return false;
		}
		Statement.SetBindingValueByIndex(1, SourceId);
		if (!Statement.Execute())
		{
			OutErrors.Add(FString::Printf(TEXT("Failed to mark stale chunks for source %s."), *SourceId));
			OutErrors.Add(Database.GetLastError());
			return false;
		}

		return true;
	}

	bool WriteImportConflictsReport(const TArray<FIISImportConflict>& Conflicts, const FString& RunId)
	{
		if (Conflicts.Num() == 0)
		{
			return true;
		}

		TArray<TSharedPtr<FJsonValue>> Values;
		for (const FIISImportConflict& Conflict : Conflicts)
		{
			TSharedRef<FJsonObject> Obj = MakeShared<FJsonObject>();
			Obj->SetStringField(TEXT("chunk_id"), Conflict.ChunkId);
			Obj->SetStringField(TEXT("old_text_sha256"), Conflict.OldSha);
			Obj->SetStringField(TEXT("new_text_sha256"), Conflict.NewSha);
			Obj->SetStringField(TEXT("source_id"), Conflict.SourceId);
			Obj->SetStringField(TEXT("policy"), Conflict.Policy);
			Values.Add(MakeShared<FJsonValueObject>(Obj));
		}

		TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
		Root->SetStringField(TEXT("run_id"), RunId);
		Root->SetArrayField(TEXT("conflicts"), Values);

		FString JsonOutput;
		const TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> Writer =
			TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&JsonOutput);
		if (!FJsonSerializer::Serialize(Root, Writer))
		{
			return false;
		}

		const FString ReportPath = FIISStoragePaths::GetReportsDir()
			/ FString::Printf(TEXT("import_conflicts_%s.json"), *RunId);
		return FFileHelper::SaveStringToFile(
			JsonOutput, *ReportPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}

	bool InsertChunkIntoCatalog(
		FSQLiteDatabase& Database,
		const FIISIndexChunk& Chunk,
		const FString& ImportedAtUtc,
		const bool bFtsActive,
		FIISCatalogBuildReport& Report)
	{
		FIISIndexChunk PreparedChunk = Chunk;
		PrepareChunkForCatalog(PreparedChunk);

		if (PreparedChunk.bIsAiGenerated || PreparedChunk.bAllowsMigrationDecision || PreparedChunk.bAllowsPatchGeneration)
		{
			++Report.Summary.SkippedChunkCount;
			Report.Errors.Add(FString::Printf(
				TEXT("Skipped unsafe chunk %s: is_ai_generated=%s, allows_migration_decision=%s, allows_patch_generation=%s."),
				*PreparedChunk.ChunkId,
				PreparedChunk.bIsAiGenerated ? TEXT("true") : TEXT("false"),
				PreparedChunk.bAllowsMigrationDecision ? TEXT("true") : TEXT("false"),
				PreparedChunk.bAllowsPatchGeneration ? TEXT("true") : TEXT("false")));
			return false;
		}

		if (!InsertChunkRow(Database, PreparedChunk, ImportedAtUtc, Report.Errors))
		{
			++Report.Summary.SkippedChunkCount;
			return false;
		}

		InsertLabels(Database, PreparedChunk, Report.Errors);
		InsertGroups(Database, PreparedChunk, Report.Errors);
		InsertSourceRefs(Database, PreparedChunk, Report.Errors);
		if (bFtsActive)
		{
			InsertFtsRow(Database, PreparedChunk, Report.Warnings);
		}

		return true;
	}

	bool UpsertChunkIntoCatalog(
		FSQLiteDatabase& Database,
		FIISIndexChunk& Chunk,
		const FString& ImportedAtUtc,
		const bool bFtsActive,
		FIISCatalogBuildReport& Report,
		TArray<FIISImportConflict>& OutConflicts,
		bool& bOutInserted,
		bool& bOutUpdated)
	{
		bOutInserted = false;
		bOutUpdated = false;
		PrepareChunkForCatalog(Chunk);

#if WITH_DEV_AUTOMATION_TESTS
		if (!GForceUpsertFailureChunkId.IsEmpty() && Chunk.ChunkId == GForceUpsertFailureChunkId)
		{
			Report.Errors.Add(FString::Printf(
				TEXT("Forced upsert failure for automation test chunk %s."), *Chunk.ChunkId));
			return false;
		}
#endif

		if (Chunk.bIsAiGenerated || Chunk.bAllowsMigrationDecision || Chunk.bAllowsPatchGeneration)
		{
			++Report.Summary.SkippedChunkCount;
			Report.Errors.Add(FString::Printf(
				TEXT("Skipped unsafe chunk %s: is_ai_generated=%s, allows_migration_decision=%s, allows_patch_generation=%s."),
				*Chunk.ChunkId,
				Chunk.bIsAiGenerated ? TEXT("true") : TEXT("false"),
				Chunk.bAllowsMigrationDecision ? TEXT("true") : TEXT("false"),
				Chunk.bAllowsPatchGeneration ? TEXT("true") : TEXT("false")));
			return true;
		}

		bool bExists = false;
		FString ExistingSha;
		FString ExistingLifecycle;
		if (!QueryExistingChunkSha(Database, Chunk.ChunkId, bExists, ExistingSha, ExistingLifecycle))
		{
			Report.Errors.Add(FString::Printf(TEXT("Failed to query existing chunk sha for %s."), *Chunk.ChunkId));
			return false;
		}

		const EIISChunkUpsertAction Action = FIISChunkCatalog::ClassifyUpsert(bExists, ExistingSha, Chunk.TextSha256);
		if (Action == EIISChunkUpsertAction::NoOp)
		{
			++Report.Summary.UnchangedChunkCount;
			if (IsStaleLifecycleState(ExistingLifecycle))
			{
				if (!ReactivateChunkLifecycle(Database, Chunk.ChunkId, Report.Errors))
				{
					return false;
				}
				bOutUpdated = true;
			}
			return true;
		}

		if (Action == EIISChunkUpsertAction::Conflict)
		{
			FIISImportConflict Conflict;
			Conflict.ChunkId = Chunk.ChunkId;
			Conflict.OldSha = ExistingSha;
			Conflict.NewSha = Chunk.TextSha256;
			Conflict.SourceId = Chunk.SourceId;
			Conflict.Policy = TEXT("overwrite-in-place");
			OutConflicts.Add(Conflict);

			if (!UpdateChunkRowOnConflict(Database, Chunk, ImportedAtUtc, Report.Errors))
			{
				return false;
			}
			if (!DeleteChunkChildRows(Database, Chunk.ChunkId, Report.Errors))
			{
				return false;
			}
			if (bFtsActive)
			{
				DeleteFtsRow(Database, Chunk.ChunkId, Report.Warnings);
			}
			InsertLabels(Database, Chunk, Report.Errors);
			InsertGroups(Database, Chunk, Report.Errors);
			InsertSourceRefs(Database, Chunk, Report.Errors);
			if (bFtsActive)
			{
				InsertFtsRow(Database, Chunk, Report.Warnings);
			}
			bOutUpdated = true;
			return true;
		}

		if (!InsertChunkRow(Database, Chunk, ImportedAtUtc, Report.Errors))
		{
			++Report.Summary.SkippedChunkCount;
			return false;
		}
		InsertLabels(Database, Chunk, Report.Errors);
		InsertGroups(Database, Chunk, Report.Errors);
		InsertSourceRefs(Database, Chunk, Report.Errors);
		if (bFtsActive)
		{
			InsertFtsRow(Database, Chunk, Report.Warnings);
		}
		bOutInserted = true;
		return true;
	}

	void ReadChunkAuxiliaryTables(FSQLiteDatabase& Database, FIISIndexChunk& Chunk)
	{
		{
			FSQLitePreparedStatement Statement = Database.PrepareStatement(TEXT("SELECT label FROM chunk_labels WHERE chunk_id = ?1 ORDER BY label;"));
			if (Statement.IsValid())
			{
				Statement.SetBindingValueByIndex(1, Chunk.ChunkId);
				Statement.Execute([&Chunk](const FSQLitePreparedStatement& Row)
				{
					FString Label;
					Row.GetColumnValueByName(TEXT("label"), Label);
					if (!Label.IsEmpty())
					{
						Chunk.RetrievalLabels.Add(Label);
					}
					return ESQLitePreparedStatementExecuteRowResult::Continue;
				});
			}
		}

		{
			FSQLitePreparedStatement Statement = Database.PrepareStatement(TEXT("SELECT group_id FROM chunk_groups WHERE chunk_id = ?1 ORDER BY group_id;"));
			if (Statement.IsValid())
			{
				Statement.SetBindingValueByIndex(1, Chunk.ChunkId);
				Statement.Execute([&Chunk](const FSQLitePreparedStatement& Row)
				{
					FString Group;
					Row.GetColumnValueByName(TEXT("group_id"), Group);
					if (!Group.IsEmpty())
					{
						Chunk.RetrievalGroups.Add(Group);
					}
					return ESQLitePreparedStatementExecuteRowResult::Continue;
				});
			}
		}

		{
			FSQLitePreparedStatement Statement = Database.PrepareStatement(
				TEXT("SELECT artifact_kind, relative_path, json_pointer, fingerprint, explanation ")
				TEXT("FROM chunk_source_refs WHERE chunk_id = ?1 ORDER BY rowid;"));
			if (Statement.IsValid())
			{
				Statement.SetBindingValueByIndex(1, Chunk.ChunkId);
				Statement.Execute([&Chunk](const FSQLitePreparedStatement& Row)
				{
					FIISSourceReference Reference;
					Row.GetColumnValueByName(TEXT("artifact_kind"), Reference.ArtifactKind);
					Row.GetColumnValueByName(TEXT("relative_path"), Reference.RelativePath);
					Row.GetColumnValueByName(TEXT("json_pointer"), Reference.JsonPointer);
					Row.GetColumnValueByName(TEXT("fingerprint"), Reference.Fingerprint);
					Row.GetColumnValueByName(TEXT("explanation"), Reference.Explanation);
					Chunk.SourceReferences.Add(Reference);
					return ESQLitePreparedStatementExecuteRowResult::Continue;
				});
			}
		}
	}

	bool ReadChunkFromCurrentRow(const FSQLitePreparedStatement& Row, FIISIndexChunk& OutChunk)
	{
		OutChunk = FIISIndexChunk();
		FString ChunkKind;
		FString Sensitivity;
		int32 bIsAiGenerated = 0;
		int32 bAllowsMigrationDecision = 0;
		int32 bAllowsPatchGeneration = 0;

		Row.GetColumnValueByName(TEXT("chunk_id"), OutChunk.ChunkId);
		Row.GetColumnValueByName(TEXT("source_chunk_id"), OutChunk.SourceChunkId);
		Row.GetColumnValueByName(TEXT("chunk_kind"), ChunkKind);
		Row.GetColumnValueByName(TEXT("sensitivity"), Sensitivity);
		Row.GetColumnValueByName(TEXT("title"), OutChunk.Title);
		Row.GetColumnValueByName(TEXT("text"), OutChunk.Text);
		Row.GetColumnValueByName(TEXT("module_name"), OutChunk.ModuleName);
		Row.GetColumnValueByName(TEXT("source_id"), OutChunk.SourceId);
		Row.GetColumnValueByName(TEXT("source_run_id"), OutChunk.SourceRunId);
		Row.GetColumnValueByName(TEXT("destination_run_id"), OutChunk.DestinationRunId);
		Row.GetColumnValueByName(TEXT("is_ai_generated"), bIsAiGenerated);
		Row.GetColumnValueByName(TEXT("allows_migration_decision"), bAllowsMigrationDecision);
		Row.GetColumnValueByName(TEXT("allows_patch_generation"), bAllowsPatchGeneration);
		Row.GetColumnValueByName(TEXT("text_sha256"), OutChunk.TextSha256);
		Row.GetColumnValueByName(TEXT("lifecycle_state"), OutChunk.LifecycleState);
		Row.GetColumnValueByName(TEXT("source_refs_hash"), OutChunk.SourceRefsHash);

		OutChunk.ChunkKind = ParseChunkKind(ChunkKind);
		OutChunk.Sensitivity = ParseSensitivity(Sensitivity);
		OutChunk.bIsAiGenerated = bIsAiGenerated != 0;
		OutChunk.bAllowsMigrationDecision = bAllowsMigrationDecision != 0;
		OutChunk.bAllowsPatchGeneration = bAllowsPatchGeneration != 0;
		return !OutChunk.ChunkId.IsEmpty();
	}

	bool LoadChunkByIdFromOpenDatabase(FSQLiteDatabase& Database, const FString& ChunkId, FIISIndexChunk& OutChunk)
	{
		FSQLitePreparedStatement Statement = Database.PrepareStatement(
			TEXT("SELECT chunk_id, source_chunk_id, chunk_kind, sensitivity, title, text, module_name, source_id, source_run_id, destination_run_id, ")
			TEXT("is_ai_generated, allows_migration_decision, allows_patch_generation, text_sha256, lifecycle_state, source_refs_hash ")
			TEXT("FROM chunks WHERE chunk_id = ?1;"));
		if (!Statement.IsValid())
		{
			return false;
		}

		Statement.SetBindingValueByIndex(1, ChunkId);
		bool bFound = false;
		Statement.Execute([&OutChunk, &bFound](const FSQLitePreparedStatement& Row)
		{
			bFound = ReadChunkFromCurrentRow(Row, OutChunk);
			return ESQLitePreparedStatementExecuteRowResult::Stop;
		});

		if (bFound)
		{
			ReadChunkAuxiliaryTables(Database, OutChunk);
		}

		return bFound;
	}

	TArray<FString> MakeSearchTerms(const FString& QueryText)
	{
		TArray<FString> RawTerms;
		QueryText.ParseIntoArrayWS(RawTerms);

		TArray<FString> Terms;
		for (FString Term : RawTerms)
		{
			Term.TrimStartAndEndInline();
			while (!Term.IsEmpty() && !FChar::IsAlnum(Term[0]))
			{
				Term.RightChopInline(1);
			}
			while (!Term.IsEmpty() && !FChar::IsAlnum(Term[Term.Len() - 1]))
			{
				Term.LeftInline(Term.Len() - 1);
			}
			if (!Term.IsEmpty())
			{
				Terms.Add(Term.ToLower());
			}
		}

		if (Terms.Num() == 0 && !QueryText.TrimStartAndEnd().IsEmpty())
		{
			Terms.Add(QueryText.ToLower());
		}

		return Terms;
	}

	bool ContainsAllRequiredLabels(const FIISIndexChunk& Chunk, const TArray<FString>& RequiredLabels)
	{
		for (const FString& RequiredLabel : RequiredLabels)
		{
			bool bFound = false;
			for (const FString& Label : Chunk.RetrievalLabels)
			{
				if (Label.Equals(RequiredLabel, ESearchCase::IgnoreCase))
				{
					bFound = true;
					break;
				}
			}

			if (!bFound)
			{
				return false;
			}
		}

		return true;
	}

	bool HasExcludedSensitivity(const FIISIndexChunk& Chunk, const TArray<FString>& ExcludedSensitivities)
	{
		const FString Sensitivity = ChunkSensitivityToString(Chunk.Sensitivity);
		for (const FString& ExcludedSensitivity : ExcludedSensitivities)
		{
			if (Sensitivity.Equals(ExcludedSensitivity, ESearchCase::IgnoreCase)
				|| NormalizeEnumToken(Sensitivity) == NormalizeEnumToken(ExcludedSensitivity))
			{
				return true;
			}
		}

		return false;
	}

	float AddPreferredGroupScore(const FIISIndexChunk& Chunk, const TArray<FString>& PreferredGroups)
	{
		float Score = 0.0f;
		for (const FString& PreferredGroup : PreferredGroups)
		{
			for (const FString& Group : Chunk.RetrievalGroups)
			{
				if (Group.Equals(PreferredGroup, ESearchCase::IgnoreCase))
				{
					Score += 1.0f;
				}
			}
		}
		return Score;
	}

	float ComputeHybridPreferredGroupBonus(const FIISIndexChunk& Chunk, const TArray<FString>& PreferredGroups)
	{
		for (const FString& PreferredGroup : PreferredGroups)
		{
			for (const FString& Group : Chunk.RetrievalGroups)
			{
				if (Group.Equals(PreferredGroup, ESearchCase::IgnoreCase))
				{
					return PreferredGroupHybridBonus;
				}
			}
		}
		return 0.0f;
	}

	float ComputeExactTitleMatchBonus(const FIISIndexChunk& Chunk, const FString& QueryText)
	{
		return !QueryText.TrimStartAndEnd().IsEmpty()
			&& Chunk.Title.TrimStartAndEnd().Equals(QueryText.TrimStartAndEnd(), ESearchCase::IgnoreCase)
			? ExactTitleMatchHybridBonus
			: 0.0f;
	}

	TArray<FString> CollectMatchedLabels(const FIISIndexChunk& Chunk, const FIISSearchQuery& Query)
	{
		TArray<FString> Matches;
		const TArray<FString> Terms = MakeSearchTerms(Query.QueryText);
		for (const FString& Label : Chunk.RetrievalLabels)
		{
			for (const FString& RequiredLabel : Query.RequiredLabels)
			{
				if (Label.Equals(RequiredLabel, ESearchCase::IgnoreCase))
				{
					Matches.AddUnique(Label);
				}
			}
			for (const FString& Term : Terms)
			{
				if (Label.Contains(Term, ESearchCase::IgnoreCase))
				{
					Matches.AddUnique(Label);
				}
			}
		}
		return Matches;
	}

	TArray<FString> CollectMatchedGroups(const FIISIndexChunk& Chunk, const FIISSearchQuery& Query)
	{
		TArray<FString> Matches;
		const TArray<FString> Terms = MakeSearchTerms(Query.QueryText);
		for (const FString& Group : Chunk.RetrievalGroups)
		{
			for (const FString& PreferredGroup : Query.PreferredGroups)
			{
				if (Group.Equals(PreferredGroup, ESearchCase::IgnoreCase))
				{
					Matches.AddUnique(Group);
				}
			}
			for (const FString& Term : Terms)
			{
				if (Group.Contains(Term, ESearchCase::IgnoreCase))
				{
					Matches.AddUnique(Group);
				}
			}
		}
		return Matches;
	}

	FString MakeSnippet(const FIISIndexChunk& Chunk, const FString& QueryText)
	{
		const FString Source = Chunk.Text.IsEmpty() ? Chunk.Title : Chunk.Text;
		if (Source.Len() <= 240)
		{
			return Source;
		}

		const int32 FoundIndex = Source.Find(QueryText, ESearchCase::IgnoreCase);
		const int32 StartIndex = FoundIndex == INDEX_NONE ? 0 : FMath::Max(0, FoundIndex - 80);
		return Source.Mid(StartIndex, 240);
	}

	float ScoreChunkFallback(
		const FIISIndexChunk& Chunk,
		const TArray<FString>& Terms,
		const FIISSearchQuery& Query,
		FString& OutExplanation)
	{
		float Score = 0.0f;
		int32 TitleHits = 0;
		int32 TextHits = 0;
		int32 LabelGroupHits = 0;

		for (const FString& Term : Terms)
		{
			if (Chunk.Title.Contains(Term, ESearchCase::IgnoreCase))
			{
				Score += 2.0f;
				++TitleHits;
			}
			if (Chunk.Text.Contains(Term, ESearchCase::IgnoreCase))
			{
				Score += 1.0f;
				++TextHits;
			}
			for (const FString& Label : Chunk.RetrievalLabels)
			{
				if (Label.Contains(Term, ESearchCase::IgnoreCase))
				{
					Score += 1.0f;
					++LabelGroupHits;
				}
			}
			for (const FString& Group : Chunk.RetrievalGroups)
			{
				if (Group.Contains(Term, ESearchCase::IgnoreCase))
				{
					Score += 1.0f;
					++LabelGroupHits;
				}
			}
		}

		const float PreferredScore = AddPreferredGroupScore(Chunk, Query.PreferredGroups);
		Score += PreferredScore;

		OutExplanation = FString::Printf(
			TEXT("Fallback lexical score: title_hits=%d, text_hits=%d, label_group_hits=%d, preferred_group_bonus=%.1f."),
			TitleHits,
			TextHits,
			LabelGroupHits,
			PreferredScore);
		return Score;
	}

	bool LoadAllChunksFromCatalog(FSQLiteDatabase& Database, TArray<FIISIndexChunk>& OutChunks)
	{
		bool bSuccess = true;
		Database.Execute(
			TEXT("SELECT chunk_id, source_chunk_id, chunk_kind, sensitivity, title, text, module_name, source_id, source_run_id, destination_run_id, ")
			TEXT("is_ai_generated, allows_migration_decision, allows_patch_generation, text_sha256, lifecycle_state, source_refs_hash ")
			TEXT("FROM chunks ORDER BY chunk_id;"),
			[&Database, &OutChunks, &bSuccess](const FSQLitePreparedStatement& Row)
			{
				FIISIndexChunk Chunk;
				if (!ReadChunkFromCurrentRow(Row, Chunk))
				{
					bSuccess = false;
					return ESQLitePreparedStatementExecuteRowResult::Error;
				}

				ReadChunkAuxiliaryTables(Database, Chunk);
				OutChunks.Add(MoveTemp(Chunk));
				return ESQLitePreparedStatementExecuteRowResult::Continue;
			});

		return bSuccess;
	}

	bool ApplySearchFilters(const FIISIndexChunk& Chunk, const FIISSearchQuery& Query)
	{
		return FIISChunkCatalog::IsRetrievableLifecycleState(Chunk.LifecycleState)
			&& ContainsAllRequiredLabels(Chunk, Query.RequiredLabels)
			&& !HasExcludedSensitivity(Chunk, Query.ExcludedSensitivities)
			&& !Chunk.bIsAiGenerated
			&& !Chunk.bAllowsMigrationDecision
			&& !Chunk.bAllowsPatchGeneration;
	}

	FIISSearchResult MakeSearchResult(
		const FIISIndexChunk& Chunk,
		const FString& QueryText,
		const float Score,
		const FString& ScoreExplanation)
	{
		FIISSearchResult Result;
		Result.ChunkId = Chunk.ChunkId;
		Result.Title = Chunk.Title;
		Result.Snippet = MakeSnippet(Chunk, QueryText);
		Result.Score = Score;
		Result.ScoreExplanation = ScoreExplanation;
		Result.Chunk = Chunk;
		return Result;
	}

	struct FIISVectorRecord
	{
		FString ChunkId;
		FString RouteId;
		FString TaskKind;
		FString ProviderId;
		FString ModelId;
		FString TextSha256;
		int32 Dimensions = 0;
		TArray<float> Vector;
	};

	struct FIISQueryVector
	{
		FString RouteId;
		FString ProviderId;
		FString ModelId;
		int32 Dimensions = 0;
		TArray<float> Vector;
	};

	FString GetVectorStorePath()
	{
		return FIISStoragePaths::GetVectorsDir() / TEXT("chunk_vectors.jsonl");
	}

	bool LoadVectorRecords(TArray<FIISVectorRecord>& OutRecords, FIISSearchResponse& OutResponse)
	{
		OutRecords.Reset();

		const FString VectorStorePath = GetVectorStorePath();
		if (!FPaths::FileExists(VectorStorePath))
		{
			OutResponse.Warnings.Add(FString::Printf(TEXT("IIS vector store does not exist yet: %s"), *VectorStorePath));
			return true;
		}

		TArray<FString> Lines;
		if (!FFileHelper::LoadFileToStringArray(Lines, *VectorStorePath))
		{
			OutResponse.Errors.Add(FString::Printf(TEXT("Failed to read IIS vector store: %s"), *VectorStorePath));
			return false;
		}

		int32 LineNumber = 0;
		for (const FString& Line : Lines)
		{
			++LineNumber;
			const FString Trimmed = Line.TrimStartAndEnd();
			if (Trimmed.IsEmpty())
			{
				continue;
			}

			TSharedPtr<FJsonObject> Object;
			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Trimmed);
			if (!FJsonSerializer::Deserialize(Reader, Object) || !Object.IsValid())
			{
				OutResponse.Warnings.Add(FString::Printf(TEXT("Skipping malformed vector JSONL line %d."), LineNumber));
				continue;
			}

			FIISVectorRecord Record;
			Object->TryGetStringField(TEXT("chunk_id"), Record.ChunkId);
			Object->TryGetStringField(TEXT("route_id"), Record.RouteId);
			Object->TryGetStringField(TEXT("task_kind"), Record.TaskKind);
			Object->TryGetStringField(TEXT("provider_id"), Record.ProviderId);
			Object->TryGetStringField(TEXT("model_id"), Record.ModelId);
			Object->TryGetStringField(TEXT("text_sha256"), Record.TextSha256);
			{
				double DimensionsDouble = 0.0;
				Object->TryGetNumberField(TEXT("dimensions"), DimensionsDouble);
				Record.Dimensions = static_cast<int32>(DimensionsDouble);
			}

			const TArray<TSharedPtr<FJsonValue>>* VectorValues = nullptr;
			if (Object->TryGetArrayField(TEXT("vector"), VectorValues) && VectorValues)
			{
				for (const TSharedPtr<FJsonValue>& Value : *VectorValues)
				{
					double Number = 0.0;
					if (Value.IsValid() && Value->TryGetNumber(Number))
					{
						Record.Vector.Add(static_cast<float>(Number));
					}
				}
			}

			if (Record.Dimensions <= 0)
			{
				Record.Dimensions = Record.Vector.Num();
			}

			if (Record.ChunkId.IsEmpty() || Record.RouteId.IsEmpty() || Record.Vector.Num() == 0)
			{
				OutResponse.Warnings.Add(FString::Printf(TEXT("Skipping incomplete vector record at line %d."), LineNumber));
				continue;
			}

			OutRecords.Add(MoveTemp(Record));
		}

		return true;
	}

	FIISVectorRecordIn ToVectorRecordIn(const FIISVectorRecord& Record)
	{
		FIISVectorRecordIn Out;
		Out.ChunkId = Record.ChunkId;
		Out.RouteId = Record.RouteId;
		Out.ModelId = Record.ModelId;
		Out.TextSha256 = Record.TextSha256;
		Out.Dimensions = Record.Dimensions;
		Out.Vector = Record.Vector;
		return Out;
	}

	int32 CollectVectorSearchResultsFromBackend(
		FSQLiteDatabase& Database,
		const FIISSearchQuery& Query,
		const TArray<FIISVectorRecord>& VectorRecords,
		const TMap<FString, FIISQueryVector>& QueryVectorsByRoute,
		IIISVectorIndexBackend& Backend,
		FIISSearchResponse& OutResponse)
	{
		const int32 MaxResults = FMath::Clamp(Query.MaxResults <= 0 ? 10 : Query.MaxResults, 1, 100);

		TMap<FString, TArray<const FIISVectorRecord*>> RecordsByRoute;
		for (const FIISVectorRecord& Record : VectorRecords)
		{
			RecordsByRoute.FindOrAdd(Record.RouteId).Add(&Record);
		}

		int32 ComparedVectorCount = 0;
		TArray<FString> BackendWarnings;

		for (const TPair<FString, TArray<const FIISVectorRecord*>>& RoutePair : RecordsByRoute)
		{
			const FString& RouteId = RoutePair.Key;
			const FIISQueryVector* QueryVector = QueryVectorsByRoute.Find(RouteId);
			if (!QueryVector)
			{
				continue;
			}

			TArray<FIISVectorRecordIn> RouteRecords;
			TMap<FString, const FIISVectorRecord*> RecordByChunkId;
			for (const FIISVectorRecord* Record : RoutePair.Value)
			{
				if (QueryVector->Vector.Num() != Record->Vector.Num())
				{
					OutResponse.Warnings.Add(FString::Printf(
						TEXT("Skipping vector for chunk '%s' because query dimensions (%d) do not match stored vector dimensions (%d)."),
						*Record->ChunkId,
						QueryVector->Vector.Num(),
						Record->Vector.Num()));
					continue;
				}

				RouteRecords.Add(ToVectorRecordIn(*Record));
				RecordByChunkId.Add(Record->ChunkId, Record);
			}

			if (RouteRecords.Num() == 0)
			{
				continue;
			}

			Backend.Rebuild(RouteRecords, BackendWarnings);
			const int32 KCandidate = FMath::Max(MaxResults * 4, RouteRecords.Num());
			const TArray<FIISVectorHit> Hits = Backend.Search(QueryVector->Vector, KCandidate, BackendWarnings);

			for (const FIISVectorHit& Hit : Hits)
			{
				const FIISVectorRecord* const* RecordPtr = RecordByChunkId.Find(Hit.ChunkId);
				if (!RecordPtr || !*RecordPtr)
				{
					continue;
				}
				const FIISVectorRecord& Record = **RecordPtr;

				FIISIndexChunk Chunk;
				if (!LoadChunkByIdFromOpenDatabase(Database, Record.ChunkId, Chunk) || !ApplySearchFilters(Chunk, Query))
				{
					continue;
				}

				const float Cosine = Hit.Score;
				const float PreferredScore = AddPreferredGroupScore(Chunk, Query.PreferredGroups);
				const float Score = ((Cosine + 1.0f) * 50.0f) + PreferredScore;
				const FString Explanation = FString::Printf(
					TEXT("Vector cosine similarity=%.6f, normalized_score=%.3f, route_id=%s, provider_id=%s, model_id=%s, dimensions=%d, preferred_group_bonus=%.1f."),
					Cosine,
					(Cosine + 1.0f) * 50.0f,
					*Record.RouteId,
					*QueryVector->ProviderId,
					*QueryVector->ModelId,
					QueryVector->Vector.Num(),
					PreferredScore);
				FIISSearchResult Result = MakeSearchResult(Chunk, Query.QueryText, Score, Explanation);
				Result.VectorScore = Score;
				Result.MatchedLabels = CollectMatchedLabels(Chunk, Query);
				Result.MatchedGroups = CollectMatchedGroups(Chunk, Query);
				OutResponse.Results.Add(MoveTemp(Result));
				++ComparedVectorCount;
			}
		}

		OutResponse.Warnings.Append(BackendWarnings);
		return ComparedVectorCount;
	}

	void FinalizeVectorSearchResponse(
		const FIISSearchQuery& Query,
		int32 ComparedVectorCount,
		FIISSearchResponse& OutResponse)
	{
		OutResponse.VectorCandidateCount = ComparedVectorCount;

		OutResponse.Results.Sort([](const FIISSearchResult& Left, const FIISSearchResult& Right)
		{
			if (!FMath::IsNearlyEqual(Left.Score, Right.Score))
			{
				return Left.Score > Right.Score;
			}
			return Left.ChunkId < Right.ChunkId;
		});

		const int32 MaxResults = FMath::Clamp(Query.MaxResults <= 0 ? 10 : Query.MaxResults, 1, 100);
		if (OutResponse.Results.Num() > MaxResults)
		{
			OutResponse.Results.SetNum(MaxResults);
		}

		OutResponse.MergedCandidateCount = ComparedVectorCount;
		OutResponse.FinalResultCount = OutResponse.Results.Num();
		OutResponse.DiagnosticsSummary = FString::Printf(
			TEXT("mode=Vector query='%s' vector_candidates=%d final_results=%d warnings=%d"),
			*Query.QueryText,
			OutResponse.VectorCandidateCount,
			OutResponse.FinalResultCount,
			OutResponse.Warnings.Num());

		if (ComparedVectorCount == 0)
		{
			OutResponse.Status = EIISSearchStatus::Warning;
			OutResponse.Warnings.Add(TEXT("No stored vectors survived route, dimension, and search filter checks."));
		}
		else if (OutResponse.Results.Num() > 0)
		{
			OutResponse.Status = OutResponse.Warnings.Num() > 0 ? EIISSearchStatus::Warning : EIISSearchStatus::Ready;
		}
		else
		{
			OutResponse.Status = OutResponse.Warnings.Num() > 0 ? EIISSearchStatus::Warning : EIISSearchStatus::Empty;
		}
	}

	bool ExecuteQueryEmbeddingForRoute(
		IIISEmbeddingRouteExecutor& EmbeddingExecutor,
		const FString& RouteId,
		const FIISSearchQuery& Query,
		FIISQueryVector& OutQueryVector,
		FIISSearchResponse& OutResponse)
	{
		FIISEmbeddingRequest Request;
		Request.RouteId = RouteId;
		Request.TaskKind = RouteId;
		Request.InputText = Query.QueryText;
		Request.bLocalOnly = true;
		Request.bNormalize = true;
		Request.Metadata.Add(TEXT("iis_search_mode"), TEXT("Vector"));

		FIISEmbeddingResponse Response;
		EmbeddingExecutor.ExecuteEmbeddingRoute(Request, Response);
		if (!Response.bSuccess)
		{
			const FString ErrorCode = Response.ErrorCode.IsEmpty() ? TEXT("embedding_provider_failed") : Response.ErrorCode;
			OutResponse.Warnings.Add(FString::Printf(
				TEXT("Vector query embedding failed for route '%s' (%s): %s"),
				*RouteId,
				*ErrorCode,
				*Response.ErrorMessage));
			return false;
		}

		OutQueryVector.RouteId = RouteId;
		OutQueryVector.ProviderId = Response.ProviderId;
		OutQueryVector.ModelId = Response.ModelId;
		OutQueryVector.Vector = Response.Vector;
		OutQueryVector.Dimensions = Response.Dimensions > 0 ? Response.Dimensions : Response.Vector.Num();

		if (OutQueryVector.Vector.Num() == 0 || OutQueryVector.Dimensions <= 0)
		{
			OutResponse.Warnings.Add(FString::Printf(TEXT("Vector query embedding for route '%s' returned an empty vector."), *RouteId));
			return false;
		}

		return true;
	}

	bool SearchVector(FSQLiteDatabase& Database, const FIISSearchQuery& Query, FIISSearchResponse& OutResponse)
	{
		const FString TrimmedQuery = Query.QueryText.TrimStartAndEnd();
		if (TrimmedQuery.IsEmpty())
		{
			OutResponse.Status = EIISSearchStatus::Warning;
			OutResponse.Warnings.Add(TEXT("Vector search query is empty; returning a controlled empty response."));
			return true;
		}

		TArray<FIISVectorRecord> VectorRecords;
		if (!LoadVectorRecords(VectorRecords, OutResponse))
		{
			OutResponse.Status = EIISSearchStatus::Error;
			return false;
		}
		if (VectorRecords.Num() == 0)
		{
			OutResponse.Status = EIISSearchStatus::Warning;
			OutResponse.Warnings.Add(TEXT("IIS vector store contains no usable vectors; run embedding jobs first."));
			return true;
		}

		TSharedPtr<IIISEmbeddingRouteExecutor> EmbeddingExecutor = FIISEmbeddingRouteExecutorRegistry::GetExecutor();
		if (!EmbeddingExecutor)
		{
			OutResponse.Status = EIISSearchStatus::Warning;
			OutResponse.Warnings.Add(TEXT("No IIS embedding route executor is registered; vector search could not embed the query."));
			return true;
		}

		TArray<FString> RouteIds;
		for (const FIISVectorRecord& Record : VectorRecords)
		{
			RouteIds.AddUnique(Record.RouteId);
		}

		TMap<FString, FIISQueryVector> QueryVectorsByRoute;
		for (const FString& RouteId : RouteIds)
		{
			FIISQueryVector QueryVector;
			if (ExecuteQueryEmbeddingForRoute(*EmbeddingExecutor, RouteId, Query, QueryVector, OutResponse))
			{
				QueryVectorsByRoute.Add(RouteId, MoveTemp(QueryVector));
			}
		}

		if (QueryVectorsByRoute.Num() == 0)
		{
			OutResponse.Status = EIISSearchStatus::Warning;
			OutResponse.Warnings.Add(TEXT("No query embedding routes succeeded; vector search returned no results."));
			return true;
		}

		TUniquePtr<IIISVectorIndexBackend> Backend = CreateVectorIndexBackend();
		const int32 ComparedVectorCount = CollectVectorSearchResultsFromBackend(
			Database,
			Query,
			VectorRecords,
			QueryVectorsByRoute,
			*Backend,
			OutResponse);
		FinalizeVectorSearchResponse(Query, ComparedVectorCount, OutResponse);
		return true;
	}

	bool SearchFallback(FSQLiteDatabase& Database, const FIISSearchQuery& Query, FIISSearchResponse& OutResponse)
	{
		const TArray<FString> Terms = MakeSearchTerms(Query.QueryText);
		if (Terms.Num() == 0)
		{
			OutResponse.Status = EIISSearchStatus::Warning;
			OutResponse.Warnings.Add(TEXT("Search query is empty; returning a controlled empty response."));
			return true;
		}

		TArray<FIISIndexChunk> Chunks;
		if (!LoadAllChunksFromCatalog(Database, Chunks))
		{
			OutResponse.Status = EIISSearchStatus::Error;
			OutResponse.Errors.Add(TEXT("Failed to load chunks from IIS catalog."));
			return false;
		}

		for (const FIISIndexChunk& Chunk : Chunks)
		{
			if (!ApplySearchFilters(Chunk, Query))
			{
				continue;
			}

			FString ScoreExplanation;
			const float Score = ScoreChunkFallback(Chunk, Terms, Query, ScoreExplanation);
			if (Score <= 0.0f)
			{
				continue;
			}

			FIISSearchResult Result = MakeSearchResult(Chunk, Query.QueryText, Score, ScoreExplanation);
			Result.LexicalScore = Score;
			Result.MatchedLabels = CollectMatchedLabels(Chunk, Query);
			Result.MatchedGroups = CollectMatchedGroups(Chunk, Query);
			OutResponse.Results.Add(MoveTemp(Result));
		}

		OutResponse.LexicalCandidateCount = OutResponse.Results.Num();

		OutResponse.Results.Sort([](const FIISSearchResult& Left, const FIISSearchResult& Right)
		{
			if (!FMath::IsNearlyEqual(Left.Score, Right.Score))
			{
				return Left.Score > Right.Score;
			}
			return Left.ChunkId < Right.ChunkId;
		});

		const int32 MaxResults = FMath::Clamp(Query.MaxResults <= 0 ? 10 : Query.MaxResults, 1, 100);
		if (OutResponse.Results.Num() > MaxResults)
		{
			OutResponse.Results.SetNum(MaxResults);
		}

		OutResponse.FinalResultCount = OutResponse.Results.Num();
		OutResponse.MergedCandidateCount = OutResponse.Results.Num();
		OutResponse.DiagnosticsSummary = FString::Printf(
			TEXT("mode=Lexical query='%s' lexical_candidates=%d final_results=%d warnings=%d"),
			*Query.QueryText,
			OutResponse.LexicalCandidateCount,
			OutResponse.FinalResultCount,
			OutResponse.Warnings.Num());
		OutResponse.Status = OutResponse.Results.Num() > 0 ? EIISSearchStatus::Ready : EIISSearchStatus::Empty;
		return true;
	}

	FString MakeFtsQuery(const FString& QueryText)
	{
		TArray<FString> Terms = MakeSearchTerms(QueryText);
		for (FString& Term : Terms)
		{
			Term += TEXT("*");
		}
		return FString::Join(Terms, TEXT(" "));
	}

	bool IsFtsTableAvailable(FSQLiteDatabase& Database)
	{
		int64 Count = 0;
		Database.Execute(
			TEXT("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='chunks_fts';"),
			[&Count](const FSQLitePreparedStatement& Row)
			{
				Row.GetColumnValueByIndex(0, Count);
				return ESQLitePreparedStatementExecuteRowResult::Stop;
			});
		return Count > 0;
	}

	bool SearchFts(FSQLiteDatabase& Database, const FIISSearchQuery& Query, FIISSearchResponse& OutResponse)
	{
		if (!IsFtsTableAvailable(Database))
		{
			return false;
		}

		const FString FtsQuery = MakeFtsQuery(Query.QueryText);
		if (FtsQuery.IsEmpty())
		{
			return false;
		}

		FSQLitePreparedStatement Statement = Database.PrepareStatement(
			TEXT("SELECT chunks_fts.chunk_id AS chunk_id, bm25(chunks_fts) AS rank ")
			TEXT("FROM chunks_fts JOIN chunks ON chunks_fts.chunk_id = chunks.chunk_id ")
			TEXT("WHERE chunks_fts MATCH ?1 ORDER BY rank LIMIT ?2;"));
		if (!Statement.IsValid())
		{
			return false;
		}

		const int32 MaxResults = FMath::Clamp(Query.MaxResults <= 0 ? 10 : Query.MaxResults, 1, 100);
		Statement.SetBindingValueByIndex(1, FtsQuery);
		Statement.SetBindingValueByIndex(2, MaxResults * 4);

		bool bExecuted = true;
		Statement.Execute([&Database, &Query, &OutResponse, &MaxResults](const FSQLitePreparedStatement& Row)
		{
			FString ChunkId;
			double Rank = 0.0;
			Row.GetColumnValueByName(TEXT("chunk_id"), ChunkId);
			Row.GetColumnValueByName(TEXT("rank"), Rank);

			FIISIndexChunk Chunk;
			if (!LoadChunkByIdFromOpenDatabase(Database, ChunkId, Chunk) || !ApplySearchFilters(Chunk, Query))
			{
				return ESQLitePreparedStatementExecuteRowResult::Continue;
			}

			const float PreferredScore = AddPreferredGroupScore(Chunk, Query.PreferredGroups);
			const float Score = static_cast<float>(1000.0 / (1.0 + FMath::Abs(Rank))) + PreferredScore;
			const FString Explanation = FString::Printf(
				TEXT("SQLite FTS5 bm25 rank=%.6f, preferred_group_bonus=%.1f."),
				Rank,
				PreferredScore);
			FIISSearchResult Result = MakeSearchResult(Chunk, Query.QueryText, Score, Explanation);
			Result.LexicalScore = Score;
			Result.MatchedLabels = CollectMatchedLabels(Chunk, Query);
			Result.MatchedGroups = CollectMatchedGroups(Chunk, Query);
			OutResponse.Results.Add(MoveTemp(Result));

			return OutResponse.Results.Num() >= MaxResults
				? ESQLitePreparedStatementExecuteRowResult::Stop
				: ESQLitePreparedStatementExecuteRowResult::Continue;
		});

		if (!bExecuted)
		{
			return false;
		}

		OutResponse.LexicalCandidateCount = OutResponse.Results.Num();
		OutResponse.MergedCandidateCount = OutResponse.Results.Num();
		OutResponse.FinalResultCount = OutResponse.Results.Num();
		OutResponse.DiagnosticsSummary = FString::Printf(
			TEXT("mode=Lexical query='%s' lexical_candidates=%d final_results=%d warnings=%d"),
			*Query.QueryText,
			OutResponse.LexicalCandidateCount,
			OutResponse.FinalResultCount,
			OutResponse.Warnings.Num());
		OutResponse.Status = OutResponse.Results.Num() > 0 ? EIISSearchStatus::Ready : EIISSearchStatus::Empty;
		return OutResponse.Results.Num() > 0;
	}

	struct FIISHybridCandidate
	{
		FString ChunkId;
		FIISIndexChunk Chunk;
		FString Snippet;
		float RawLexicalScore = 0.0f;
		float RawVectorScore = 0.0f;
		float NormalizedLexicalScore = 0.0f;
		float NormalizedVectorScore = 0.0f;
		float PreferredGroupBonus = 0.0f;
		float ExactTitleBonus = 0.0f;
		float HybridScore = 0.0f;
		bool bHasLexical = false;
		bool bHasVector = false;
		FString LexicalExplanation;
		FString VectorExplanation;
	};

	void AddHybridSourceResult(
		TMap<FString, FIISHybridCandidate>& CandidatesByChunkId,
		const FIISSearchResult& Result,
		const bool bIsLexical)
	{
		FIISHybridCandidate& Candidate = CandidatesByChunkId.FindOrAdd(Result.ChunkId);
		if (Candidate.ChunkId.IsEmpty())
		{
			Candidate.ChunkId = Result.ChunkId;
			Candidate.Chunk = Result.Chunk;
			Candidate.Snippet = Result.Snippet;
		}

		if (bIsLexical)
		{
			Candidate.bHasLexical = true;
			Candidate.RawLexicalScore = FMath::Max(Candidate.RawLexicalScore, Result.Score);
			Candidate.LexicalExplanation = Result.ScoreExplanation;
		}
		else
		{
			Candidate.bHasVector = true;
			Candidate.RawVectorScore = FMath::Max(Candidate.RawVectorScore, Result.Score);
			Candidate.VectorExplanation = Result.ScoreExplanation;
		}
	}

	float ComputeMaxScore(const TArray<FIISSearchResult>& Results)
	{
		float MaxScore = 0.0f;
		for (const FIISSearchResult& Result : Results)
		{
			MaxScore = FMath::Max(MaxScore, Result.Score);
		}
		return MaxScore;
	}

	bool SearchHybrid(FSQLiteDatabase& Database, const FIISSearchQuery& Query, FIISSearchResponse& OutResponse)
	{
		FIISSearchQuery LexicalQuery = Query;
		LexicalQuery.SearchMode = EIISSearchMode::Lexical;
		LexicalQuery.MaxResults = MaxLexicalCandidatesBeforeMerge;

		FIISSearchResponse LexicalResponse;
		LexicalResponse.QueryText = Query.QueryText;
		const bool bUsedFts = SearchFts(Database, LexicalQuery, LexicalResponse);
		if (!bUsedFts)
		{
			SearchFallback(Database, LexicalQuery, LexicalResponse);
		}

		FIISSearchQuery VectorQuery = Query;
		VectorQuery.SearchMode = EIISSearchMode::Vector;
		VectorQuery.MaxResults = MaxVectorCandidatesBeforeMerge;

		FIISSearchResponse VectorResponse;
		VectorResponse.QueryText = Query.QueryText;
		SearchVector(Database, VectorQuery, VectorResponse);

		OutResponse.Warnings.Append(LexicalResponse.Warnings);
		OutResponse.Warnings.Append(VectorResponse.Warnings);
		OutResponse.Errors.Append(LexicalResponse.Errors);
		OutResponse.Errors.Append(VectorResponse.Errors);

		TMap<FString, FIISHybridCandidate> CandidatesByChunkId;
		for (const FIISSearchResult& Result : LexicalResponse.Results)
		{
			AddHybridSourceResult(CandidatesByChunkId, Result, true);
		}
		for (const FIISSearchResult& Result : VectorResponse.Results)
		{
			AddHybridSourceResult(CandidatesByChunkId, Result, false);
		}

		const float MaxLexicalScore = ComputeMaxScore(LexicalResponse.Results);
		const float MaxVectorScore = ComputeMaxScore(VectorResponse.Results);

		TArray<FIISHybridCandidate> Candidates;
		CandidatesByChunkId.GenerateValueArray(Candidates);
		for (FIISHybridCandidate& Candidate : Candidates)
		{
			Candidate.NormalizedLexicalScore = MaxLexicalScore > 0.0f
				? Candidate.RawLexicalScore / MaxLexicalScore
				: 0.0f;
			Candidate.NormalizedVectorScore = MaxVectorScore > 0.0f
				? Candidate.RawVectorScore / MaxVectorScore
				: 0.0f;
			Candidate.PreferredGroupBonus = ComputeHybridPreferredGroupBonus(Candidate.Chunk, Query.PreferredGroups);
			Candidate.ExactTitleBonus = ComputeExactTitleMatchBonus(Candidate.Chunk, Query.QueryText);
			Candidate.HybridScore = FMath::Clamp(
				DefaultHybridLexicalWeight * Candidate.NormalizedLexicalScore
				+ DefaultHybridVectorWeight * Candidate.NormalizedVectorScore
				+ Candidate.PreferredGroupBonus
				+ Candidate.ExactTitleBonus,
				0.0f,
				1.0f);
		}

		Candidates.Sort([](const FIISHybridCandidate& Left, const FIISHybridCandidate& Right)
		{
			if (!FMath::IsNearlyEqual(Left.HybridScore, Right.HybridScore))
			{
				return Left.HybridScore > Right.HybridScore;
			}
			return Left.ChunkId < Right.ChunkId;
		});

		const int32 MaxResults = FMath::Clamp(Query.MaxResults <= 0 ? 10 : Query.MaxResults, 1, 100);
		for (const FIISHybridCandidate& Candidate : Candidates)
		{
			if (OutResponse.Results.Num() >= MaxResults)
			{
				break;
			}

			const FString Explanation = FString::Printf(
				TEXT("Hybrid score %.3f = lexical %.3f*%.2f + vector %.3f*%.2f + preferred_group %.2f + exact_title %.2f. Raw lexical=%.3f (%s). Raw vector=%.3f (%s)."),
				Candidate.HybridScore,
				Candidate.NormalizedLexicalScore,
				DefaultHybridLexicalWeight,
				Candidate.NormalizedVectorScore,
				DefaultHybridVectorWeight,
				Candidate.PreferredGroupBonus,
				Candidate.ExactTitleBonus,
				Candidate.RawLexicalScore,
				Candidate.bHasLexical ? *Candidate.LexicalExplanation : TEXT("missing"),
				Candidate.RawVectorScore,
				Candidate.bHasVector ? *Candidate.VectorExplanation : TEXT("missing"));

			FIISSearchResult Result = MakeSearchResult(Candidate.Chunk, Query.QueryText, Candidate.HybridScore, Explanation);
			Result.Snippet = Candidate.Snippet.IsEmpty() ? Result.Snippet : Candidate.Snippet;
			Result.LexicalScore = Candidate.NormalizedLexicalScore;
			Result.VectorScore = Candidate.NormalizedVectorScore;
			Result.HybridScore = Candidate.HybridScore;
			Result.MatchedLabels = CollectMatchedLabels(Candidate.Chunk, Query);
			Result.MatchedGroups = CollectMatchedGroups(Candidate.Chunk, Query);
			OutResponse.Results.Add(MoveTemp(Result));
		}

		OutResponse.LexicalCandidateCount = LexicalResponse.Results.Num();
		OutResponse.VectorCandidateCount = VectorResponse.Results.Num();
		OutResponse.MergedCandidateCount = Candidates.Num();
		OutResponse.FinalResultCount = OutResponse.Results.Num();
		OutResponse.DiagnosticsSummary = FString::Printf(
			TEXT("mode=Hybrid query='%s' lexical_candidates=%d vector_candidates=%d merged_candidates=%d final_results=%d lexical_weight=%.2f vector_weight=%.2f warnings=%d"),
			*Query.QueryText,
			OutResponse.LexicalCandidateCount,
			OutResponse.VectorCandidateCount,
			OutResponse.MergedCandidateCount,
			OutResponse.FinalResultCount,
			DefaultHybridLexicalWeight,
			DefaultHybridVectorWeight,
			OutResponse.Warnings.Num());

		if (OutResponse.Errors.Num() > 0 && OutResponse.Results.Num() == 0)
		{
			OutResponse.Status = EIISSearchStatus::Error;
			return false;
		}

		if (OutResponse.Results.Num() > 0)
		{
			OutResponse.Status = OutResponse.Warnings.Num() > 0 ? EIISSearchStatus::Warning : EIISSearchStatus::Ready;
		}
		else
		{
			OutResponse.Status = OutResponse.Warnings.Num() > 0 ? EIISSearchStatus::Warning : EIISSearchStatus::Empty;
		}

		return OutResponse.Status != EIISSearchStatus::Error;
	}

	TSharedRef<FJsonObject> MakeCatalogSummaryObject(const FIISCatalogBuildSummary& Summary)
	{
		TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetStringField(TEXT("status"), CatalogStatusToString(Summary.Status));
		Object->SetNumberField(TEXT("source_chunk_count"), Summary.SourceChunkCount);
		Object->SetNumberField(TEXT("catalog_chunk_count"), Summary.CatalogChunkCount);
		Object->SetNumberField(TEXT("inserted_chunk_count"), Summary.InsertedChunkCount);
		Object->SetNumberField(TEXT("updated_chunk_count"), Summary.UpdatedChunkCount);
		Object->SetNumberField(TEXT("skipped_chunk_count"), Summary.SkippedChunkCount);
		Object->SetNumberField(TEXT("unchanged_chunk_count"), Summary.UnchangedChunkCount);
		Object->SetNumberField(TEXT("label_count"), Summary.LabelCount);
		Object->SetNumberField(TEXT("group_count"), Summary.GroupCount);
		Object->SetNumberField(TEXT("warning_count"), Summary.WarningCount);
		Object->SetNumberField(TEXT("error_count"), Summary.ErrorCount);
		return Object;
	}

	TSharedRef<FJsonObject> MakeCatalogReportObject(
		const FIISCatalogBuildReport& Report,
		const bool bSQLiteActive,
		const bool bFtsActive,
		const bool bFallbackSearchActive,
		const int32 SourceReferenceCount)
	{
		TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetStringField(TEXT("schema_version"), Report.SchemaVersion);
		Object->SetStringField(TEXT("tool_name"), Report.ToolName);
		Object->SetStringField(TEXT("generated_at_utc"), Report.GeneratedAtUtc);
		Object->SetStringField(TEXT("catalog_path"), Report.CatalogPath);
		Object->SetStringField(TEXT("source_chunk_store_path"), Report.SourceChunkStorePath);
		Object->SetObjectField(TEXT("summary"), MakeCatalogSummaryObject(Report.Summary));
		Object->SetBoolField(TEXT("sqlite_active"), bSQLiteActive);
		Object->SetBoolField(TEXT("fts_active"), bFtsActive);
		Object->SetBoolField(TEXT("fallback_search_active"), bFallbackSearchActive);
		Object->SetNumberField(TEXT("source_reference_count"), SourceReferenceCount);
		Object->SetArrayField(TEXT("warnings"), MakeStringArray(Report.Warnings));
		Object->SetArrayField(TEXT("errors"), MakeStringArray(Report.Errors));
		return Object;
	}

	bool WriteCatalogBuildReportJson(
		const FIISCatalogBuildReport& Report,
		const bool bSQLiteActive,
		const bool bFtsActive,
		const bool bFallbackSearchActive,
		const int32 SourceReferenceCount)
	{
		return SaveJsonObjectToFile(
			MakeCatalogReportObject(Report, bSQLiteActive, bFtsActive, bFallbackSearchActive, SourceReferenceCount),
			FIISStoragePaths::GetIndexesDir() / CatalogBuildReportJsonName);
	}

	bool WriteCatalogBuildReportMarkdown(
		const FIISCatalogBuildReport& Report,
		const bool bSQLiteActive,
		const bool bFtsActive,
		const bool bFallbackSearchActive,
		const int32 SourceReferenceCount)
	{
		TArray<FString> Lines;
		Lines.Add(TEXT("# IIS Chunk Catalog Build Report"));
		Lines.Add(TEXT(""));
		Lines.Add(FString::Printf(TEXT("- GeneratedAtUtc: %s"), *Report.GeneratedAtUtc));
		Lines.Add(FString::Printf(TEXT("- Status: %s"), *CatalogStatusToString(Report.Summary.Status)));
		Lines.Add(FString::Printf(TEXT("- CatalogPath: %s"), *Report.CatalogPath));
		Lines.Add(FString::Printf(TEXT("- SourceChunkStore: %s"), *Report.SourceChunkStorePath));
		Lines.Add(FString::Printf(TEXT("- SQLiteActive: %s"), bSQLiteActive ? TEXT("true") : TEXT("false")));
		Lines.Add(FString::Printf(TEXT("- FTSActive: %s"), bFtsActive ? TEXT("true") : TEXT("false")));
		Lines.Add(FString::Printf(TEXT("- FallbackSearchActive: %s"), bFallbackSearchActive ? TEXT("true") : TEXT("false")));
		Lines.Add(TEXT(""));
		Lines.Add(TEXT("## Summary"));
		Lines.Add(FString::Printf(TEXT("- SourceChunkCount: %d"), Report.Summary.SourceChunkCount));
		Lines.Add(FString::Printf(TEXT("- CatalogChunkCount: %d"), Report.Summary.CatalogChunkCount));
		Lines.Add(FString::Printf(TEXT("- InsertedChunkCount: %d"), Report.Summary.InsertedChunkCount));
		Lines.Add(FString::Printf(TEXT("- UpdatedChunkCount: %d"), Report.Summary.UpdatedChunkCount));
		Lines.Add(FString::Printf(TEXT("- SkippedChunkCount: %d"), Report.Summary.SkippedChunkCount));
		Lines.Add(FString::Printf(TEXT("- UnchangedChunkCount: %d"), Report.Summary.UnchangedChunkCount));
		Lines.Add(FString::Printf(TEXT("- LabelCount: %d"), Report.Summary.LabelCount));
		Lines.Add(FString::Printf(TEXT("- GroupCount: %d"), Report.Summary.GroupCount));
		Lines.Add(FString::Printf(TEXT("- SourceReferenceCount: %d"), SourceReferenceCount));
		Lines.Add(FString::Printf(TEXT("- WarningCount: %d"), Report.Summary.WarningCount));
		Lines.Add(FString::Printf(TEXT("- ErrorCount: %d"), Report.Summary.ErrorCount));

		if (Report.Warnings.Num() > 0)
		{
			Lines.Add(TEXT(""));
			Lines.Add(TEXT("## Warnings"));
			for (const FString& Warning : Report.Warnings)
			{
				Lines.Add(FString::Printf(TEXT("- %s"), *Warning));
			}
		}

		if (Report.Errors.Num() > 0)
		{
			Lines.Add(TEXT(""));
			Lines.Add(TEXT("## Errors"));
			for (const FString& Error : Report.Errors)
			{
				Lines.Add(FString::Printf(TEXT("- %s"), *Error));
			}
		}

		return FFileHelper::SaveStringArrayToFile(Lines, *(FIISStoragePaths::GetIndexesDir() / CatalogBuildReportMarkdownName));
	}

	TArray<TSharedPtr<FJsonValue>> MakeSourceReferencesJsonArray(const TArray<FIISSourceReference>& References)
	{
		TArray<TSharedPtr<FJsonValue>> Values;
		for (const FIISSourceReference& Reference : References)
		{
			TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
			Object->SetStringField(TEXT("artifact_kind"), Reference.ArtifactKind);
			Object->SetStringField(TEXT("relative_path"), Reference.RelativePath);
			Object->SetStringField(TEXT("json_pointer"), Reference.JsonPointer);
			Object->SetStringField(TEXT("fingerprint"), Reference.Fingerprint);
			Object->SetStringField(TEXT("explanation"), Reference.Explanation);
			Values.Add(MakeShared<FJsonValueObject>(Object));
		}
		return Values;
	}

	TSharedRef<FJsonObject> MakeSearchResultObject(const FIISSearchResult& Result)
	{
		TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetStringField(TEXT("chunk_id"), Result.ChunkId);
		Object->SetStringField(TEXT("title"), Result.Title);
		Object->SetStringField(TEXT("snippet"), Result.Snippet);
		Object->SetNumberField(TEXT("score"), Result.Score);
		Object->SetNumberField(TEXT("lexical_score"), Result.LexicalScore);
		Object->SetNumberField(TEXT("vector_score"), Result.VectorScore);
		Object->SetNumberField(TEXT("hybrid_score"), Result.HybridScore);
		Object->SetStringField(TEXT("score_explanation"), Result.ScoreExplanation);
		Object->SetArrayField(TEXT("matched_labels"), MakeStringArray(Result.MatchedLabels));
		Object->SetArrayField(TEXT("matched_groups"), MakeStringArray(Result.MatchedGroups));
		Object->SetStringField(TEXT("chunk_kind"), ChunkKindToString(Result.Chunk.ChunkKind));
		Object->SetStringField(TEXT("sensitivity"), ChunkSensitivityToString(Result.Chunk.Sensitivity));
		Object->SetArrayField(TEXT("retrieval_labels"), MakeStringArray(Result.Chunk.RetrievalLabels));
		Object->SetArrayField(TEXT("retrieval_groups"), MakeStringArray(Result.Chunk.RetrievalGroups));
		Object->SetArrayField(TEXT("source_references"), MakeSourceReferencesJsonArray(Result.Chunk.SourceReferences));
		return Object;
	}

	TArray<TSharedPtr<FJsonValue>> MakeSearchResultsArray(const TArray<FIISSearchResult>& Results)
	{
		TArray<TSharedPtr<FJsonValue>> Values;
		for (const FIISSearchResult& Result : Results)
		{
			Values.Add(MakeShared<FJsonValueObject>(MakeSearchResultObject(Result)));
		}
		return Values;
	}

	TSharedRef<FJsonObject> MakeContextPackObject(const FIISContextPack& ContextPack)
	{
		TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetStringField(TEXT("context_pack_id"), ContextPack.ContextPackId);
		Object->SetStringField(TEXT("created_at_utc"), ContextPack.CreatedAtUtc);
		Object->SetStringField(TEXT("status"), ContextPackStatusToString(ContextPack.Status));
		Object->SetStringField(TEXT("query_text"), ContextPack.QueryText);
		Object->SetStringField(TEXT("search_mode"), ContextPack.SearchMode);
		Object->SetBoolField(TEXT("allows_migration_decision"), ContextPack.bAllowsMigrationDecision);
		Object->SetBoolField(TEXT("allows_patch_generation"), ContextPack.bAllowsPatchGeneration);
		Object->SetArrayField(TEXT("guardrails"), MakeStringArray(ContextPack.Guardrails));
		Object->SetArrayField(TEXT("warnings"), MakeStringArray(ContextPack.Warnings));

		TArray<TSharedPtr<FJsonValue>> Items;
		for (const FIISContextPackItem& Item : ContextPack.Items)
		{
			TSharedRef<FJsonObject> ItemObject = MakeShared<FJsonObject>();
			ItemObject->SetStringField(TEXT("chunk_id"), Item.ChunkId);
			ItemObject->SetStringField(TEXT("title"), Item.Title);
			ItemObject->SetStringField(TEXT("text"), Item.Text);
			ItemObject->SetArrayField(TEXT("retrieval_labels"), MakeStringArray(Item.RetrievalLabels));
			ItemObject->SetArrayField(TEXT("source_references"), MakeSourceReferencesJsonArray(Item.SourceReferences));
			Items.Add(MakeShared<FJsonValueObject>(ItemObject));
		}
		Object->SetArrayField(TEXT("items"), Items);
		return Object;
	}

	TSharedRef<FJsonObject> MakeSearchDiagnosticsObject(
		const FIISSearchQuery& Query,
		const FIISSearchResponse& Response)
	{
		TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetStringField(TEXT("mode"), SearchModeToString(Query.SearchMode));
		Object->SetStringField(TEXT("query"), Query.QueryText);
		Object->SetArrayField(TEXT("required_labels"), MakeStringArray(Query.RequiredLabels));
		Object->SetArrayField(TEXT("preferred_groups"), MakeStringArray(Query.PreferredGroups));
		Object->SetArrayField(TEXT("excluded_sensitivities"), MakeStringArray(Query.ExcludedSensitivities));
		Object->SetNumberField(TEXT("lexical_candidate_count"), Response.LexicalCandidateCount);
		Object->SetNumberField(TEXT("vector_candidate_count"), Response.VectorCandidateCount);
		Object->SetNumberField(TEXT("merged_candidate_count"), Response.MergedCandidateCount);
		Object->SetNumberField(TEXT("final_result_count"), Response.FinalResultCount);
		Object->SetNumberField(TEXT("warning_count"), Response.Warnings.Num());
		Object->SetNumberField(TEXT("error_count"), Response.Errors.Num());
		Object->SetStringField(TEXT("summary"), Response.DiagnosticsSummary);
		return Object;
	}

	bool WriteContextPackReportMarkdown(const FIISContextPack& ContextPack, const FString& MarkdownPath)
	{
		TArray<FString> Lines;
		Lines.Add(TEXT("# IIS Context Pack Report"));
		Lines.Add(TEXT(""));
		Lines.Add(FString::Printf(TEXT("- ContextPackId: %s"), *ContextPack.ContextPackId));
		Lines.Add(FString::Printf(TEXT("- CreatedAtUtc: %s"), *ContextPack.CreatedAtUtc));
		Lines.Add(FString::Printf(TEXT("- Query: %s"), *ContextPack.QueryText));
		Lines.Add(FString::Printf(TEXT("- SearchMode: %s"), *ContextPack.SearchMode));
		Lines.Add(FString::Printf(TEXT("- Status: %s"), *ContextPackStatusToString(ContextPack.Status)));
		Lines.Add(FString::Printf(TEXT("- Items: %d"), ContextPack.Items.Num()));
		Lines.Add(FString::Printf(TEXT("- AllowsMigrationDecision: %s"), ContextPack.bAllowsMigrationDecision ? TEXT("true") : TEXT("false")));
		Lines.Add(FString::Printf(TEXT("- AllowsPatchGeneration: %s"), ContextPack.bAllowsPatchGeneration ? TEXT("true") : TEXT("false")));

		if (ContextPack.Guardrails.Num() > 0)
		{
			Lines.Add(TEXT(""));
			Lines.Add(TEXT("## Guardrails"));
			for (const FString& Guardrail : ContextPack.Guardrails)
			{
				Lines.Add(FString::Printf(TEXT("- %s"), *Guardrail));
			}
		}

		if (ContextPack.Warnings.Num() > 0)
		{
			Lines.Add(TEXT(""));
			Lines.Add(TEXT("## Warnings"));
			for (const FString& Warning : ContextPack.Warnings)
			{
				Lines.Add(FString::Printf(TEXT("- %s"), *Warning));
			}
		}

		Lines.Add(TEXT(""));
		Lines.Add(TEXT("## Items"));
		for (const FIISContextPackItem& Item : ContextPack.Items)
		{
			Lines.Add(TEXT(""));
			Lines.Add(FString::Printf(TEXT("### %s"), Item.Title.IsEmpty() ? *Item.ChunkId : *Item.Title));
			Lines.Add(FString::Printf(TEXT("- Chunk: %s"), *Item.ChunkId));
			Lines.Add(FString::Printf(TEXT("- Labels: %s"), *FString::Join(Item.RetrievalLabels, TEXT(", "))));
			Lines.Add(FString::Printf(TEXT("- Source References: %d"), Item.SourceReferences.Num()));
			Lines.Add(FString::Printf(TEXT("- Text: %s"), *Item.Text.Left(600)));
		}

		return FFileHelper::SaveStringArrayToFile(Lines, *MarkdownPath);
	}
}

#if WITH_DEV_AUTOMATION_TESTS
void FIISChunkCatalog::SetForceUpsertFailureChunkIdForTest(const FString& ChunkId)
{
	GForceUpsertFailureChunkId = ChunkId;
}

void FIISChunkCatalog::ClearForceUpsertFailureChunkIdForTest()
{
	GForceUpsertFailureChunkId.Reset();
}
#endif

bool FIISChunkCatalog::IsActiveLifecycleState(const FString& LifecycleState)
{
	return LifecycleState.IsEmpty() || LifecycleState.Equals(TEXT("active"), ESearchCase::IgnoreCase);
}

bool FIISChunkCatalog::IsRetrievableLifecycleState(const FString& LifecycleState)
{
	return IsActiveLifecycleState(LifecycleState);
}

EIISChunkUpsertAction FIISChunkCatalog::ClassifyUpsert(
	const bool bExists,
	const FString& ExistingSha,
	const FString& NewSha)
{
	if (!bExists)
	{
		return EIISChunkUpsertAction::Insert;
	}
	if (ExistingSha == NewSha)
	{
		return EIISChunkUpsertAction::NoOp;
	}
	return EIISChunkUpsertAction::Conflict;
}

FString FIISChunkCatalog::GetCatalogPath()
{
	return FIISStoragePaths::GetIndexesDir() / CatalogFileName;
}

bool FIISChunkCatalog::BuildOrUpdateCatalogFromChunkStore(
	FString& OutReportPath,
	TArray<FString>& OutWarnings)
{
	return BuildOrUpdateCatalogFromChunkStorePath(
		FIISStoragePaths::GetChunksDir() / TEXT("chunk_store.jsonl"),
		OutReportPath,
		OutWarnings);
}

bool FIISChunkCatalog::BuildOrUpdateCatalogFromChunkStorePath(
	const FString& ChunkStoreJsonlPath,
	FString& OutReportPath,
	TArray<FString>& OutWarnings)
{
	OutReportPath.Reset();
	OutWarnings.Reset();

	FIISStoragePaths::EnsureDefaultFolders();

	FIISCatalogBuildReport Report;
	Report.GeneratedAtUtc = FDateTime::UtcNow().ToIso8601();
	Report.CatalogPath = GetCatalogPath();
	Report.SourceChunkStorePath = ChunkStoreJsonlPath.IsEmpty()
		? FString()
		: FPaths::ConvertRelativePathToFull(ChunkStoreJsonlPath);

	TArray<FIISIndexChunk> SourceChunks;
	const bool bLoaded = LoadChunksFromJsonl(Report.SourceChunkStorePath, SourceChunks, Report);

	bool bSQLiteActive = false;
	bool bFtsActive = false;
	bool bFallbackSearchActive = true;
	int32 SourceReferenceCount = 0;

	if (bLoaded)
	{
		FSQLiteDatabase Database;
		if (OpenCatalogDatabase(Database, Report.Errors))
		{
			bSQLiteActive = true;
			if (EnsureCatalogSchema(Database, Report, bFtsActive))
			{
				TArray<FIISImportConflict> ImportConflicts;
				const FString ImportRunId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
				TMap<FString, TSet<FString>> SourceIdToSeenChunkIds;
				bool bImportOk = ExecuteStatement(Database, TEXT("BEGIN TRANSACTION;"), Report.Errors);

				const FString ImportedAtUtc = Report.GeneratedAtUtc;
				for (FIISIndexChunk& Chunk : SourceChunks)
				{
					if (!bImportOk)
					{
						break;
					}

					if (!Chunk.SourceId.IsEmpty())
					{
						SourceIdToSeenChunkIds.FindOrAdd(Chunk.SourceId).Add(Chunk.ChunkId);
					}

					bool bInserted = false;
					bool bUpdated = false;
					if (!UpsertChunkIntoCatalog(
						Database, Chunk, ImportedAtUtc, bFtsActive, Report, ImportConflicts, bInserted, bUpdated))
					{
						bImportOk = false;
						break;
					}

					if (bInserted)
					{
						++Report.Summary.InsertedChunkCount;
					}
					else if (bUpdated)
					{
						++Report.Summary.UpdatedChunkCount;
					}
				}

				if (bImportOk)
				{
					for (const TPair<FString, TSet<FString>>& Pair : SourceIdToSeenChunkIds)
					{
						if (!MarkStaleChunksForSource(Database, Pair.Key, Pair.Value, Report.Errors))
						{
							bImportOk = false;
							break;
						}
					}
				}

				ExecuteStatement(
					Database,
					bImportOk ? TEXT("COMMIT TRANSACTION;") : TEXT("ROLLBACK TRANSACTION;"),
					Report.Errors);

				if (bImportOk)
				{
					WriteImportConflictsReport(ImportConflicts, ImportRunId);
				}

				Report.Summary.CatalogChunkCount = QueryScalarInt(Database, TEXT("SELECT COUNT(*) FROM chunks;"));
				Report.Summary.LabelCount = QueryScalarInt(Database, TEXT("SELECT COUNT(*) FROM chunk_labels;"));
				Report.Summary.GroupCount = QueryScalarInt(Database, TEXT("SELECT COUNT(*) FROM chunk_groups;"));
				SourceReferenceCount = QueryScalarInt(Database, TEXT("SELECT COUNT(*) FROM chunk_source_refs;"));
			}
			Database.Close();
		}
	}

	Report.Summary.WarningCount = Report.Warnings.Num();
	Report.Summary.ErrorCount = Report.Errors.Num();
	if (!bLoaded || !bSQLiteActive || Report.Summary.ErrorCount > 0)
	{
		Report.Summary.Status = Report.Summary.CatalogChunkCount > 0 ? EIISCatalogStatus::Warning : EIISCatalogStatus::Error;
	}
	else if (Report.Summary.CatalogChunkCount == 0)
	{
		Report.Summary.Status = EIISCatalogStatus::Empty;
	}
	else if (Report.Summary.WarningCount > 0 || Report.Summary.SkippedChunkCount > 0)
	{
		Report.Summary.Status = EIISCatalogStatus::Warning;
	}
	// UnchangedChunkCount (idempotent re-import) does not downgrade status to Warning.
	else
	{
		Report.Summary.Status = EIISCatalogStatus::Ready;
	}

	OutReportPath = FIISStoragePaths::GetIndexesDir() / CatalogBuildReportJsonName;
	WriteCatalogBuildReportJson(Report, bSQLiteActive, bFtsActive, bFallbackSearchActive, SourceReferenceCount);
	WriteCatalogBuildReportMarkdown(Report, bSQLiteActive, bFtsActive, bFallbackSearchActive, SourceReferenceCount);

	OutWarnings.Append(Report.Warnings);
	OutWarnings.Append(Report.Errors);

	return Report.Summary.Status == EIISCatalogStatus::Ready || Report.Summary.Status == EIISCatalogStatus::Warning;
}

bool FIISChunkCatalog::SearchCatalog(const FIISSearchQuery& Query, FIISSearchResponse& OutResponse)
{
	OutResponse = FIISSearchResponse();
	OutResponse.QueryText = Query.QueryText;

	FIISSearchQuery EffectiveQuery = Query;
	if (EffectiveQuery.SearchMode == EIISSearchMode::Unknown)
	{
		EffectiveQuery.SearchMode = EIISSearchMode::Lexical;
		OutResponse.Warnings.Add(TEXT("Unknown IIS search mode requested; using lexical search."));
	}
	if (!FPaths::FileExists(GetCatalogPath()))
	{
		OutResponse.Status = EIISSearchStatus::Warning;
		OutResponse.Warnings.Add(FString::Printf(TEXT("IIS catalog does not exist yet: %s"), *GetCatalogPath()));
		return true;
	}

	FSQLiteDatabase Database;
	if (!OpenCatalogDatabase(Database, OutResponse.Errors))
	{
		OutResponse.Status = EIISSearchStatus::Error;
		return false;
	}

	if (EffectiveQuery.SearchMode == EIISSearchMode::Hybrid)
	{
		SearchHybrid(Database, EffectiveQuery, OutResponse);
	}
	else if (EffectiveQuery.SearchMode == EIISSearchMode::Vector)
	{
		SearchVector(Database, EffectiveQuery, OutResponse);
	}
	else
	{
		const bool bUsedFts = SearchFts(Database, EffectiveQuery, OutResponse);
		if (!bUsedFts)
		{
			if (IsFtsTableAvailable(Database))
			{
				OutResponse.Warnings.Add(TEXT("SQLite FTS returned no usable rows for this query; deterministic lexical fallback was used."));
			}
			SearchFallback(Database, EffectiveQuery, OutResponse);
		}
	}

	OutResponse.FinalResultCount = OutResponse.Results.Num();
	Database.Close();
	return OutResponse.Status != EIISSearchStatus::Error;
}

bool FIISChunkCatalog::LoadChunkById(
	const FString& ChunkId,
	FIISIndexChunk& OutChunk,
	TArray<FString>& OutWarnings)
{
	OutChunk = FIISIndexChunk();
	OutWarnings.Reset();

	if (ChunkId.IsEmpty())
	{
		OutWarnings.Add(TEXT("Chunk id is empty."));
		return false;
	}

	if (!FPaths::FileExists(GetCatalogPath()))
	{
		OutWarnings.Add(FString::Printf(TEXT("IIS catalog does not exist yet: %s"), *GetCatalogPath()));
		return false;
	}

	FSQLiteDatabase Database;
	TArray<FString> Errors;
	if (!OpenCatalogDatabase(Database, Errors))
	{
		OutWarnings.Append(Errors);
		return false;
	}

	const bool bLoaded = LoadChunkByIdFromOpenDatabase(Database, ChunkId, OutChunk);
	Database.Close();
	if (!bLoaded)
	{
		OutWarnings.Add(FString::Printf(TEXT("Chunk was not found in IIS catalog: %s"), *ChunkId));
	}
	return bLoaded;
}

bool FIISChunkCatalog::WriteSearchReport(
	const FIISSearchQuery& Query,
	const FIISSearchResponse& Response,
	const FIISContextPack* ContextPack,
	FString& OutSearchReportPath)
{
	FIISStoragePaths::EnsureDefaultFolders();

	TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
	Object->SetStringField(TEXT("schema_version"), TEXT("0.1.0"));
	Object->SetStringField(TEXT("tool_name"), TEXT("Internal Index Service"));
	Object->SetStringField(TEXT("generated_at_utc"), FDateTime::UtcNow().ToIso8601());
	Object->SetStringField(TEXT("query_text"), Query.QueryText);
	Object->SetStringField(TEXT("mode"), SearchModeToString(Query.SearchMode));
	Object->SetStringField(TEXT("status"), SearchStatusToString(Response.Status));
	Object->SetNumberField(TEXT("result_count"), Response.Results.Num());
	Object->SetObjectField(TEXT("diagnostics"), MakeSearchDiagnosticsObject(Query, Response));
	Object->SetArrayField(TEXT("warnings"), MakeStringArray(Response.Warnings));
	Object->SetArrayField(TEXT("errors"), MakeStringArray(Response.Errors));
	Object->SetArrayField(TEXT("results"), MakeSearchResultsArray(Response.Results));
	if (ContextPack)
	{
		Object->SetObjectField(TEXT("context_pack"), MakeContextPackObject(*ContextPack));
	}

	OutSearchReportPath = FIISStoragePaths::GetLogsDir() / SearchReportJsonName;
	const bool bJsonSaved = SaveJsonObjectToFile(Object, OutSearchReportPath);

	TArray<FString> Lines;
	Lines.Add(TEXT("# IIS Search Report"));
	Lines.Add(TEXT(""));
	Lines.Add(FString::Printf(TEXT("- Query: %s"), *Query.QueryText));
	Lines.Add(FString::Printf(TEXT("- Mode: %s"), *SearchModeToString(Query.SearchMode)));
	Lines.Add(FString::Printf(TEXT("- Status: %s"), *SearchStatusToString(Response.Status)));
	Lines.Add(FString::Printf(TEXT("- Results: %d"), Response.Results.Num()));
	Lines.Add(FString::Printf(TEXT("- Diagnostics: %s"), *Response.DiagnosticsSummary));
	Lines.Add(TEXT(""));
	Lines.Add(TEXT("## Diagnostics"));
	Lines.Add(FString::Printf(TEXT("- RequiredLabels: %s"), *FString::Join(Query.RequiredLabels, TEXT(", "))));
	Lines.Add(FString::Printf(TEXT("- PreferredGroups: %s"), *FString::Join(Query.PreferredGroups, TEXT(", "))));
	Lines.Add(FString::Printf(TEXT("- ExcludedSensitivities: %s"), *FString::Join(Query.ExcludedSensitivities, TEXT(", "))));
	Lines.Add(FString::Printf(TEXT("- LexicalCandidateCount: %d"), Response.LexicalCandidateCount));
	Lines.Add(FString::Printf(TEXT("- VectorCandidateCount: %d"), Response.VectorCandidateCount));
	Lines.Add(FString::Printf(TEXT("- MergedCandidateCount: %d"), Response.MergedCandidateCount));
	Lines.Add(FString::Printf(TEXT("- FinalResultCount: %d"), Response.FinalResultCount));
	if (ContextPack)
	{
		Lines.Add(FString::Printf(TEXT("- ContextPackItems: %d"), ContextPack->Items.Num()));
		Lines.Add(FString::Printf(TEXT("- ContextPackAllowsMigrationDecision: %s"), ContextPack->bAllowsMigrationDecision ? TEXT("true") : TEXT("false")));
		Lines.Add(FString::Printf(TEXT("- ContextPackAllowsPatchGeneration: %s"), ContextPack->bAllowsPatchGeneration ? TEXT("true") : TEXT("false")));
	}

	if (Response.Warnings.Num() > 0)
	{
		Lines.Add(TEXT(""));
		Lines.Add(TEXT("## Warnings"));
		for (const FString& Warning : Response.Warnings)
		{
			Lines.Add(FString::Printf(TEXT("- %s"), *Warning));
		}
	}

	if (Response.Errors.Num() > 0)
	{
		Lines.Add(TEXT(""));
		Lines.Add(TEXT("## Errors"));
		for (const FString& Error : Response.Errors)
		{
			Lines.Add(FString::Printf(TEXT("- %s"), *Error));
		}
	}

	Lines.Add(TEXT(""));
	Lines.Add(TEXT("## Results"));
	for (const FIISSearchResult& Result : Response.Results)
	{
		Lines.Add(TEXT(""));
		Lines.Add(FString::Printf(TEXT("### %s"), Result.Title.IsEmpty() ? *Result.ChunkId : *Result.Title));
		Lines.Add(FString::Printf(TEXT("- Chunk: %s"), *Result.ChunkId));
		Lines.Add(FString::Printf(TEXT("- Score: %.4f"), Result.Score));
		Lines.Add(FString::Printf(TEXT("- LexicalScore: %.4f"), Result.LexicalScore));
		Lines.Add(FString::Printf(TEXT("- VectorScore: %.4f"), Result.VectorScore));
		Lines.Add(FString::Printf(TEXT("- HybridScore: %.4f"), Result.HybridScore));
		Lines.Add(FString::Printf(TEXT("- Explanation: %s"), *Result.ScoreExplanation));
		Lines.Add(FString::Printf(TEXT("- MatchedLabels: %s"), *FString::Join(Result.MatchedLabels, TEXT(", "))));
		Lines.Add(FString::Printf(TEXT("- MatchedGroups: %s"), *FString::Join(Result.MatchedGroups, TEXT(", "))));
		Lines.Add(FString::Printf(TEXT("- Labels: %s"), *FString::Join(Result.Chunk.RetrievalLabels, TEXT(", "))));
		Lines.Add(FString::Printf(TEXT("- Groups: %s"), *FString::Join(Result.Chunk.RetrievalGroups, TEXT(", "))));
		Lines.Add(FString::Printf(TEXT("- Source References: %d"), Result.Chunk.SourceReferences.Num()));
		Lines.Add(FString::Printf(TEXT("- Snippet: %s"), *Result.Snippet));
	}

	const bool bMarkdownSaved = FFileHelper::SaveStringArrayToFile(
		Lines,
		*(FIISStoragePaths::GetLogsDir() / SearchReportMarkdownName));

	return bJsonSaved && bMarkdownSaved;
}

bool FIISChunkCatalog::WriteContextPackReport(
	const FIISContextPack& ContextPack,
	FString& OutContextPackReportPath)
{
	FIISStoragePaths::EnsureDefaultFolders();

	OutContextPackReportPath = FIISStoragePaths::GetContextPacksDir() / ContextPackReportJsonName;
	const bool bJsonSaved = SaveJsonObjectToFile(MakeContextPackObject(ContextPack), OutContextPackReportPath);
	const bool bMarkdownSaved = WriteContextPackReportMarkdown(
		ContextPack,
		FIISStoragePaths::GetContextPacksDir() / ContextPackReportMarkdownName);

	return bJsonSaved && bMarkdownSaved;
}

#if WITH_DEV_AUTOMATION_TESTS
bool FIISChunkCatalog::SearchVectorDelegatedForTest(
	const FString& CatalogDatabasePath,
	const TArray<FIISVectorRecordIn>& VectorRecords,
	const TMap<FString, TArray<float>>& QueryVectorsByRoute,
	const FIISSearchQuery& Query,
	FIISSearchResponse& OutResponse)
{
	OutResponse = FIISSearchResponse();
	OutResponse.QueryText = Query.QueryText;

	FSQLiteDatabase Database;
	if (!Database.Open(*CatalogDatabasePath, ESQLiteDatabaseOpenMode::ReadOnly))
	{
		OutResponse.Status = EIISSearchStatus::Error;
		OutResponse.Errors.Add(FString::Printf(TEXT("Failed to open SQLite catalog: %s"), *CatalogDatabasePath));
		OutResponse.Errors.Add(Database.GetLastError());
		return false;
	}

	TArray<FIISVectorRecord> CatalogRecords;
	CatalogRecords.Reserve(VectorRecords.Num());
	for (const FIISVectorRecordIn& RecordIn : VectorRecords)
	{
		FIISVectorRecord Record;
		Record.ChunkId = RecordIn.ChunkId;
		Record.RouteId = RecordIn.RouteId;
		Record.ModelId = RecordIn.ModelId;
		Record.TextSha256 = RecordIn.TextSha256;
		Record.Dimensions = RecordIn.Dimensions;
		Record.Vector = RecordIn.Vector;
		CatalogRecords.Add(MoveTemp(Record));
	}

	TMap<FString, FIISQueryVector> QueryVectors;
	for (const TPair<FString, TArray<float>>& Pair : QueryVectorsByRoute)
	{
		FIISQueryVector QueryVector;
		QueryVector.RouteId = Pair.Key;
		QueryVector.ProviderId = TEXT("test");
		QueryVector.ModelId = TEXT("test");
		QueryVector.Vector = Pair.Value;
		QueryVector.Dimensions = Pair.Value.Num();
		QueryVectors.Add(Pair.Key, MoveTemp(QueryVector));
	}

	TUniquePtr<IIISVectorIndexBackend> Backend = CreateVectorIndexBackend();
	const int32 ComparedVectorCount = CollectVectorSearchResultsFromBackend(
		Database,
		Query,
		CatalogRecords,
		QueryVectors,
		*Backend,
		OutResponse);
	FinalizeVectorSearchResponse(Query, ComparedVectorCount, OutResponse);
	Database.Close();
	return true;
}
#endif
