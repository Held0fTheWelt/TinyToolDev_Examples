/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#include "IISUmcpToolProvider.h"

#include "IISAgentAccessService.h"
#include "IISAgentAccessTypes.h"
#include "IISServiceInterface.h"
#include "InternalIndexServiceModule.h"
#include "IISStoragePaths.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
	EIISAgentToolKind ToolKindFromLocalName(const FString& Name)
	{
		FString Local = Name;
		Local.RemoveFromStart(TEXT("iis_"));
		if (Local == TEXT("search")) return EIISAgentToolKind::Search;
		if (Local == TEXT("get_context_pack")) return EIISAgentToolKind::GetContextPack;
		if (Local == TEXT("get_chunk")) return EIISAgentToolKind::GetChunk;
		if (Local == TEXT("get_source_references")) return EIISAgentToolKind::GetSourceReferences;
		if (Local == TEXT("find_usages")) return EIISAgentToolKind::FindUsages;
		if (Local == TEXT("explain_blueprint")) return EIISAgentToolKind::ExplainBlueprint;
		return EIISAgentToolKind::Unknown;
	}
}

bool FIISUmcpToolProvider::IsReady() const
{
	return FInternalIndexServiceModule::IsAvailable();
}

void FIISUmcpToolProvider::GetToolDescriptors(TArray<FUmcpToolDescriptor>& OutTools) const
{
	OutTools.Reset();

	FString ContractsPath;
	FIISAgentAccessService::WriteAgentToolContracts(ContractsPath);
	const FString ManifestPath = FIISStoragePaths::GetAgentContractsDir() / TEXT("mcp_tool_manifest.json");

	FString ManifestText;
	if (!FFileHelper::LoadFileToString(ManifestText, *ManifestPath))
	{
		return;
	}

	TSharedPtr<FJsonObject> Manifest;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ManifestText);
	if (!FJsonSerializer::Deserialize(Reader, Manifest) || !Manifest.IsValid())
	{
		return;
	}

	const TArray<TSharedPtr<FJsonValue>>* ToolsArray = nullptr;
	if (!Manifest->TryGetArrayField(TEXT("tools"), ToolsArray) || !ToolsArray)
	{
		return;
	}

	for (const TSharedPtr<FJsonValue>& ToolValue : *ToolsArray)
	{
		const TSharedPtr<FJsonObject> ToolObj = ToolValue->AsObject();
		if (!ToolObj.IsValid())
		{
			continue;
		}

		FString Name;
		ToolObj->TryGetStringField(TEXT("name"), Name);
		Name.RemoveFromStart(TEXT("iis_"));

		FString Description;
		ToolObj->TryGetStringField(TEXT("description"), Description);

		FString SchemaJson;
		const TSharedPtr<FJsonObject>* SchemaObj = nullptr;
		if (ToolObj->TryGetObjectField(TEXT("input_schema"), SchemaObj) && SchemaObj && (*SchemaObj).IsValid())
		{
			const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&SchemaJson);
			FJsonSerializer::Serialize((*SchemaObj).ToSharedRef(), Writer);
		}

		FUmcpToolDescriptor Descriptor;
		Descriptor.Name = Name;
		Descriptor.Description = Description;
		Descriptor.InputSchemaJson = SchemaJson;
		OutTools.Add(Descriptor);
	}
}

void FIISUmcpToolProvider::InvokeTool(const FUmcpToolInvocation& Invocation, FUmcpToolResult& OutResult)
{
	const EIISAgentToolKind Kind = ToolKindFromLocalName(Invocation.ToolName);
	if (Kind == EIISAgentToolKind::Unknown)
	{
		OutResult.Status = EUmcpToolStatus::Error;
		OutResult.ErrorCode = TEXT("unknown_tool");
		OutResult.ErrorMessage = FString::Printf(TEXT("Unknown IIS tool '%s'"), *Invocation.ToolName);
		return;
	}

	if (!FInternalIndexServiceModule::IsAvailable())
	{
		OutResult.Status = EUmcpToolStatus::Error;
		OutResult.ErrorCode = TEXT("service_unavailable");
		OutResult.ErrorMessage = TEXT("Internal Index Service is not available.");
		return;
	}

	FIISAgentToolRequest Request;
	Request.ToolKind = Kind;

	TSharedPtr<FJsonObject> Args;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Invocation.ArgumentsJson);
	if (FJsonSerializer::Deserialize(Reader, Args) && Args.IsValid())
	{
		Args->TryGetStringField(TEXT("query_text"), Request.QueryText);
		Args->TryGetStringField(TEXT("chunk_id"), Request.ChunkId);
		Args->TryGetStringField(TEXT("asset_path"), Request.AssetPath);
		Args->TryGetStringField(TEXT("symbol_name"), Request.SymbolName);
		int32 MaxResults = Request.MaxResults;
		if (Args->TryGetNumberField(TEXT("max_results"), MaxResults))
		{
			Request.MaxResults = MaxResults;
		}
	}

	IInternalIndexService& Service = FInternalIndexServiceModule::Get().GetService();
	FIISAgentToolResponse Response;
	const bool bExecuted = Service.ExecuteAgentTool(Request, Response);

	FString ResponseJson;
	if (FIISAgentAccessService::SerializeAgentToolResponseToJson(Response, ResponseJson) && !ResponseJson.IsEmpty())
	{
		OutResult.ResultJson = ResponseJson;
	}
	else
	{
		OutResult.ResultJson = FString::Printf(TEXT("{\"tool\":\"%s\",\"executed\":%s,\"status\":%d}"),
			*Invocation.ToolName,
			bExecuted ? TEXT("true") : TEXT("false"),
			(int32)Response.Status);
	}

	OutResult.Status = (Response.Status == EIISAgentToolStatus::Error)
		? EUmcpToolStatus::Error
		: EUmcpToolStatus::Ok;
	if (OutResult.Status == EUmcpToolStatus::Error)
	{
		OutResult.ErrorCode = TEXT("agent_error");
	}
}
