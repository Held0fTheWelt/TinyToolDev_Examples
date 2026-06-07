// SPDX-FileCopyrightText: Copyright (c) 2025-2026 Yves Tanas
// SPDX-License-Identifier: LicenseRef-Fab-Standard-EULA
//
// This file is part of the "Internal Index Service" Unreal Engine plugin.
// Use, reproduction, distribution, and modification are governed by the Fab Standard End User License Agreement,
// available at: https://www.fab.com/eula.

#include "IISAgentAccessService.h"

#include "IISChunkCatalog.h"
#include "IISLocalIndexService.h"
#include "IISBlueprintExplanationService.h"
#include "IISUsageGraphImporter.h"
#include "IISStoragePaths.h"
#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
	const TCHAR* AgentSearchBaseName = TEXT("latest_agent_search_response");
	const TCHAR* AgentContextPackBaseName = TEXT("latest_agent_context_pack");
	const TCHAR* AgentChunkLookupBaseName = TEXT("latest_agent_chunk_lookup");
	const TCHAR* AgentSourceReferencesBaseName = TEXT("latest_agent_source_references");
	const TCHAR* AgentUsageReportBaseName = TEXT("latest_agent_usage_report");
	const TCHAR* AgentBlueprintExplanationBaseName = TEXT("latest_agent_blueprint_explanation");

	const TCHAR* AgentContractsFileName = TEXT("iis_agent_tool_contracts.json");
	const TCHAR* AgentRegistryFileName = TEXT("iis_agent_tool_registry.json");
	const TCHAR* AgentMcpManifestFileName = TEXT("mcp_tool_manifest.json");
	const TCHAR* AgentAccessReportFileName = TEXT("iis_agent_access_report.md");
	const TCHAR* AgentInvocationResultFileName = TEXT("invocation_result.json");

	struct FIISAgentToolDefinition
	{
		EIISAgentToolKind ToolKind = EIISAgentToolKind::Unknown;
		FString ToolName;
		FString Description;
		TArray<TSharedPtr<FJsonValue>> InputFields;
		FString LatestOutputFileName;
		FString ToolNote;
		FString ExecutionHandler;
	};

	FString MakeAgentJsonPath(const FString& BaseFileName);
	FString GetBaseFileNameForTool(EIISAgentToolKind ToolKind);

	FString BoolToString(const bool bValue)
	{
		return bValue ? TEXT("true") : TEXT("false");
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

	FString AgentToolStatusToString(const EIISAgentToolStatus Status)
	{
		switch (Status)
		{
		case EIISAgentToolStatus::Ready:
			return TEXT("Ready");
		case EIISAgentToolStatus::Empty:
			return TEXT("Empty");
		case EIISAgentToolStatus::Warning:
			return TEXT("Warning");
		case EIISAgentToolStatus::Error:
			return TEXT("Error");
		case EIISAgentToolStatus::Forbidden:
			return TEXT("Forbidden");
		case EIISAgentToolStatus::Unknown:
		default:
			return TEXT("Unknown");
		}
	}

	FString AgentToolKindToString(const EIISAgentToolKind ToolKind)
	{
		switch (ToolKind)
		{
		case EIISAgentToolKind::Search:
			return TEXT("Search");
		case EIISAgentToolKind::GetContextPack:
			return TEXT("GetContextPack");
		case EIISAgentToolKind::GetChunk:
			return TEXT("GetChunk");
		case EIISAgentToolKind::GetSourceReferences:
			return TEXT("GetSourceReferences");
		case EIISAgentToolKind::FindUsages:
			return TEXT("FindUsages");
		case EIISAgentToolKind::ExplainBlueprint:
			return TEXT("ExplainBlueprint");
		case EIISAgentToolKind::Unknown:
		default:
			return TEXT("Unknown");
		}
	}

	FString AgentToolNameForKind(const EIISAgentToolKind ToolKind)
	{
		switch (ToolKind)
		{
		case EIISAgentToolKind::Search:
			return TEXT("iis_search");
		case EIISAgentToolKind::GetContextPack:
			return TEXT("iis_get_context_pack");
		case EIISAgentToolKind::GetChunk:
			return TEXT("iis_get_chunk");
		case EIISAgentToolKind::GetSourceReferences:
			return TEXT("iis_get_source_references");
		case EIISAgentToolKind::FindUsages:
			return TEXT("iis_find_usages");
		case EIISAgentToolKind::ExplainBlueprint:
			return TEXT("iis_explain_blueprint");
		case EIISAgentToolKind::Unknown:
		default:
			return TEXT("iis_unknown");
		}
	}

	bool TryParseAgentToolName(const FString& ToolName, EIISAgentToolKind& OutToolKind)
	{
		const FString Normalized = ToolName.TrimStartAndEnd().ToLower();
		if (Normalized == TEXT("iis_search") || Normalized == TEXT("search"))
		{
			OutToolKind = EIISAgentToolKind::Search;
			return true;
		}
		if (Normalized == TEXT("iis_get_context_pack") || Normalized == TEXT("get_context_pack"))
		{
			OutToolKind = EIISAgentToolKind::GetContextPack;
			return true;
		}
		if (Normalized == TEXT("iis_get_chunk") || Normalized == TEXT("get_chunk"))
		{
			OutToolKind = EIISAgentToolKind::GetChunk;
			return true;
		}
		if (Normalized == TEXT("iis_get_source_references") || Normalized == TEXT("get_source_references"))
		{
			OutToolKind = EIISAgentToolKind::GetSourceReferences;
			return true;
		}
		if (Normalized == TEXT("iis_find_usages") || Normalized == TEXT("find_usages"))
		{
			OutToolKind = EIISAgentToolKind::FindUsages;
			return true;
		}
		if (Normalized == TEXT("iis_explain_blueprint") || Normalized == TEXT("explain_blueprint"))
		{
			OutToolKind = EIISAgentToolKind::ExplainBlueprint;
			return true;
		}

		OutToolKind = EIISAgentToolKind::Unknown;
		return false;
	}

	EIISSearchMode ParseSearchModeFromString(const FString& SearchMode)
	{
		const FString Normalized = SearchMode.TrimStartAndEnd().ToLower();
		if (Normalized == TEXT("lexical"))
		{
			return EIISSearchMode::Lexical;
		}
		if (Normalized == TEXT("vector"))
		{
			return EIISSearchMode::Vector;
		}
		if (Normalized == TEXT("hybrid"))
		{
			return EIISSearchMode::Hybrid;
		}
		return EIISSearchMode::Unknown;
	}

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

	EIISAgentToolStatus AgentStatusFromSearchStatus(const EIISSearchStatus Status)
	{
		switch (Status)
		{
		case EIISSearchStatus::Ready:
			return EIISAgentToolStatus::Ready;
		case EIISSearchStatus::Empty:
			return EIISAgentToolStatus::Empty;
		case EIISSearchStatus::Warning:
			return EIISAgentToolStatus::Warning;
		case EIISSearchStatus::Error:
			return EIISAgentToolStatus::Error;
		case EIISSearchStatus::Unknown:
		default:
			return EIISAgentToolStatus::Unknown;
		}
	}

	EIISAgentToolStatus AgentStatusFromContextPackStatus(const EIISContextPackStatus Status)
	{
		switch (Status)
		{
		case EIISContextPackStatus::Ready:
			return EIISAgentToolStatus::Ready;
		case EIISContextPackStatus::Empty:
			return EIISAgentToolStatus::Empty;
		case EIISContextPackStatus::Warning:
			return EIISAgentToolStatus::Warning;
		case EIISContextPackStatus::Error:
			return EIISAgentToolStatus::Error;
		case EIISContextPackStatus::Unknown:
		default:
			return EIISAgentToolStatus::Unknown;
		}
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

	TArray<FString> MakeDefaultGuardrailStrings()
	{
		TArray<FString> Guardrails;
		Guardrails.Add(TEXT("This response contains retrieved evidence only."));
		Guardrails.Add(TEXT("It does not authorize migration, placement, patching, copying, or project mutation."));
		Guardrails.Add(TEXT("It may reference source evidence, destination evidence, or private review notes depending on filters."));
		Guardrails.Add(TEXT("All claims must be traced back to source references."));
		return Guardrails;
	}

	FString SanitizePathFragment(const FString& Value)
	{
		FString Sanitized;
		Sanitized.Reserve(Value.Len());
		for (const TCHAR Character : Value)
		{
			if (FChar::IsAlnum(Character) || Character == TEXT('_') || Character == TEXT('-'))
			{
				Sanitized.AppendChar(Character);
			}
			else
			{
				Sanitized.AppendChar(TEXT('_'));
			}
		}

		return Sanitized.IsEmpty() ? TEXT("run") : Sanitized.Left(160);
	}

	FString MakeAgentRunId(const EIISAgentToolKind ToolKind, const FString& GeneratedAtUtc)
	{
		return FString::Printf(
			TEXT("%s_%s_%lld"),
			*AgentToolNameForKind(ToolKind),
			*SanitizePathFragment(GeneratedAtUtc),
			FDateTime::UtcNow().GetTicks());
	}

	FString MakeAgentRunDir(const FIISAgentToolResponse& Response)
	{
		return FIISStoragePaths::GetAgentRunsDir() / SanitizePathFragment(Response.RunId);
	}

	void AddDefaultGuardrailsToResponse(FIISAgentToolResponse& Response)
	{
		Response.Guardrails.Reset();

		FIISAgentGuardrail EvidenceOnly;
		EvidenceOnly.GuardrailId = TEXT("evidence_only");
		EvidenceOnly.Text = TEXT("This response contains retrieved evidence only.");
		EvidenceOnly.Severity = TEXT("info");
		Response.Guardrails.Add(EvidenceOnly);

		FIISAgentGuardrail NoMutationAuthority;
		NoMutationAuthority.GuardrailId = TEXT("no_mutation_authority");
		NoMutationAuthority.Text = TEXT("It does not authorize migration, placement, patching, copying, or project mutation.");
		NoMutationAuthority.Severity = TEXT("critical");
		Response.Guardrails.Add(NoMutationAuthority);

		FIISAgentGuardrail SensitivityFilters;
		SensitivityFilters.GuardrailId = TEXT("sensitivity_filters");
		SensitivityFilters.Text = TEXT("It may reference source evidence, destination evidence, or private review notes depending on filters.");
		SensitivityFilters.Severity = TEXT("warning");
		Response.Guardrails.Add(SensitivityFilters);

		FIISAgentGuardrail SourceTraceability;
		SourceTraceability.GuardrailId = TEXT("source_traceability");
		SourceTraceability.Text = TEXT("All claims must be traced back to source references.");
		SourceTraceability.Severity = TEXT("critical");
		Response.Guardrails.Add(SourceTraceability);

		Response.bAllowsMigrationDecision = false;
		Response.bAllowsPatchGeneration = false;
		Response.bAllowsProjectMutation = false;
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

	FString MakeSourceReferenceKey(const FIISSourceReference& Reference)
	{
		return FString::Printf(
			TEXT("%s|%s|%s|%s|%s"),
			*Reference.ArtifactKind,
			*Reference.RelativePath,
			*Reference.JsonPointer,
			*Reference.Fingerprint,
			*Reference.Explanation);
	}

	void AppendUniqueSourceReferences(
		const TArray<FIISSourceReference>& References,
		TArray<FIISSourceReference>& OutReferences)
	{
		TSet<FString> ExistingKeys;
		for (const FIISSourceReference& ExistingReference : OutReferences)
		{
			ExistingKeys.Add(MakeSourceReferenceKey(ExistingReference));
		}

		for (const FIISSourceReference& Reference : References)
		{
			const FString Key = MakeSourceReferenceKey(Reference);
			if (!ExistingKeys.Contains(Key))
			{
				ExistingKeys.Add(Key);
				OutReferences.Add(Reference);
			}
		}
	}

	TArray<TSharedPtr<FJsonValue>> MakeSourceReferencesArray(const TArray<FIISSourceReference>& References)
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

	TSharedRef<FJsonObject> MakeChunkObject(const FIISIndexChunk& Chunk)
	{
		TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetStringField(TEXT("chunk_id"), Chunk.ChunkId);
		Object->SetStringField(TEXT("source_chunk_id"), Chunk.SourceChunkId);
		Object->SetStringField(TEXT("chunk_kind"), ChunkKindToString(Chunk.ChunkKind));
		Object->SetStringField(TEXT("sensitivity"), ChunkSensitivityToString(Chunk.Sensitivity));
		Object->SetStringField(TEXT("title"), Chunk.Title);
		Object->SetStringField(TEXT("text"), Chunk.Text);
		Object->SetStringField(TEXT("module_name"), Chunk.ModuleName);
		Object->SetStringField(TEXT("source_id"), Chunk.SourceId);
		Object->SetStringField(TEXT("source_run_id"), Chunk.SourceRunId);
		Object->SetStringField(TEXT("destination_run_id"), Chunk.DestinationRunId);
		Object->SetArrayField(TEXT("retrieval_labels"), MakeStringArray(Chunk.RetrievalLabels));
		Object->SetArrayField(TEXT("retrieval_groups"), MakeStringArray(Chunk.RetrievalGroups));
		Object->SetArrayField(TEXT("source_references"), MakeSourceReferencesArray(Chunk.SourceReferences));
		Object->SetBoolField(TEXT("is_ai_generated"), Chunk.bIsAiGenerated);
		Object->SetBoolField(TEXT("allows_migration_decision"), Chunk.bAllowsMigrationDecision);
		Object->SetBoolField(TEXT("allows_patch_generation"), Chunk.bAllowsPatchGeneration);
		Object->SetArrayField(TEXT("warnings"), MakeStringArray(Chunk.Warnings));
		return Object;
	}

	TArray<TSharedPtr<FJsonValue>> MakeChunksArray(const TArray<FIISIndexChunk>& Chunks)
	{
		TArray<TSharedPtr<FJsonValue>> Values;
		for (const FIISIndexChunk& Chunk : Chunks)
		{
			Values.Add(MakeShared<FJsonValueObject>(MakeChunkObject(Chunk)));
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
		Object->SetObjectField(TEXT("chunk"), MakeChunkObject(Result.Chunk));
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

	TSharedRef<FJsonObject> MakeSearchResponseObject(const FIISSearchResponse& Response)
	{
		TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetStringField(TEXT("status"), SearchStatusToString(Response.Status));
		Object->SetStringField(TEXT("query_text"), Response.QueryText);
		Object->SetNumberField(TEXT("lexical_candidate_count"), Response.LexicalCandidateCount);
		Object->SetNumberField(TEXT("vector_candidate_count"), Response.VectorCandidateCount);
		Object->SetNumberField(TEXT("merged_candidate_count"), Response.MergedCandidateCount);
		Object->SetNumberField(TEXT("final_result_count"), Response.FinalResultCount);
		Object->SetStringField(TEXT("diagnostics_summary"), Response.DiagnosticsSummary);
		Object->SetArrayField(TEXT("results"), MakeSearchResultsArray(Response.Results));
		Object->SetArrayField(TEXT("warnings"), MakeStringArray(Response.Warnings));
		Object->SetArrayField(TEXT("errors"), MakeStringArray(Response.Errors));
		return Object;
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
			ItemObject->SetArrayField(TEXT("source_references"), MakeSourceReferencesArray(Item.SourceReferences));
			Items.Add(MakeShared<FJsonValueObject>(ItemObject));
		}
		Object->SetArrayField(TEXT("items"), Items);
		return Object;
	}

	TArray<TSharedPtr<FJsonValue>> MakeGuardrailsArray(const TArray<FIISAgentGuardrail>& Guardrails)
	{
		TArray<TSharedPtr<FJsonValue>> Values;
		for (const FIISAgentGuardrail& Guardrail : Guardrails)
		{
			TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
			Object->SetStringField(TEXT("guardrail_id"), Guardrail.GuardrailId);
			Object->SetStringField(TEXT("text"), Guardrail.Text);
			Object->SetStringField(TEXT("severity"), Guardrail.Severity);
			Values.Add(MakeShared<FJsonValueObject>(Object));
		}
		return Values;
	}

	TSharedRef<FJsonObject> MakeAgentRequestObject(const FIISAgentToolRequest& Request)
	{
		TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetStringField(TEXT("tool_kind"), AgentToolKindToString(Request.ToolKind));
		Object->SetStringField(TEXT("query_text"), Request.QueryText);
		Object->SetStringField(TEXT("chunk_id"), Request.ChunkId);
		Object->SetStringField(TEXT("asset_path"), Request.AssetPath);
		Object->SetStringField(TEXT("symbol_name"), Request.SymbolName);
		Object->SetStringField(TEXT("search_mode"), SearchModeToString(Request.SearchMode));
		Object->SetNumberField(TEXT("max_results"), Request.MaxResults);
		Object->SetArrayField(TEXT("required_labels"), MakeStringArray(Request.RequiredLabels));
		Object->SetArrayField(TEXT("preferred_groups"), MakeStringArray(Request.PreferredGroups));
		Object->SetArrayField(TEXT("excluded_sensitivities"), MakeStringArray(Request.ExcludedSensitivities));
		return Object;
	}

	TSharedRef<FJsonObject> MakeUsageGraphObject(const FIISUsageQueryResult& Graph)
	{
		TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetBoolField(TEXT("graph_evidence_available"), Graph.bGraphEvidenceAvailable);

		auto MakeUsageArray = [](const TArray<FIISUsageRecord>& Records) -> TArray<TSharedPtr<FJsonValue>>
		{
			TArray<TSharedPtr<FJsonValue>> Values;
			for (const FIISUsageRecord& Record : Records)
			{
				TSharedRef<FJsonObject> UsageObject = MakeShared<FJsonObject>();
				UsageObject->SetStringField(TEXT("symbol_id"), Record.SymbolId);
				UsageObject->SetStringField(TEXT("usage_kind"), Record.UsageKind == EIISUsageKind::Declaration
					? TEXT("declaration")
					: (Record.UsageKind == EIISUsageKind::Reference ? TEXT("reference") : TEXT("unknown")));
				UsageObject->SetStringField(TEXT("relative_path"), Record.Location.RelativePath);
				UsageObject->SetStringField(TEXT("json_pointer"), Record.Location.JsonPointer);
				Values.Add(MakeShared<FJsonValueObject>(UsageObject));
			}
			return Values;
		};

		auto MakeCallArray = [](const TArray<FIISCallEdge>& Edges) -> TArray<TSharedPtr<FJsonValue>>
		{
			TArray<TSharedPtr<FJsonValue>> Values;
			for (const FIISCallEdge& Edge : Edges)
			{
				TSharedRef<FJsonObject> EdgeObject = MakeShared<FJsonObject>();
				EdgeObject->SetStringField(TEXT("caller_symbol_id"), Edge.CallerSymbolId);
				EdgeObject->SetStringField(TEXT("callee_symbol_id"), Edge.CalleeSymbolId);
				EdgeObject->SetStringField(TEXT("relative_path"), Edge.Evidence.RelativePath);
				Values.Add(MakeShared<FJsonValueObject>(EdgeObject));
			}
			return Values;
		};

		Object->SetArrayField(TEXT("declarations"), MakeUsageArray(Graph.Declarations));
		Object->SetArrayField(TEXT("references"), MakeUsageArray(Graph.References));
		Object->SetArrayField(TEXT("calls"), MakeCallArray(Graph.Calls));
		Object->SetArrayField(TEXT("module_refs"), MakeStringArray(Graph.ModuleRefs));
		Object->SetArrayField(TEXT("warnings"), MakeStringArray(Graph.Warnings));
		return Object;
	}

	TSharedRef<FJsonObject> MakeBlueprintExplanationObject(const FIISBlueprintExplanation& Explanation)
	{
		TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetBoolField(TEXT("ir_available"), Explanation.bIRAvailable);
		Object->SetStringField(TEXT("blueprint_name"), Explanation.BlueprintName);
		Object->SetStringField(TEXT("asset_path"), Explanation.AssetPath);
		Object->SetStringField(TEXT("parent_class"), Explanation.ParentClass);

		TArray<TSharedPtr<FJsonValue>> GraphValues;
		for (const FIISBlueprintGraphSummary& Graph : Explanation.Graphs)
		{
			TSharedRef<FJsonObject> GraphObject = MakeShared<FJsonObject>();
			GraphObject->SetStringField(TEXT("graph_name"), Graph.GraphName);
			GraphObject->SetStringField(TEXT("graph_kind"), Graph.GraphKind);
			GraphObject->SetNumberField(TEXT("node_count"), Graph.NodeCount);
			GraphValues.Add(MakeShared<FJsonValueObject>(GraphObject));
		}
		Object->SetArrayField(TEXT("graphs"), GraphValues);
		Object->SetArrayField(TEXT("events_and_functions"), MakeStringArray(Explanation.EventsAndFunctions));
		Object->SetArrayField(TEXT("variables"), MakeStringArray(Explanation.Variables));
		Object->SetArrayField(TEXT("components"), MakeStringArray(Explanation.Components));
		Object->SetArrayField(TEXT("referenced_assets"), MakeStringArray(Explanation.ReferencedAssets));
		Object->SetArrayField(TEXT("referenced_classes_or_functions"), MakeStringArray(Explanation.ReferencedClassesOrFunctions));
		Object->SetArrayField(TEXT("unsupported_nodes"), MakeStringArray(Explanation.UnsupportedNodes));
		Object->SetArrayField(TEXT("network_authority_hints"), MakeStringArray(Explanation.NetworkAuthorityHints));
		Object->SetArrayField(TEXT("source_references"), MakeSourceReferencesArray(Explanation.SourceReferences));
		Object->SetArrayField(TEXT("guardrails"), MakeStringArray(Explanation.Guardrails));
		Object->SetArrayField(TEXT("warnings"), MakeStringArray(Explanation.Warnings));
		return Object;
	}

	TSharedRef<FJsonObject> MakeAgentResponseObject(const FIISAgentToolResponse& Response)
	{
		TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetStringField(TEXT("schema_version"), Response.SchemaVersion);
		Object->SetStringField(TEXT("tool_name"), Response.ToolName);
		Object->SetStringField(TEXT("generated_at_utc"), Response.GeneratedAtUtc);
		Object->SetStringField(TEXT("run_id"), Response.RunId);
		Object->SetStringField(TEXT("run_directory"), MakeAgentRunDir(Response));
		Object->SetStringField(TEXT("latest_response_path"), MakeAgentJsonPath(GetBaseFileNameForTool(Response.ToolKind)));
		Object->SetStringField(TEXT("tool_kind"), AgentToolKindToString(Response.ToolKind));
		Object->SetStringField(TEXT("status"), AgentToolStatusToString(Response.Status));
		Object->SetObjectField(TEXT("request"), MakeAgentRequestObject(Response.Request));
		Object->SetObjectField(TEXT("search_response"), MakeSearchResponseObject(Response.SearchResponse));
		Object->SetObjectField(TEXT("context_pack"), MakeContextPackObject(Response.ContextPack));
		Object->SetArrayField(TEXT("chunks"), MakeChunksArray(Response.Chunks));
		Object->SetArrayField(TEXT("source_references"), MakeSourceReferencesArray(Response.SourceReferences));
		Object->SetArrayField(TEXT("guardrails"), MakeGuardrailsArray(Response.Guardrails));
		Object->SetBoolField(TEXT("allows_migration_decision"), Response.bAllowsMigrationDecision);
		Object->SetBoolField(TEXT("allows_patch_generation"), Response.bAllowsPatchGeneration);
		Object->SetBoolField(TEXT("allows_project_mutation"), Response.bAllowsProjectMutation);
		Object->SetArrayField(TEXT("warnings"), MakeStringArray(Response.Warnings));
		Object->SetArrayField(TEXT("errors"), MakeStringArray(Response.Errors));
		Object->SetObjectField(TEXT("usage_graph"), MakeUsageGraphObject(Response.UsageGraph));
		Object->SetObjectField(TEXT("blueprint_explanation"), MakeBlueprintExplanationObject(Response.BlueprintExplanation));
		return Object;
	}

	FIISSearchQuery MakeSearchQueryFromAgentRequest(const FIISAgentToolRequest& Request)
	{
		FIISSearchQuery Query;
		Query.QueryText = Request.QueryText;
		Query.SearchMode = Request.SearchMode == EIISSearchMode::Unknown ? EIISSearchMode::Hybrid : Request.SearchMode;
		Query.MaxResults = Request.MaxResults <= 0 ? 10 : Request.MaxResults;
		Query.RequiredLabels = Request.RequiredLabels;
		Query.PreferredGroups = Request.PreferredGroups;
		Query.ExcludedSensitivities = Request.ExcludedSensitivities;
		return Query;
	}

	void InitializeResponse(
		const FIISAgentToolRequest& Request,
		const EIISAgentToolKind ToolKind,
		FIISAgentToolResponse& OutResponse)
	{
		OutResponse = FIISAgentToolResponse();
		OutResponse.GeneratedAtUtc = FDateTime::UtcNow().ToIso8601();
		OutResponse.ToolKind = ToolKind;
		OutResponse.RunId = MakeAgentRunId(ToolKind, OutResponse.GeneratedAtUtc);
		OutResponse.Request = Request;
		OutResponse.Request.ToolKind = ToolKind;
		OutResponse.bAllowsMigrationDecision = false;
		OutResponse.bAllowsPatchGeneration = false;
		OutResponse.bAllowsProjectMutation = false;
		AddDefaultGuardrailsToResponse(OutResponse);
	}

	void AppendSearchEvidenceToAgentResponse(FIISAgentToolResponse& Response)
	{
		for (const FIISSearchResult& Result : Response.SearchResponse.Results)
		{
			Response.Chunks.Add(Result.Chunk);
			AppendUniqueSourceReferences(Result.Chunk.SourceReferences, Response.SourceReferences);
		}
	}

	void AppendContextPackEvidenceToAgentResponse(FIISAgentToolResponse& Response)
	{
		for (const FIISContextPackItem& Item : Response.ContextPack.Items)
		{
			AppendUniqueSourceReferences(Item.SourceReferences, Response.SourceReferences);
		}
	}

	FString MakeAgentJsonPath(const FString& BaseFileName)
	{
		return FIISStoragePaths::GetAgentDir() / FString::Printf(TEXT("%s.json"), *BaseFileName);
	}

	FString MakeAgentMarkdownPath(const FString& BaseFileName)
	{
		return FIISStoragePaths::GetAgentDir() / FString::Printf(TEXT("%s.md"), *BaseFileName);
	}

	FString GetBaseFileNameForTool(const EIISAgentToolKind ToolKind)
	{
		switch (ToolKind)
		{
		case EIISAgentToolKind::Search:
			return AgentSearchBaseName;
		case EIISAgentToolKind::GetContextPack:
			return AgentContextPackBaseName;
		case EIISAgentToolKind::GetChunk:
			return AgentChunkLookupBaseName;
		case EIISAgentToolKind::GetSourceReferences:
			return AgentSourceReferencesBaseName;
		case EIISAgentToolKind::FindUsages:
			return AgentUsageReportBaseName;
		case EIISAgentToolKind::ExplainBlueprint:
			return AgentBlueprintExplanationBaseName;
		case EIISAgentToolKind::Unknown:
		default:
			return TEXT("latest_agent_response");
		}
	}

	TArray<TSharedPtr<FJsonValue>> MakeForbiddenActionsArray()
	{
		TArray<FString> ForbiddenActions;
		ForbiddenActions.Add(TEXT("AI answer generation"));
		ForbiddenActions.Add(TEXT("LLM reranking"));
		ForbiddenActions.Add(TEXT("Patch generation"));
		ForbiddenActions.Add(TEXT("Asset, Blueprint, or project mutation"));
		ForbiddenActions.Add(TEXT("Source or asset copying"));
		ForbiddenActions.Add(TEXT("Source/destination mapping"));
		ForbiddenActions.Add(TEXT("Migration planning"));
		ForbiddenActions.Add(TEXT("Remote API calls"));
		return MakeStringArray(ForbiddenActions);
	}

	TSharedPtr<FJsonValue> MakeInputField(
		const FString& Name,
		const FString& Type,
		const bool bRequired,
		const FString& Description)
	{
		TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetStringField(TEXT("name"), Name);
		Object->SetStringField(TEXT("type"), Type);
		Object->SetBoolField(TEXT("required"), bRequired);
		Object->SetStringField(TEXT("description"), Description);
		return MakeShared<FJsonValueObject>(Object);
	}

	TSharedRef<FJsonObject> MakeJsonSchemaForInputFields(const TArray<TSharedPtr<FJsonValue>>& InputFields)
	{
		TSharedRef<FJsonObject> Schema = MakeShared<FJsonObject>();
		TSharedRef<FJsonObject> Properties = MakeShared<FJsonObject>();
		TArray<TSharedPtr<FJsonValue>> Required;

		Schema->SetStringField(TEXT("type"), TEXT("object"));
		for (const TSharedPtr<FJsonValue>& FieldValue : InputFields)
		{
			if (!FieldValue.IsValid())
			{
				continue;
			}

			const TSharedPtr<FJsonObject> Field = FieldValue->AsObject();
			if (!Field.IsValid())
			{
				continue;
			}

			const FString Name = Field->GetStringField(TEXT("name"));
			const FString Type = Field->GetStringField(TEXT("type"));
			const FString Description = Field->GetStringField(TEXT("description"));
			const bool bRequired = Field->GetBoolField(TEXT("required"));

			TSharedRef<FJsonObject> Property = MakeShared<FJsonObject>();
			Property->SetStringField(TEXT("description"), Description);
			if (Type.EndsWith(TEXT("[]")))
			{
				TSharedRef<FJsonObject> Items = MakeShared<FJsonObject>();
				Items->SetStringField(TEXT("type"), TEXT("string"));
				Property->SetStringField(TEXT("type"), TEXT("array"));
				Property->SetObjectField(TEXT("items"), Items);
			}
			else if (Type == TEXT("integer"))
			{
				Property->SetStringField(TEXT("type"), TEXT("integer"));
			}
			else
			{
				Property->SetStringField(TEXT("type"), TEXT("string"));
				if (Type.Contains(TEXT("|")))
				{
					TArray<FString> EnumValues;
					Type.ParseIntoArray(EnumValues, TEXT("|"), true);
					Property->SetArrayField(TEXT("enum"), MakeStringArray(EnumValues));
				}
			}

			Properties->SetObjectField(Name, Property);
			if (bRequired)
			{
				Required.Add(MakeShared<FJsonValueString>(Name));
			}
		}

		Schema->SetObjectField(TEXT("properties"), Properties);
		Schema->SetArrayField(TEXT("required"), Required);
		Schema->SetBoolField(TEXT("additionalProperties"), false);
		return Schema;
	}

	TSharedRef<FJsonObject> MakeAgentOutputJsonSchema()
	{
		TSharedRef<FJsonObject> Schema = MakeShared<FJsonObject>();
		TSharedRef<FJsonObject> Properties = MakeShared<FJsonObject>();

		Schema->SetStringField(TEXT("type"), TEXT("object"));
		Properties->SetObjectField(TEXT("schema_version"), MakeShared<FJsonObject>());
		Properties->SetObjectField(TEXT("tool_name"), MakeShared<FJsonObject>());
		Properties->SetObjectField(TEXT("generated_at_utc"), MakeShared<FJsonObject>());
		Properties->SetObjectField(TEXT("run_id"), MakeShared<FJsonObject>());
		Properties->SetObjectField(TEXT("tool_kind"), MakeShared<FJsonObject>());
		Properties->SetObjectField(TEXT("status"), MakeShared<FJsonObject>());
		Properties->SetObjectField(TEXT("request"), MakeShared<FJsonObject>());
		Properties->SetObjectField(TEXT("search_response"), MakeShared<FJsonObject>());
		Properties->SetObjectField(TEXT("context_pack"), MakeShared<FJsonObject>());
		Properties->SetObjectField(TEXT("chunks"), MakeShared<FJsonObject>());
		Properties->SetObjectField(TEXT("source_references"), MakeShared<FJsonObject>());
		Properties->SetObjectField(TEXT("guardrails"), MakeShared<FJsonObject>());
		Properties->SetObjectField(TEXT("allows_migration_decision"), MakeShared<FJsonObject>());
		Properties->SetObjectField(TEXT("allows_patch_generation"), MakeShared<FJsonObject>());
		Properties->SetObjectField(TEXT("allows_project_mutation"), MakeShared<FJsonObject>());
		Properties->SetObjectField(TEXT("warnings"), MakeShared<FJsonObject>());
		Properties->SetObjectField(TEXT("errors"), MakeShared<FJsonObject>());
		Schema->SetObjectField(TEXT("properties"), Properties);
		Schema->SetStringField(TEXT("description"), TEXT("FIISAgentToolResponse JSON with guardrails, evidence references, mutation flags, diagnostics, warnings, and errors."));
		return Schema;
	}

	TSharedRef<FJsonObject> MakeToolSchema(
		const FString& Name,
		const FString& Description,
		const TArray<TSharedPtr<FJsonValue>>& InputFields,
		const FString& OutputFileName,
		const FString& ToolNote)
	{
		TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetStringField(TEXT("name"), Name);
		Object->SetStringField(TEXT("description"), Description);
		Object->SetArrayField(TEXT("input_fields"), InputFields);
		Object->SetObjectField(TEXT("input_schema"), MakeJsonSchemaForInputFields(InputFields));
		Object->SetObjectField(TEXT("output_schema"), MakeAgentOutputJsonSchema());
		Object->SetStringField(TEXT("output_file_shape"), TEXT("FIISAgentToolResponse JSON with guardrails, request, warnings, errors, evidence, and source references."));
		Object->SetStringField(TEXT("latest_output_file"), FIISStoragePaths::GetAgentDir() / OutputFileName);
		Object->SetStringField(TEXT("note"), ToolNote);
		Object->SetArrayField(TEXT("guardrails"), MakeStringArray(MakeDefaultGuardrailStrings()));
		Object->SetArrayField(TEXT("forbidden_actions"), MakeForbiddenActionsArray());
		return Object;
	}

	TArray<TSharedPtr<FJsonValue>> MakeSearchInputFields()
	{
		TArray<TSharedPtr<FJsonValue>> Fields;
		Fields.Add(MakeInputField(TEXT("query_text"), TEXT("string"), true, TEXT("Local evidence query text.")));
		Fields.Add(MakeInputField(TEXT("search_mode"), TEXT("lexical|vector|hybrid"), false, TEXT("Retrieval mode. Defaults to hybrid.")));
		Fields.Add(MakeInputField(TEXT("max_results"), TEXT("integer"), false, TEXT("Maximum result count.")));
		Fields.Add(MakeInputField(TEXT("required_labels"), TEXT("string[]"), false, TEXT("Labels that every result must contain.")));
		Fields.Add(MakeInputField(TEXT("preferred_groups"), TEXT("string[]"), false, TEXT("Groups that affect ranking but do not hard-filter.")));
		Fields.Add(MakeInputField(TEXT("excluded_sensitivities"), TEXT("string[]"), false, TEXT("Sensitivities to exclude from results.")));
		return Fields;
	}

	TArray<TSharedPtr<FJsonValue>> MakeChunkInputFields()
	{
		TArray<TSharedPtr<FJsonValue>> Fields;
		Fields.Add(MakeInputField(TEXT("chunk_id"), TEXT("string"), true, TEXT("IIS chunk identifier.")));
		return Fields;
	}

	TArray<TSharedPtr<FJsonValue>> MakeUsageInputFields()
	{
		TArray<TSharedPtr<FJsonValue>> Fields;
		Fields.Add(MakeInputField(TEXT("symbol_name"), TEXT("string"), true, TEXT("Symbol or token to search lexically in indexed evidence.")));
		Fields.Add(MakeInputField(TEXT("max_results"), TEXT("integer"), false, TEXT("Maximum evidence result count.")));
		return Fields;
	}

	TArray<TSharedPtr<FJsonValue>> MakeBlueprintInputFields()
	{
		TArray<TSharedPtr<FJsonValue>> Fields;
		Fields.Add(MakeInputField(TEXT("asset_path_or_query"), TEXT("string"), true, TEXT("Blueprint asset path, name, or evidence query.")));
		Fields.Add(MakeInputField(TEXT("max_results"), TEXT("integer"), false, TEXT("Maximum evidence result count.")));
		return Fields;
	}

	TArray<FIISAgentToolDefinition> MakeAgentToolDefinitions()
	{
		TArray<FIISAgentToolDefinition> Definitions;

		FIISAgentToolDefinition Search;
		Search.ToolKind = EIISAgentToolKind::Search;
		Search.ToolName = TEXT("iis_search");
		Search.Description = TEXT("Search local IIS evidence with lexical, vector, or hybrid retrieval.");
		Search.InputFields = MakeSearchInputFields();
		Search.LatestOutputFileName = TEXT("latest_agent_search_response.json");
		Search.ToolNote = TEXT("Delegates to IIS search and preserves ranking diagnostics.");
		Search.ExecutionHandler = TEXT("FIISAgentAccessService::Search");
		Definitions.Add(Search);

		FIISAgentToolDefinition ContextPack;
		ContextPack.ToolKind = EIISAgentToolKind::GetContextPack;
		ContextPack.ToolName = TEXT("iis_get_context_pack");
		ContextPack.Description = TEXT("Build a retrieval-only context pack for an evidence query.");
		ContextPack.InputFields = MakeSearchInputFields();
		ContextPack.LatestOutputFileName = TEXT("latest_agent_context_pack.json");
		ContextPack.ToolNote = TEXT("Delegates to IIS context-pack construction and preserves guardrails.");
		ContextPack.ExecutionHandler = TEXT("FIISAgentAccessService::GetContextPack");
		Definitions.Add(ContextPack);

		FIISAgentToolDefinition Chunk;
		Chunk.ToolKind = EIISAgentToolKind::GetChunk;
		Chunk.ToolName = TEXT("iis_get_chunk");
		Chunk.Description = TEXT("Load one exact IIS chunk by chunk_id.");
		Chunk.InputFields = MakeChunkInputFields();
		Chunk.LatestOutputFileName = TEXT("latest_agent_chunk_lookup.json");
		Chunk.ToolNote = TEXT("Performs exact local lookup only; no search or AI execution.");
		Chunk.ExecutionHandler = TEXT("FIISAgentAccessService::GetChunk");
		Definitions.Add(Chunk);

		FIISAgentToolDefinition SourceReferences;
		SourceReferences.ToolKind = EIISAgentToolKind::GetSourceReferences;
		SourceReferences.ToolName = TEXT("iis_get_source_references");
		SourceReferences.Description = TEXT("Load source references for one exact IIS chunk by chunk_id.");
		SourceReferences.InputFields = MakeChunkInputFields();
		SourceReferences.LatestOutputFileName = TEXT("latest_agent_source_references.json");
		SourceReferences.ToolNote = TEXT("Returns citation/evidence pointers only.");
		SourceReferences.ExecutionHandler = TEXT("FIISAgentAccessService::GetSourceReferences");
		Definitions.Add(SourceReferences);

		FIISAgentToolDefinition Usages;
		Usages.ToolKind = EIISAgentToolKind::FindUsages;
		Usages.ToolName = TEXT("iis_find_usages");
		Usages.Description = TEXT(
			"Find symbol usages from the imported usage graph (declarations, references, calls, asset and blueprint refs); "
			"falls back to lexical chunk search when graph evidence is unavailable.");
		Usages.InputFields = MakeUsageInputFields();
		Usages.LatestOutputFileName = TEXT("latest_agent_usage_report.json");
		Usages.ToolNote = TEXT(
			"Returns categorized graph evidence when the usage graph is imported; otherwise lexical fallback with an explicit warning.");
		Usages.ExecutionHandler = TEXT("FIISAgentAccessService::FindUsages");
		Definitions.Add(Usages);

		FIISAgentToolDefinition Blueprint;
		Blueprint.ToolKind = EIISAgentToolKind::ExplainBlueprint;
		Blueprint.ToolName = TEXT("iis_explain_blueprint");
		Blueprint.Description = TEXT(
			"Return a deterministic structured Blueprint explanation assembled from imported UII Blueprint IR when available.");
		Blueprint.InputFields = MakeBlueprintInputFields();
		Blueprint.LatestOutputFileName = TEXT("latest_agent_blueprint_explanation.json");
		Blueprint.ToolNote = TEXT(
			"IR-based structured explanation (graphs, variables, references, network hints); search fallback when IR is not imported.");
		Blueprint.ExecutionHandler = TEXT("FIISAgentAccessService::ExplainBlueprint");
		Definitions.Add(Blueprint);

		return Definitions;
	}

	TSharedRef<FJsonObject> MakeToolSchemaFromDefinition(const FIISAgentToolDefinition& Definition)
	{
		TSharedRef<FJsonObject> Schema = MakeToolSchema(
			Definition.ToolName,
			Definition.Description,
			Definition.InputFields,
			Definition.LatestOutputFileName,
			Definition.ToolNote);
		Schema->SetStringField(TEXT("tool_kind"), AgentToolKindToString(Definition.ToolKind));
		Schema->SetStringField(TEXT("execution_handler"), Definition.ExecutionHandler);
		return Schema;
	}

	TSharedRef<FJsonObject> MakeRegistryToolObject(const FIISAgentToolDefinition& Definition)
	{
		TSharedRef<FJsonObject> Object = MakeToolSchemaFromDefinition(Definition);
		Object->SetStringField(TEXT("tool_name"), Definition.ToolName);
		Object->SetStringField(TEXT("local_invocation_notes"), TEXT("Invoke locally by writing an input JSON file and calling ExecuteAgentToolFromJson(InputJsonPath, OutResponseJsonPath)."));
		return Object;
	}

	void WriteSingleSchema(const FString& FileName, const TSharedRef<FJsonObject>& Schema)
	{
		SaveJsonObjectToFile(Schema, FIISStoragePaths::GetAgentContractsDir() / FileName);
	}

	bool LoadJsonObjectFromFile(
		const FString& JsonPath,
		TSharedPtr<FJsonObject>& OutObject,
		TArray<FString>& OutErrors)
	{
		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *JsonPath))
		{
			OutErrors.Add(FString::Printf(TEXT("Failed to read local IIS agent invocation JSON: %s"), *JsonPath));
			return false;
		}

		const TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(JsonText);
		if (!FJsonSerializer::Deserialize(Reader, OutObject) || !OutObject.IsValid())
		{
			OutErrors.Add(FString::Printf(TEXT("Failed to parse local IIS agent invocation JSON: %s"), *JsonPath));
			return false;
		}

		return true;
	}

	TSharedPtr<FJsonObject> GetInvocationArguments(const TSharedPtr<FJsonObject>& Invocation)
	{
		if (!Invocation.IsValid())
		{
			return MakeShared<FJsonObject>();
		}

		const TSharedPtr<FJsonObject>* Arguments = nullptr;
		if (Invocation->TryGetObjectField(TEXT("arguments"), Arguments) && Arguments && Arguments->IsValid())
		{
			return *Arguments;
		}

		return MakeShared<FJsonObject>();
	}

	FString GetStringArgument(const TSharedPtr<FJsonObject>& Arguments, const FString& FieldName)
	{
		FString Value;
		if (Arguments.IsValid())
		{
			Arguments->TryGetStringField(FieldName, Value);
		}
		return Value;
	}

	int32 GetIntArgument(const TSharedPtr<FJsonObject>& Arguments, const FString& FieldName, const int32 DefaultValue)
	{
		if (!Arguments.IsValid())
		{
			return DefaultValue;
		}

		double NumberValue = 0.0;
		if (Arguments->TryGetNumberField(FieldName, NumberValue))
		{
			return static_cast<int32>(NumberValue);
		}

		return DefaultValue;
	}

	TArray<FString> GetStringArrayArgument(const TSharedPtr<FJsonObject>& Arguments, const FString& FieldName)
	{
		TArray<FString> Values;
		if (!Arguments.IsValid())
		{
			return Values;
		}

		const TArray<TSharedPtr<FJsonValue>>* JsonValues = nullptr;
		if (Arguments->TryGetArrayField(FieldName, JsonValues) && JsonValues)
		{
			for (const TSharedPtr<FJsonValue>& JsonValue : *JsonValues)
			{
				if (JsonValue.IsValid())
				{
					Values.Add(JsonValue->AsString());
				}
			}
			return Values;
		}

		FString SingleValue;
		if (Arguments->TryGetStringField(FieldName, SingleValue) && !SingleValue.TrimStartAndEnd().IsEmpty())
		{
			Values.Add(SingleValue);
		}

		return Values;
	}

	void ApplyCommonSearchArguments(
		const TSharedPtr<FJsonObject>& Arguments,
		FIISAgentToolRequest& Request)
	{
		Request.QueryText = GetStringArgument(Arguments, TEXT("query_text"));
		const EIISSearchMode ParsedSearchMode = ParseSearchModeFromString(GetStringArgument(Arguments, TEXT("search_mode")));
		Request.SearchMode = ParsedSearchMode == EIISSearchMode::Unknown ? EIISSearchMode::Hybrid : ParsedSearchMode;
		Request.MaxResults = GetIntArgument(Arguments, TEXT("max_results"), 10);
		Request.RequiredLabels = GetStringArrayArgument(Arguments, TEXT("required_labels"));
		Request.PreferredGroups = GetStringArrayArgument(Arguments, TEXT("preferred_groups"));
		Request.ExcludedSensitivities = GetStringArrayArgument(Arguments, TEXT("excluded_sensitivities"));
	}

	bool BuildAgentRequestFromInvocation(
		const FString& ToolName,
		const TSharedPtr<FJsonObject>& Arguments,
		FIISAgentToolRequest& OutRequest,
		TArray<FString>& OutErrors)
	{
		EIISAgentToolKind ToolKind = EIISAgentToolKind::Unknown;
		if (!TryParseAgentToolName(ToolName, ToolKind))
		{
			OutErrors.Add(FString::Printf(TEXT("Unknown IIS agent tool: %s"), *ToolName));
			return false;
		}

		OutRequest = FIISAgentToolRequest();
		OutRequest.ToolKind = ToolKind;
		switch (ToolKind)
		{
		case EIISAgentToolKind::Search:
		case EIISAgentToolKind::GetContextPack:
			ApplyCommonSearchArguments(Arguments, OutRequest);
			break;
		case EIISAgentToolKind::GetChunk:
		case EIISAgentToolKind::GetSourceReferences:
			OutRequest.ChunkId = GetStringArgument(Arguments, TEXT("chunk_id"));
			break;
		case EIISAgentToolKind::FindUsages:
			OutRequest.SymbolName = GetStringArgument(Arguments, TEXT("symbol_name"));
			OutRequest.QueryText = GetStringArgument(Arguments, TEXT("query_text"));
			OutRequest.MaxResults = GetIntArgument(Arguments, TEXT("max_results"), 10);
			break;
		case EIISAgentToolKind::ExplainBlueprint:
			OutRequest.AssetPath = GetStringArgument(Arguments, TEXT("asset_path_or_query"));
			if (OutRequest.AssetPath.IsEmpty())
			{
				OutRequest.AssetPath = GetStringArgument(Arguments, TEXT("asset_path"));
			}
			OutRequest.QueryText = GetStringArgument(Arguments, TEXT("query_text"));
			OutRequest.MaxResults = GetIntArgument(Arguments, TEXT("max_results"), 10);
			OutRequest.SearchMode = EIISSearchMode::Hybrid;
			break;
		case EIISAgentToolKind::Unknown:
		default:
			OutErrors.Add(TEXT("Unknown IIS agent tool kind."));
			return false;
		}

		return true;
	}

	bool WriteInvocationResult(
		const FString& ToolName,
		const FIISAgentToolResponse& Response,
		FString& OutResponseJsonPath)
	{
		const FString RunDir = MakeAgentRunDir(Response);
		IFileManager::Get().MakeDirectory(*RunDir, true);

		TSharedRef<FJsonObject> Result = MakeShared<FJsonObject>();
		Result->SetStringField(TEXT("schema_version"), TEXT("0.1.0"));
		Result->SetStringField(TEXT("tool"), ToolName.IsEmpty() ? AgentToolNameForKind(Response.ToolKind) : ToolName);
		Result->SetStringField(TEXT("status"), AgentToolStatusToString(Response.Status));
		Result->SetStringField(TEXT("response_path"), RunDir / TEXT("response.json"));
		Result->SetStringField(TEXT("response_markdown_path"), RunDir / TEXT("response.md"));
		Result->SetStringField(TEXT("latest_response_path"), MakeAgentJsonPath(GetBaseFileNameForTool(Response.ToolKind)));
		Result->SetStringField(TEXT("run_directory"), RunDir);
		Result->SetStringField(TEXT("run_id"), Response.RunId);
		Result->SetArrayField(TEXT("guardrails"), MakeGuardrailsArray(Response.Guardrails));
		Result->SetBoolField(TEXT("allows_migration_decision"), Response.bAllowsMigrationDecision);
		Result->SetBoolField(TEXT("allows_patch_generation"), Response.bAllowsPatchGeneration);
		Result->SetBoolField(TEXT("allows_project_mutation"), Response.bAllowsProjectMutation);
		Result->SetArrayField(TEXT("warnings"), MakeStringArray(Response.Warnings));
		Result->SetArrayField(TEXT("errors"), MakeStringArray(Response.Errors));

		OutResponseJsonPath = RunDir / AgentInvocationResultFileName;
		return SaveJsonObjectToFile(Result, OutResponseJsonPath);
	}
}

bool FIISAgentAccessService::ExecuteAgentTool(
	const FIISAgentToolRequest& Request,
	FIISAgentToolResponse& OutResponse)
{
	switch (Request.ToolKind)
	{
	case EIISAgentToolKind::Search:
		return Search(Request, OutResponse);
	case EIISAgentToolKind::GetContextPack:
		return GetContextPack(Request, OutResponse);
	case EIISAgentToolKind::GetChunk:
		return GetChunk(Request.ChunkId, OutResponse);
	case EIISAgentToolKind::GetSourceReferences:
		return GetSourceReferences(Request.ChunkId, OutResponse);
	case EIISAgentToolKind::FindUsages:
		return FindUsages(Request, OutResponse);
	case EIISAgentToolKind::ExplainBlueprint:
		return ExplainBlueprint(Request, OutResponse);
	case EIISAgentToolKind::Unknown:
	default:
		InitializeResponse(Request, EIISAgentToolKind::Unknown, OutResponse);
		OutResponse.Status = EIISAgentToolStatus::Error;
		OutResponse.Errors.Add(TEXT("Unknown IIS agent tool kind."));
		WriteAgentResponse(OutResponse, GetBaseFileNameForTool(EIISAgentToolKind::Unknown));
		return false;
	}
}

bool FIISAgentAccessService::ExecuteAgentToolFromJson(
	const FString& InputJsonPath,
	FString& OutResponseJsonPath)
{
	OutResponseJsonPath.Reset();
	FIISStoragePaths::EnsureDefaultFolders();

	TArray<FString> Errors;
	TSharedPtr<FJsonObject> Invocation;
	FString ToolName;
	FIISAgentToolRequest Request;

	const bool bLoaded = LoadJsonObjectFromFile(InputJsonPath, Invocation, Errors);
	if (bLoaded)
	{
		Invocation->TryGetStringField(TEXT("tool"), ToolName);
		const TSharedPtr<FJsonObject> Arguments = GetInvocationArguments(Invocation);
		BuildAgentRequestFromInvocation(ToolName, Arguments, Request, Errors);
	}

	FIISAgentToolResponse Response;
	if (Errors.Num() > 0)
	{
		InitializeResponse(Request, Request.ToolKind, Response);
		Response.Status = EIISAgentToolStatus::Error;
		Response.Errors = Errors;
		WriteAgentResponse(Response, GetBaseFileNameForTool(Response.ToolKind));
		WriteInvocationResult(ToolName, Response, OutResponseJsonPath);
		return false;
	}

	const bool bExecuted = ExecuteAgentTool(Request, Response);
	const bool bInvocationSaved = WriteInvocationResult(ToolName, Response, OutResponseJsonPath);
	return bExecuted && bInvocationSaved && Response.Status != EIISAgentToolStatus::Error;
}

bool FIISAgentAccessService::Search(
	const FIISAgentToolRequest& Request,
	FIISAgentToolResponse& OutResponse)
{
	FIISAgentToolRequest EffectiveRequest = Request;
	EffectiveRequest.ToolKind = EIISAgentToolKind::Search;
	InitializeResponse(EffectiveRequest, EIISAgentToolKind::Search, OutResponse);

	const FIISSearchQuery Query = MakeSearchQueryFromAgentRequest(EffectiveRequest);
	FIISLocalIndexService Service;
	const bool bSearchOk = Service.Search(Query, OutResponse.SearchResponse);
	OutResponse.Status = AgentStatusFromSearchStatus(OutResponse.SearchResponse.Status);
	OutResponse.Warnings.Append(OutResponse.SearchResponse.Warnings);
	OutResponse.Errors.Append(OutResponse.SearchResponse.Errors);
	AppendSearchEvidenceToAgentResponse(OutResponse);

	WriteAgentResponse(OutResponse, AgentSearchBaseName);
	return bSearchOk && OutResponse.Status != EIISAgentToolStatus::Error;
}

bool FIISAgentAccessService::GetContextPack(
	const FIISAgentToolRequest& Request,
	FIISAgentToolResponse& OutResponse)
{
	FIISAgentToolRequest EffectiveRequest = Request;
	EffectiveRequest.ToolKind = EIISAgentToolKind::GetContextPack;
	InitializeResponse(EffectiveRequest, EIISAgentToolKind::GetContextPack, OutResponse);

	const FIISSearchQuery Query = MakeSearchQueryFromAgentRequest(EffectiveRequest);
	FIISLocalIndexService Service;
	const bool bContextOk = Service.BuildContextPack(Query, OutResponse.ContextPack);
	OutResponse.Status = AgentStatusFromContextPackStatus(OutResponse.ContextPack.Status);
	OutResponse.Warnings.Append(OutResponse.ContextPack.Warnings);
	AppendContextPackEvidenceToAgentResponse(OutResponse);

	WriteAgentResponse(OutResponse, AgentContextPackBaseName);
	return bContextOk && OutResponse.Status != EIISAgentToolStatus::Error;
}

bool FIISAgentAccessService::GetChunk(
	const FString& ChunkId,
	FIISAgentToolResponse& OutResponse)
{
	FIISAgentToolRequest Request;
	Request.ToolKind = EIISAgentToolKind::GetChunk;
	Request.ChunkId = ChunkId;
	InitializeResponse(Request, EIISAgentToolKind::GetChunk, OutResponse);

	FIISIndexChunk Chunk;
	TArray<FString> Warnings;
	const bool bLoaded = FIISChunkCatalog::LoadChunkById(ChunkId, Chunk, Warnings);
	OutResponse.Warnings.Append(Warnings);
	if (bLoaded)
	{
		OutResponse.Status = EIISAgentToolStatus::Ready;
		OutResponse.Chunks.Add(Chunk);
		OutResponse.SourceReferences = Chunk.SourceReferences;
	}
	else
	{
		OutResponse.Status = EIISAgentToolStatus::Empty;
	}

	WriteAgentResponse(OutResponse, AgentChunkLookupBaseName);
	return bLoaded;
}

bool FIISAgentAccessService::GetSourceReferences(
	const FString& ChunkId,
	FIISAgentToolResponse& OutResponse)
{
	FIISAgentToolRequest Request;
	Request.ToolKind = EIISAgentToolKind::GetSourceReferences;
	Request.ChunkId = ChunkId;
	InitializeResponse(Request, EIISAgentToolKind::GetSourceReferences, OutResponse);

	FIISIndexChunk Chunk;
	TArray<FString> Warnings;
	const bool bLoaded = FIISChunkCatalog::LoadChunkById(ChunkId, Chunk, Warnings);
	OutResponse.Warnings.Append(Warnings);
	if (bLoaded)
	{
		OutResponse.Status = Chunk.SourceReferences.Num() > 0
			? EIISAgentToolStatus::Ready
			: EIISAgentToolStatus::Empty;
		OutResponse.SourceReferences = Chunk.SourceReferences;
	}
	else
	{
		OutResponse.Status = EIISAgentToolStatus::Empty;
	}

	WriteAgentResponse(OutResponse, AgentSourceReferencesBaseName);
	return bLoaded;
}

bool FIISAgentAccessService::QueryUsages(const FString& Query, FIISUsageQueryResult& OutResult)
{
	return FIISUsageGraphImporter::QueryUsages(Query, OutResult);
}

bool FIISAgentAccessService::FindUsages(
	const FIISAgentToolRequest& Request,
	FIISAgentToolResponse& OutResponse)
{
	FIISAgentToolRequest EffectiveRequest = Request;
	EffectiveRequest.ToolKind = EIISAgentToolKind::FindUsages;
	EffectiveRequest.QueryText = !Request.SymbolName.TrimStartAndEnd().IsEmpty()
		? Request.SymbolName
		: Request.QueryText;
	EffectiveRequest.SearchMode = EIISSearchMode::Lexical;
	EffectiveRequest.PreferredGroups.AddUnique(TEXT("by_chunk_kind:Code"));
	EffectiveRequest.PreferredGroups.AddUnique(TEXT("by_chunk_kind:Module"));
	EffectiveRequest.PreferredGroups.AddUnique(TEXT("by_chunk_kind:Reflection"));
	InitializeResponse(EffectiveRequest, EIISAgentToolKind::FindUsages, OutResponse);

	if (EffectiveRequest.QueryText.TrimStartAndEnd().IsEmpty())
	{
		OutResponse.Status = EIISAgentToolStatus::Error;
		OutResponse.Errors.Add(TEXT("Find usages requires symbol_name or query_text."));
		WriteAgentResponse(OutResponse, AgentUsageReportBaseName);
		return false;
	}

	FIISUsageQueryResult GraphResult;
	const bool bGraphQueryOk = QueryUsages(EffectiveRequest.QueryText, GraphResult);
	OutResponse.UsageGraph = GraphResult;
	OutResponse.Warnings.Append(GraphResult.Warnings);

	if (bGraphQueryOk && GraphResult.bGraphEvidenceAvailable)
	{
		OutResponse.Status = (GraphResult.Declarations.Num() + GraphResult.References.Num() + GraphResult.Calls.Num()) > 0
			? EIISAgentToolStatus::Ready
			: EIISAgentToolStatus::Empty;
		WriteAgentResponse(OutResponse, AgentUsageReportBaseName);
		return true;
	}

	OutResponse.Warnings.Add(TEXT("This is lexical evidence search, not a complete usage graph."));
	if (!GraphResult.bGraphEvidenceAvailable)
	{
		OutResponse.Warnings.Add(TEXT("Graph evidence incomplete or not imported."));
	}

	const FIISSearchQuery Query = MakeSearchQueryFromAgentRequest(EffectiveRequest);
	FIISLocalIndexService Service;
	const bool bSearchOk = Service.Search(Query, OutResponse.SearchResponse);
	OutResponse.Status = AgentStatusFromSearchStatus(OutResponse.SearchResponse.Status);
	OutResponse.Warnings.Append(OutResponse.SearchResponse.Warnings);
	OutResponse.Errors.Append(OutResponse.SearchResponse.Errors);
	AppendSearchEvidenceToAgentResponse(OutResponse);

	WriteAgentResponse(OutResponse, AgentUsageReportBaseName);
	return bSearchOk && OutResponse.Status != EIISAgentToolStatus::Error;
}

bool FIISAgentAccessService::ExplainBlueprint(
	const FIISAgentToolRequest& Request,
	FIISAgentToolResponse& OutResponse)
{
	FIISAgentToolRequest EffectiveRequest = Request;
	EffectiveRequest.ToolKind = EIISAgentToolKind::ExplainBlueprint;
	EffectiveRequest.QueryText = !Request.AssetPath.TrimStartAndEnd().IsEmpty()
		? Request.AssetPath
		: Request.QueryText;
	EffectiveRequest.SearchMode = Request.SearchMode == EIISSearchMode::Unknown
		? EIISSearchMode::Hybrid
		: Request.SearchMode;
	EffectiveRequest.PreferredGroups.AddUnique(TEXT("by_chunk_kind:Blueprint"));
	EffectiveRequest.PreferredGroups.AddUnique(TEXT("by_chunk_kind:BlueprintGraph"));
	InitializeResponse(EffectiveRequest, EIISAgentToolKind::ExplainBlueprint, OutResponse);

	if (EffectiveRequest.QueryText.TrimStartAndEnd().IsEmpty())
	{
		OutResponse.Status = EIISAgentToolStatus::Error;
		OutResponse.Errors.Add(TEXT("Explain Blueprint requires asset_path or query_text."));
		WriteAgentResponse(OutResponse, AgentBlueprintExplanationBaseName);
		return false;
	}

	FString IRJson;
	FString IRSourcePath;
	TArray<FString> IRWarnings;
	if (FIISBlueprintExplanationService::TryLoadBlueprintIRJson(EffectiveRequest.QueryText, IRJson, IRSourcePath, IRWarnings))
	{
		FIISBlueprintExplanation Explanation;
		if (FIISBlueprintExplanationService::AssembleFromIRJson(IRJson, IRSourcePath, Explanation))
		{
			for (const FIISAgentGuardrail& Guardrail : OutResponse.Guardrails)
			{
				const FString GuardrailLabel = !Guardrail.GuardrailId.IsEmpty() ? Guardrail.GuardrailId : Guardrail.Text;
				Explanation.Guardrails.AddUnique(GuardrailLabel);
			}
			Explanation.Warnings.Append(IRWarnings);
			OutResponse.BlueprintExplanation = Explanation;
			AppendUniqueSourceReferences(Explanation.SourceReferences, OutResponse.SourceReferences);

			const int32 PopulatedFields =
				Explanation.Graphs.Num()
				+ Explanation.EventsAndFunctions.Num()
				+ Explanation.Variables.Num()
				+ Explanation.Components.Num();
			OutResponse.Status = PopulatedFields > 0 ? EIISAgentToolStatus::Ready : EIISAgentToolStatus::Empty;
			WriteAgentResponse(OutResponse, AgentBlueprintExplanationBaseName);
			return true;
		}
		OutResponse.Warnings.Append(IRWarnings);
	}

	OutResponse.Warnings.Add(TEXT("This is retrieved Blueprint evidence, not a generated explanation."));
	OutResponse.Warnings.Add(TEXT("Blueprint IR not available; using retrieved evidence."));

	const FIISSearchQuery Query = MakeSearchQueryFromAgentRequest(EffectiveRequest);
	FIISLocalIndexService Service;
	const bool bSearchOk = Service.Search(Query, OutResponse.SearchResponse);
	OutResponse.Status = AgentStatusFromSearchStatus(OutResponse.SearchResponse.Status);
	OutResponse.Warnings.Append(OutResponse.SearchResponse.Warnings);
	OutResponse.Errors.Append(OutResponse.SearchResponse.Errors);
	AppendSearchEvidenceToAgentResponse(OutResponse);

	for (const FIISIndexChunk& Chunk : OutResponse.Chunks)
	{
		if (Chunk.Title.Contains(TEXT("unsupported"), ESearchCase::IgnoreCase)
			|| Chunk.Text.Contains(TEXT("unsupported"), ESearchCase::IgnoreCase))
		{
			OutResponse.Warnings.Add(TEXT("Unsupported-node evidence is present in one or more returned chunks."));
			break;
		}
	}

	WriteAgentResponse(OutResponse, AgentBlueprintExplanationBaseName);
	return bSearchOk && OutResponse.Status != EIISAgentToolStatus::Error;
}

bool FIISAgentAccessService::WriteAgentToolContracts(FString& OutContractsPath)
{
	OutContractsPath.Reset();
	FIISStoragePaths::EnsureDefaultFolders();
	IFileManager::Get().MakeDirectory(*FIISStoragePaths::GetAgentContractsDir(), true);

	TArray<TSharedPtr<FJsonValue>> ToolSchemas;
	TArray<TSharedPtr<FJsonValue>> RegistryTools;
	TArray<TSharedPtr<FJsonValue>> ManifestTools;

	for (const FIISAgentToolDefinition& Definition : MakeAgentToolDefinitions())
	{
		const TSharedRef<FJsonObject> Schema = MakeToolSchemaFromDefinition(Definition);
		ToolSchemas.Add(MakeShared<FJsonValueObject>(Schema));
		RegistryTools.Add(MakeShared<FJsonValueObject>(MakeRegistryToolObject(Definition)));
		ManifestTools.Add(MakeShared<FJsonValueObject>(MakeRegistryToolObject(Definition)));

		if (Definition.ToolName == TEXT("iis_search"))
		{
			WriteSingleSchema(TEXT("iis_search.schema.json"), Schema);
		}
		else if (Definition.ToolName == TEXT("iis_get_context_pack"))
		{
			WriteSingleSchema(TEXT("iis_get_context_pack.schema.json"), Schema);
		}
		else if (Definition.ToolName == TEXT("iis_get_chunk"))
		{
			WriteSingleSchema(TEXT("iis_get_chunk.schema.json"), Schema);
		}
		else if (Definition.ToolName == TEXT("iis_get_source_references"))
		{
			WriteSingleSchema(TEXT("iis_get_source_references.schema.json"), Schema);
		}
		else if (Definition.ToolName == TEXT("iis_find_usages"))
		{
			WriteSingleSchema(TEXT("iis_find_usages.schema.json"), Schema);
		}
		else if (Definition.ToolName == TEXT("iis_explain_blueprint"))
		{
			WriteSingleSchema(TEXT("iis_explain_blueprint.schema.json"), Schema);
		}
	}

	TSharedRef<FJsonObject> Contracts = MakeShared<FJsonObject>();
	Contracts->SetStringField(TEXT("schema_version"), TEXT("0.1.0"));
	Contracts->SetStringField(TEXT("tool_name"), TEXT("Internal Index Service"));
	Contracts->SetStringField(TEXT("generated_at_utc"), FDateTime::UtcNow().ToIso8601());
	Contracts->SetArrayField(TEXT("tools"), ToolSchemas);
	Contracts->SetArrayField(TEXT("forbidden_actions"), MakeForbiddenActionsArray());
	Contracts->SetArrayField(TEXT("guardrails"), MakeStringArray(MakeDefaultGuardrailStrings()));

	OutContractsPath = FIISStoragePaths::GetAgentContractsDir() / AgentContractsFileName;
	const bool bContractsSaved = SaveJsonObjectToFile(Contracts, OutContractsPath);

	TSharedRef<FJsonObject> Registry = MakeShared<FJsonObject>();
	Registry->SetStringField(TEXT("schema_version"), TEXT("0.1.0"));
	Registry->SetStringField(TEXT("tool_name"), TEXT("Internal Index Service"));
	Registry->SetStringField(TEXT("generated_at_utc"), FDateTime::UtcNow().ToIso8601());
	Registry->SetStringField(TEXT("registry_kind"), TEXT("local_agent_tool_registry"));
	Registry->SetArrayField(TEXT("tools"), RegistryTools);
	Registry->SetArrayField(TEXT("forbidden_actions"), MakeForbiddenActionsArray());
	Registry->SetArrayField(TEXT("guardrails"), MakeStringArray(MakeDefaultGuardrailStrings()));
	const FString RegistryPath = FIISStoragePaths::GetAgentContractsDir() / AgentRegistryFileName;
	const bool bRegistrySaved = SaveJsonObjectToFile(Registry, RegistryPath);

	TSharedRef<FJsonObject> MpcManifest = MakeShared<FJsonObject>();
	MpcManifest->SetStringField(TEXT("schema_version"), TEXT("0.1.0"));
	MpcManifest->SetStringField(TEXT("tool_name"), TEXT("Internal Index Service"));
	MpcManifest->SetStringField(TEXT("generated_at_utc"), FDateTime::UtcNow().ToIso8601());
	MpcManifest->SetStringField(TEXT("manifest_kind"), TEXT("mcp_tool_manifest"));
	MpcManifest->SetStringField(TEXT("mcp_runtime_status"), TEXT("live"));
	MpcManifest->SetStringField(TEXT("local_invocation"), TEXT("In-engine: GEngine->GetEngineSubsystem<UIISSubsystem>(). External: loopback MCP endpoint (opt-in) plus Tools/mcp_proxy stdio proxy. JSON file bridge: UIISPythonBridge.ExecuteAgentToolFromJson."));
	MpcManifest->SetArrayField(TEXT("tools"), ManifestTools);
	MpcManifest->SetArrayField(TEXT("forbidden_actions"), MakeForbiddenActionsArray());
	MpcManifest->SetArrayField(TEXT("guardrails"), MakeStringArray(MakeDefaultGuardrailStrings()));
	const FString MpcManifestPath = FIISStoragePaths::GetAgentContractsDir() / AgentMcpManifestFileName;
	const bool bMpcManifestSaved = SaveJsonObjectToFile(MpcManifest, MpcManifestPath);

	TArray<FString> Lines;
	Lines.Add(TEXT("# IIS Agent Access Report"));
	Lines.Add(TEXT(""));
	Lines.Add(FString::Printf(TEXT("- GeneratedAtUtc: %s"), *FDateTime::UtcNow().ToIso8601()));
	Lines.Add(FString::Printf(TEXT("- Contracts: %s"), *OutContractsPath));
	Lines.Add(FString::Printf(TEXT("- Registry: %s"), *RegistryPath));
	Lines.Add(FString::Printf(TEXT("- MCPManifest: %s"), *MpcManifestPath));
	Lines.Add(TEXT("- MCPServerRuntime: live (loopback endpoint opt-in via bEnableMcpEndpoint)"));
	Lines.Add(TEXT("- AIAnswerGeneration: not implemented"));
	Lines.Add(TEXT("- LocalJsonInvocation: implemented"));
	Lines.Add(TEXT(""));
	Lines.Add(TEXT("## Tools"));
	for (const FIISAgentToolDefinition& Definition : MakeAgentToolDefinitions())
	{
		Lines.Add(FString::Printf(TEXT("- %s -> %s"), *Definition.ToolName, *Definition.ExecutionHandler));
	}
	Lines.Add(TEXT(""));
	Lines.Add(TEXT("## Guardrails"));
	Lines.Add(TEXT("- This response contains retrieved evidence only."));
	Lines.Add(TEXT("- It does not authorize migration, placement, patching, copying, or project mutation."));
	Lines.Add(TEXT("- It may reference source evidence, destination evidence, or private review notes depending on filters."));
	Lines.Add(TEXT("- All claims must be traced back to source references."));

	const bool bMarkdownSaved = FFileHelper::SaveStringArrayToFile(
		Lines,
		*(FIISStoragePaths::GetLogsDir() / AgentAccessReportFileName));

	return bContractsSaved && bRegistrySaved && bMpcManifestSaved && bMarkdownSaved;
}

FString FIISAgentAccessService::GetLatestResponsePath(const EIISAgentToolKind ToolKind)
{
	return MakeAgentJsonPath(GetBaseFileNameForTool(ToolKind));
}

FString FIISAgentAccessService::GetAgentContractsPath()
{
	return FIISStoragePaths::GetAgentContractsDir() / AgentContractsFileName;
}

void FIISAgentAccessService::AddDefaultGuardrails(FIISAgentToolResponse& Response)
{
	AddDefaultGuardrailsToResponse(Response);
}

bool FIISAgentAccessService::WriteAgentResponse(
	const FIISAgentToolResponse& Response,
	const FString& BaseFileName)
{
	FIISStoragePaths::EnsureDefaultFolders();
	IFileManager::Get().MakeDirectory(*FIISStoragePaths::GetAgentDir(), true);
	IFileManager::Get().MakeDirectory(*FIISStoragePaths::GetAgentRunsDir(), true);

	const FString RunDir = MakeAgentRunDir(Response);
	IFileManager::Get().MakeDirectory(*RunDir, true);

	const bool bLatestJsonSaved = WriteAgentResponseJson(Response, MakeAgentJsonPath(BaseFileName));
	const bool bLatestMarkdownSaved = WriteAgentResponseMarkdown(Response, MakeAgentMarkdownPath(BaseFileName));
	const bool bRunRequestSaved = SaveJsonObjectToFile(MakeAgentRequestObject(Response.Request), RunDir / TEXT("request.json"));
	const bool bRunResponseSaved = WriteAgentResponseJson(Response, RunDir / TEXT("response.json"));
	const bool bRunMarkdownSaved = WriteAgentResponseMarkdown(Response, RunDir / TEXT("response.md"));
	return bLatestJsonSaved && bLatestMarkdownSaved && bRunRequestSaved && bRunResponseSaved && bRunMarkdownSaved;
}

bool FIISAgentAccessService::WriteAgentResponseJson(
	const FIISAgentToolResponse& Response,
	const FString& JsonPath)
{
	return SaveJsonObjectToFile(MakeAgentResponseObject(Response), JsonPath);
}

bool FIISAgentAccessService::WriteAgentResponseMarkdown(
	const FIISAgentToolResponse& Response,
	const FString& MarkdownPath)
{
	TArray<FString> Lines;
	Lines.Add(TEXT("# IIS Agent Tool Response"));
	Lines.Add(TEXT(""));
	Lines.Add(FString::Printf(TEXT("- Tool: %s"), *AgentToolKindToString(Response.ToolKind)));
	Lines.Add(FString::Printf(TEXT("- Status: %s"), *AgentToolStatusToString(Response.Status)));
	Lines.Add(FString::Printf(TEXT("- GeneratedAtUtc: %s"), *Response.GeneratedAtUtc));
	Lines.Add(FString::Printf(TEXT("- RunId: %s"), *Response.RunId));
	Lines.Add(FString::Printf(TEXT("- Query: %s"), *Response.Request.QueryText));
	Lines.Add(FString::Printf(TEXT("- ChunkId: %s"), *Response.Request.ChunkId));
	Lines.Add(FString::Printf(TEXT("- AssetPath: %s"), *Response.Request.AssetPath));
	Lines.Add(FString::Printf(TEXT("- SymbolName: %s"), *Response.Request.SymbolName));
	Lines.Add(FString::Printf(TEXT("- SearchMode: %s"), *SearchModeToString(Response.Request.SearchMode)));
	Lines.Add(FString::Printf(TEXT("- Results: %d"), Response.SearchResponse.Results.Num()));
	Lines.Add(FString::Printf(TEXT("- Chunks: %d"), Response.Chunks.Num()));
	Lines.Add(FString::Printf(TEXT("- SourceReferences: %d"), Response.SourceReferences.Num()));
	Lines.Add(FString::Printf(TEXT("- AllowsMigrationDecision: %s"), *BoolToString(Response.bAllowsMigrationDecision)));
	Lines.Add(FString::Printf(TEXT("- AllowsPatchGeneration: %s"), *BoolToString(Response.bAllowsPatchGeneration)));
	Lines.Add(FString::Printf(TEXT("- AllowsProjectMutation: %s"), *BoolToString(Response.bAllowsProjectMutation)));

	Lines.Add(TEXT(""));
	Lines.Add(TEXT("## Guardrails"));
	for (const FIISAgentGuardrail& Guardrail : Response.Guardrails)
	{
		Lines.Add(FString::Printf(TEXT("- [%s] %s: %s"), *Guardrail.Severity, *Guardrail.GuardrailId, *Guardrail.Text));
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
	for (const FIISSearchResult& Result : Response.SearchResponse.Results)
	{
		Lines.Add(TEXT(""));
		Lines.Add(FString::Printf(TEXT("### %s"), Result.Title.IsEmpty() ? *Result.ChunkId : *Result.Title));
		Lines.Add(FString::Printf(TEXT("- Chunk: %s"), *Result.ChunkId));
		Lines.Add(FString::Printf(TEXT("- Score: %.4f"), Result.Score));
		Lines.Add(FString::Printf(TEXT("- Explanation: %s"), *Result.ScoreExplanation));
		Lines.Add(FString::Printf(TEXT("- Source References: %d"), Result.Chunk.SourceReferences.Num()));
		Lines.Add(FString::Printf(TEXT("- Snippet: %s"), *Result.Snippet));
	}

	if (Response.Chunks.Num() > 0 && Response.SearchResponse.Results.Num() == 0)
	{
		Lines.Add(TEXT(""));
		Lines.Add(TEXT("## Chunks"));
		for (const FIISIndexChunk& Chunk : Response.Chunks)
		{
			Lines.Add(TEXT(""));
			Lines.Add(FString::Printf(TEXT("### %s"), Chunk.Title.IsEmpty() ? *Chunk.ChunkId : *Chunk.Title));
			Lines.Add(FString::Printf(TEXT("- Chunk: %s"), *Chunk.ChunkId));
			Lines.Add(FString::Printf(TEXT("- Kind: %s"), *ChunkKindToString(Chunk.ChunkKind)));
			Lines.Add(FString::Printf(TEXT("- Sensitivity: %s"), *ChunkSensitivityToString(Chunk.Sensitivity)));
			Lines.Add(FString::Printf(TEXT("- Source References: %d"), Chunk.SourceReferences.Num()));
			Lines.Add(FString::Printf(TEXT("- Text: %s"), *Chunk.Text.Left(600)));
		}
	}

	Lines.Add(TEXT(""));
	Lines.Add(TEXT("## Source References"));
	for (const FIISSourceReference& Reference : Response.SourceReferences)
	{
		Lines.Add(FString::Printf(
			TEXT("- %s | %s | %s | %s"),
			*Reference.ArtifactKind,
			*Reference.RelativePath,
			*Reference.JsonPointer,
			*Reference.Fingerprint));
	}

	if (Response.ContextPack.Items.Num() > 0)
	{
		Lines.Add(TEXT(""));
		Lines.Add(TEXT("## Context Pack"));
		Lines.Add(FString::Printf(TEXT("- ContextPackId: %s"), *Response.ContextPack.ContextPackId));
		Lines.Add(FString::Printf(TEXT("- Items: %d"), Response.ContextPack.Items.Num()));
		for (const FIISContextPackItem& Item : Response.ContextPack.Items)
		{
			Lines.Add(FString::Printf(TEXT("- %s: %s"), *Item.ChunkId, *Item.Title));
		}
	}

	return FFileHelper::SaveStringArrayToFile(Lines, *MarkdownPath);
}
