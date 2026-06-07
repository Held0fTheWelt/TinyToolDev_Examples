/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#include "IISBlueprintExplanationService.h"

#include "IISUsageGraphImporter.h"
#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace IISBlueprintExplanation
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

	static FString DefaultHandoffContractPath()
	{
		return NormalizePath(
			FPaths::Combine(
				FPaths::ProjectSavedDir(),
				TEXT("InternalIndexService"),
				TEXT("handoff"),
				TEXT("iis_import_contract.json")));
	}

	static FString DefaultBlueprintsDirectory()
	{
		return NormalizePath(FPaths::Combine(
			FPaths::ProjectSavedDir(),
			TEXT("InternalIndexService"),
			TEXT("evidence"),
			TEXT("blueprints")));
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

	static bool GetBoolField(const TSharedPtr<FJsonObject>& Object, const FString& FieldName, bool bDefault = false)
	{
		bool bValue = bDefault;
		if (Object.IsValid())
		{
			Object->TryGetBoolField(FieldName, bValue);
		}
		return bValue;
	}

	static void AddUniqueSorted(TArray<FString>& Values, const FString& Value)
	{
		if (Value.IsEmpty())
		{
			return;
		}
		Values.AddUnique(Value);
		Values.Sort();
	}

	static bool LooksLikeAssetPath(const FString& Value)
	{
		return Value.StartsWith(TEXT("/")) || Value.Contains(TEXT("/Game/")) || Value.Contains(TEXT(".")) || Value.Contains(TEXT("/"));
	}

	static bool QueryMatchesBlueprint(const FString& Query, const FString& AssetPath, const FString& BlueprintName)
	{
		const FString TrimmedQuery = Query.TrimStartAndEnd();
		if (TrimmedQuery.IsEmpty())
		{
			return false;
		}
		return AssetPath.Contains(TrimmedQuery, ESearchCase::IgnoreCase, ESearchDir::FromStart)
			|| BlueprintName.Contains(TrimmedQuery, ESearchCase::IgnoreCase, ESearchDir::FromStart)
			|| AssetPath.Equals(TrimmedQuery, ESearchCase::IgnoreCase)
			|| BlueprintName.Equals(TrimmedQuery, ESearchCase::IgnoreCase);
	}

	static bool TryDeriveNetworkHint(const TSharedPtr<FJsonObject>& NodeObject, FString& OutHint)
	{
		const FString FunctionName = GetStringField(NodeObject, TEXT("function_name"));
		const FString K2Kind = GetStringField(NodeObject, TEXT("k2_node_kind"));
		const FString NodeClass = GetStringField(NodeObject, TEXT("node_class"));

		auto ContainsNetworkToken = [](const FString& Text) -> bool
		{
			return Text.Contains(TEXT("Server"), ESearchCase::IgnoreCase)
				|| Text.Contains(TEXT("Client"), ESearchCase::IgnoreCase)
				|| Text.Contains(TEXT("Multicast"), ESearchCase::IgnoreCase)
				|| Text.Contains(TEXT("Authority"), ESearchCase::IgnoreCase)
				|| Text.Contains(TEXT("Replication"), ESearchCase::IgnoreCase)
				|| Text.Contains(TEXT("RepNotify"), ESearchCase::IgnoreCase);
		};

		if (ContainsNetworkToken(FunctionName))
		{
			OutHint = FString::Printf(TEXT("function:%s"), *FunctionName);
			return true;
		}
		if (ContainsNetworkToken(K2Kind))
		{
			OutHint = FString::Printf(TEXT("k2:%s"), *K2Kind);
			return true;
		}
		if (ContainsNetworkToken(NodeClass))
		{
			OutHint = FString::Printf(TEXT("node_class:%s"), *NodeClass);
			return true;
		}
		return false;
	}

	static void ReadStringArrayField(const TSharedPtr<FJsonObject>& Root, const TCHAR* FieldName, TArray<FString>& OutValues)
	{
		const TArray<TSharedPtr<FJsonValue>>* Array = nullptr;
		if (!Root.IsValid() || !Root->TryGetArrayField(FieldName, Array))
		{
			return;
		}
		for (const TSharedPtr<FJsonValue>& Value : *Array)
		{
			FString StringValue;
			if (Value.IsValid() && Value->TryGetString(StringValue))
			{
				AddUniqueSorted(OutValues, StringValue);
			}
		}
	}

	static void ReadNamedObjectArray(const TSharedPtr<FJsonObject>& Root, const TCHAR* FieldName, TArray<FString>& OutNames)
	{
		const TArray<TSharedPtr<FJsonValue>>* Array = nullptr;
		if (!Root.IsValid() || !Root->TryGetArrayField(FieldName, Array))
		{
			return;
		}
		for (const TSharedPtr<FJsonValue>& Value : *Array)
		{
			const TSharedPtr<FJsonObject>* Object = nullptr;
			if (!Value.IsValid() || !Value->TryGetObject(Object) || !Object)
			{
				continue;
			}
			AddUniqueSorted(OutNames, GetStringField(*Object, TEXT("name")));
		}
	}

	static bool ResolveBlueprintsDirectory(FString& OutBlueprintsDirectory, TArray<FString>& OutWarnings)
	{
		const FString ContractPath = DefaultHandoffContractPath();
		if (FPaths::FileExists(ContractPath))
		{
			FResolvedUsageEvidencePaths Paths;
			if (FIISUsageGraphImporter::ResolveEvidencePathsFromHandoffContract(ContractPath, Paths, OutWarnings)
				&& !Paths.BlueprintsDirectory.IsEmpty()
				&& FPaths::DirectoryExists(Paths.BlueprintsDirectory))
			{
				OutBlueprintsDirectory = Paths.BlueprintsDirectory;
				return true;
			}
		}

		const FString Fallback = DefaultBlueprintsDirectory();
		if (FPaths::DirectoryExists(Fallback))
		{
			OutBlueprintsDirectory = Fallback;
			OutWarnings.Add(FString::Printf(TEXT("Using fallback Blueprint IR directory: %s"), *Fallback));
			return true;
		}

		const FString AutomationBlueprintsDir = NormalizePath(
			FPaths::ProjectIntermediateDir() / TEXT("IISBlueprintExplainTests") / TEXT("blueprints"));
		if (FPaths::DirectoryExists(AutomationBlueprintsDir))
		{
			OutBlueprintsDirectory = AutomationBlueprintsDir;
			return true;
		}

		OutWarnings.Add(TEXT("Blueprint IR directory could not be resolved."));
		return false;
	}
}

bool FIISBlueprintExplanationService::AssembleFromIRJson(
	const FString& IRJson,
	const FString& SourceFilePath,
	FIISBlueprintExplanation& Out)
{
	Out = FIISBlueprintExplanation();
	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(IRJson);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		Out.bIRAvailable = false;
		Out.Warnings.Add(TEXT("blueprint IR parse failed"));
		return false;
	}

	Root->TryGetStringField(TEXT("blueprint_name"), Out.BlueprintName);
	Root->TryGetStringField(TEXT("asset_path"), Out.AssetPath);
	Root->TryGetStringField(TEXT("parent_class"), Out.ParentClass);

	const TArray<TSharedPtr<FJsonValue>>* Graphs = nullptr;
	if (Root->TryGetArrayField(TEXT("graphs"), Graphs))
	{
		for (int32 GraphIndex = 0; GraphIndex < Graphs->Num(); ++GraphIndex)
		{
			const TSharedPtr<FJsonObject>* GraphObject = nullptr;
			if (!(*Graphs)[GraphIndex].IsValid() || !(*Graphs)[GraphIndex]->TryGetObject(GraphObject) || !GraphObject)
			{
				continue;
			}

			FIISBlueprintGraphSummary Summary;
			(*GraphObject)->TryGetStringField(TEXT("graph_name"), Summary.GraphName);
			(*GraphObject)->TryGetStringField(TEXT("graph_kind"), Summary.GraphKind);

			const TArray<TSharedPtr<FJsonValue>>* Nodes = nullptr;
			if ((*GraphObject)->TryGetArrayField(TEXT("nodes"), Nodes))
			{
				Summary.NodeCount = Nodes->Num();
				for (int32 NodeIndex = 0; NodeIndex < Nodes->Num(); ++NodeIndex)
				{
					const TSharedPtr<FJsonObject>* NodeObject = nullptr;
					if (!(*Nodes)[NodeIndex].IsValid() || !(*Nodes)[NodeIndex]->TryGetObject(NodeObject) || !NodeObject)
					{
						continue;
					}

					const FString EventName = IISBlueprintExplanation::GetStringField(*NodeObject, TEXT("event_name"));
					const FString FunctionName = IISBlueprintExplanation::GetStringField(*NodeObject, TEXT("function_name"));
					const FString FunctionOwner = IISBlueprintExplanation::GetStringField(*NodeObject, TEXT("function_owner"));
					const FString NodeClass = IISBlueprintExplanation::GetStringField(*NodeObject, TEXT("node_class"));

					if (!EventName.IsEmpty())
					{
						IISBlueprintExplanation::AddUniqueSorted(Out.EventsAndFunctions, EventName);
					}
					if (!FunctionName.IsEmpty())
					{
						const FString Label = FunctionOwner.IsEmpty()
							? FunctionName
							: FString::Printf(TEXT("%s::%s"), *FunctionOwner, *FunctionName);
						IISBlueprintExplanation::AddUniqueSorted(Out.EventsAndFunctions, Label);
						IISBlueprintExplanation::AddUniqueSorted(Out.ReferencedClassesOrFunctions, Label);
					}

					if (!IISBlueprintExplanation::GetBoolField(*NodeObject, TEXT("supported_specialization"), true)
						&& !NodeClass.IsEmpty())
					{
						IISBlueprintExplanation::AddUniqueSorted(Out.UnsupportedNodes, NodeClass);
					}

					FString NetworkHint;
					if (IISBlueprintExplanation::TryDeriveNetworkHint(*NodeObject, NetworkHint))
					{
						IISBlueprintExplanation::AddUniqueSorted(Out.NetworkAuthorityHints, NetworkHint);
					}
				}
			}

			Out.Graphs.Add(MoveTemp(Summary));
		}
	}

	IISBlueprintExplanation::ReadNamedObjectArray(Root, TEXT("variables"), Out.Variables);
	IISBlueprintExplanation::ReadNamedObjectArray(Root, TEXT("components"), Out.Components);

	const TArray<TSharedPtr<FJsonValue>>* Variables = nullptr;
	if (Root->TryGetArrayField(TEXT("variables"), Variables))
	{
		for (const TSharedPtr<FJsonValue>& Value : *Variables)
		{
			const TSharedPtr<FJsonObject>* VariableObject = nullptr;
			if (!Value.IsValid() || !Value->TryGetObject(VariableObject) || !VariableObject)
			{
				continue;
			}
			const FString TypeObject = IISBlueprintExplanation::GetStringField(*VariableObject, TEXT("type_sub_category_object"));
			if (IISBlueprintExplanation::LooksLikeAssetPath(TypeObject))
			{
				IISBlueprintExplanation::AddUniqueSorted(Out.ReferencedAssets, TypeObject);
			}
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* Components = nullptr;
	if (Root->TryGetArrayField(TEXT("components"), Components))
	{
		for (const TSharedPtr<FJsonValue>& Value : *Components)
		{
			const TSharedPtr<FJsonObject>* ComponentObject = nullptr;
			if (!Value.IsValid() || !Value->TryGetObject(ComponentObject) || !ComponentObject)
			{
				continue;
			}
			const FString ComponentClass = IISBlueprintExplanation::GetStringField(*ComponentObject, TEXT("component_class"));
			if (!ComponentClass.IsEmpty())
			{
				IISBlueprintExplanation::AddUniqueSorted(Out.ReferencedClassesOrFunctions, ComponentClass);
			}
		}
	}

	IISBlueprintExplanation::ReadStringArrayField(Root, TEXT("unsupported_node_classes"), Out.UnsupportedNodes);

	if (!SourceFilePath.IsEmpty())
	{
		FIISSourceReference SourceRef;
		SourceRef.RelativePath = SourceFilePath;
		SourceRef.JsonPointer = TEXT("/");
		SourceRef.ArtifactKind = TEXT("blueprint_ir");
		Out.SourceReferences.Add(MoveTemp(SourceRef));
	}

	Out.bIRAvailable = true;
	return true;
}

bool FIISBlueprintExplanationService::TryLoadBlueprintIRJson(
	const FString& AssetPathOrQuery,
	FString& OutIRJson,
	FString& OutSourceFilePath,
	TArray<FString>& OutWarnings)
{
	OutIRJson.Reset();
	OutSourceFilePath.Reset();

	const FString Query = AssetPathOrQuery.TrimStartAndEnd();
	if (Query.IsEmpty())
	{
		OutWarnings.Add(TEXT("Blueprint IR lookup requires a non-empty asset path or query."));
		return false;
	}

	FString BlueprintsDirectory;
	if (!IISBlueprintExplanation::ResolveBlueprintsDirectory(BlueprintsDirectory, OutWarnings))
	{
		return false;
	}

	TArray<FString> BlueprintFiles;
	IFileManager::Get().FindFilesRecursive(
		BlueprintFiles,
		*BlueprintsDirectory,
		TEXT("*.blueprint_ir.json"),
		true,
		false);

	FString BestMatchPath;
	FString BestMatchJson;
	int32 BestMatchScore = -1;

	for (const FString& BlueprintFile : BlueprintFiles)
	{
		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *BlueprintFile))
		{
			continue;
		}

		TSharedPtr<FJsonObject> Root;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
		{
			continue;
		}

		const FString AssetPath = IISBlueprintExplanation::GetStringField(Root, TEXT("asset_path"));
		const FString BlueprintName = IISBlueprintExplanation::GetStringField(Root, TEXT("blueprint_name"));
		if (!IISBlueprintExplanation::QueryMatchesBlueprint(Query, AssetPath, BlueprintName))
		{
			const FString Stem = FPaths::GetBaseFilename(BlueprintFile).Replace(TEXT(".blueprint_ir"), TEXT(""));
			if (!Stem.Equals(Query, ESearchCase::IgnoreCase) && !Stem.Contains(Query, ESearchCase::IgnoreCase))
			{
				continue;
			}
		}

		int32 Score = 0;
		if (AssetPath.Equals(Query, ESearchCase::IgnoreCase) || BlueprintName.Equals(Query, ESearchCase::IgnoreCase))
		{
			Score += 100;
		}
		if (AssetPath.Contains(Query, ESearchCase::IgnoreCase) || BlueprintName.Contains(Query, ESearchCase::IgnoreCase))
		{
			Score += 10;
		}

		if (Score > BestMatchScore)
		{
			BestMatchScore = Score;
			BestMatchPath = BlueprintFile;
			BestMatchJson = MoveTemp(JsonText);
		}
	}

	if (BestMatchPath.IsEmpty())
	{
		OutWarnings.Add(FString::Printf(TEXT("No Blueprint IR matched query: %s"), *Query));
		return false;
	}

	OutIRJson = BestMatchJson;
	OutSourceFilePath = BestMatchPath;
	return true;
}
