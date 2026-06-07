/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA
 *
 * This file is part of the "Internal Index Service" Unreal Engine plugin.
 * Use of this software is governed by the Fab Standard End User License Agreement
 * (EULA) applicable to this product, available at:
 * https://www.fab.com/eula
 *
 * Except as expressly permitted by the Fab Standard EULA, any reproduction,
 * distribution, modification, or use of this software, in whole or in part,
 * is strictly prohibited.
 *
 * This software is provided on an "AS IS" basis, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied, including but not
 * limited to warranties of merchantability, fitness for a particular purpose,
 * and non-infringement.
 * available at: https://www.fab.com/eula.  */

#include "IISMcpEndpoint.h"

#include "IISAgentAccessService.h"
#include "IISAgentAccessTypes.h"
#include "IISServiceInterface.h"
#include "IISSettings.h"
#include "IISStoragePaths.h"
#include "InternalIndexServiceModule.h"
#include "HttpRequestHandler.h"
#include "Dom/JsonObject.h"
#include "HttpPath.h"
#include "HttpServerModule.h"
#include "HttpServerRequest.h"
#include "HttpServerResponse.h"
#include "IHttpRouter.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

#include <atomic>

namespace
{
	constexpr int32 GIisDefaultMcpPort = 8731;

	std::atomic<int64> GMcpRequestLogCount{0};

	FString ErrorCodeForHttpStatus(int32 Status)
	{
		switch (Status)
		{
		case 200:
			return FString();
		case 400:
			return TEXT("bad_request");
		case 401:
			return TEXT("unauthorized");
		case 500:
			return TEXT("agent_error");
		case 503:
			return TEXT("service_unavailable");
		default:
			return TEXT("http_error");
		}
	}

	FString GetRequestToken(const FHttpServerRequest& Request)
	{
		const TArray<FString>* Values = Request.Headers.Find(TEXT("Authorization"));
		if (Values && Values->Num() > 0)
		{
			FString Header = (*Values)[0];
			Header.RemoveFromStart(TEXT("Bearer "));
			return Header.TrimStartAndEnd();
		}
		return FString();
	}

	FString RequestBodyToString(const FHttpServerRequest& Request)
	{
		if (Request.Body.IsEmpty())
		{
			return FString();
		}
		return FString(Request.Body.Num(), UTF8_TO_TCHAR(reinterpret_cast<const char*>(Request.Body.GetData())));
	}

	TUniquePtr<FHttpServerResponse> MakeJsonResponse(int32 StatusCode, const FString& Body)
	{
		TUniquePtr<FHttpServerResponse> Response = FHttpServerResponse::Create(Body, TEXT("application/json"));
		Response->Code = static_cast<EHttpServerResponseCodes>(StatusCode);
		return Response;
	}

	EIISAgentToolKind ParseToolKind(const FString& Name)
	{
		if (Name == TEXT("iis_search")) return EIISAgentToolKind::Search;
		if (Name == TEXT("iis_get_context_pack")) return EIISAgentToolKind::GetContextPack;
		if (Name == TEXT("iis_get_chunk")) return EIISAgentToolKind::GetChunk;
		if (Name == TEXT("iis_get_source_references")) return EIISAgentToolKind::GetSourceReferences;
		if (Name == TEXT("iis_find_usages")) return EIISAgentToolKind::FindUsages;
		if (Name == TEXT("iis_explain_blueprint")) return EIISAgentToolKind::ExplainBlueprint;
		return EIISAgentToolKind::Unknown;
	}
}

bool FIISMcpEndpoint::IsEnabledByConfig()
{
	const UIISSettings* Settings = GetDefault<UIISSettings>();
	return Settings && Settings->bEnableMcpEndpoint;
}

int32 FIISMcpEndpoint::GetConfiguredPort()
{
	const UIISSettings* Settings = GetDefault<UIISSettings>();
	const int32 Port = Settings ? Settings->McpPort : GIisDefaultMcpPort;
	return Port > 0 ? Port : GIisDefaultMcpPort;
}

FIISMcpEndpoint::FIISMcpEndpoint() = default;

void FIISMcpEndpoint::InitializeSessionToken()
{
	SessionToken = FGuid::NewGuid().ToString(EGuidFormats::Digits);
}

void FIISMcpEndpoint::RotateToken()
{
	SessionToken = FGuid::NewGuid().ToString(EGuidFormats::Digits);
	WriteHandshakeFile(BoundPort > 0 ? BoundPort : GetConfiguredPort());
}

void FIISMcpEndpoint::AppendRequestLog(
	const FString& Tool,
	const int32 Status,
	const double LatencyMs,
	const FString& ResponsePath,
	const FString& ErrorCode)
{
	FIISStoragePaths::EnsureDefaultFolders();
	const FString Dir = FIISStoragePaths::GetLogsDir();
	IFileManager::Get().MakeDirectory(*Dir, true);
	const FString RequestLogPath = Dir / TEXT("mcp_requests.jsonl");
	const FString Line = FString::Printf(
		TEXT("{\"ts\":\"%s\",\"tool\":\"%s\",\"status\":%d,\"latency_ms\":%.1f,\"response_path\":\"%s\",\"error_code\":\"%s\"}\n"),
		*FDateTime::UtcNow().ToIso8601(),
		*Tool.ReplaceCharWithEscapedChar(),
		Status,
		LatencyMs,
		*ResponsePath.ReplaceCharWithEscapedChar(),
		*ErrorCode.ReplaceCharWithEscapedChar());
	FFileHelper::SaveStringToFile(
		Line,
		*RequestLogPath,
		FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM,
		&IFileManager::Get(),
		FILEWRITE_Append);
	GMcpRequestLogCount.fetch_add(1);
}

int64 FIISMcpEndpoint::GetRequestLogCount()
{
	return GMcpRequestLogCount.load();
}

FString FIISMcpEndpoint::GetHandshakeFilePath()
{
	return FIISStoragePaths::GetAgentContractsDir() / TEXT("mcp_endpoint.json");
}

void FIISMcpEndpoint::WriteHandshakeFile(int32 Port) const
{
	const FString Json = FString::Printf(
		TEXT("{\n  \"host\": \"127.0.0.1\",\n  \"port\": %d,\n  \"token\": \"%s\"\n}\n"),
		Port, *SessionToken);
	FFileHelper::SaveStringToFile(Json, *GetHandshakeFilePath());
}

bool FIISMcpEndpoint::IsAuthorized(const FString& AuthToken) const
{
	return !SessionToken.IsEmpty() && AuthToken == SessionToken;
}

void FIISMcpEndpoint::HandleHealth(int32& OutStatus, FString& OutBody) const
{
	const bool bAvailable = FInternalIndexServiceModule::IsAvailable();
	FString Version;
	FString IndexRoot;
	if (bAvailable)
	{
		IInternalIndexService& Service = FInternalIndexServiceModule::Get().GetService();
		Version = Service.GetServiceVersion();
		IndexRoot = Service.GetDefaultIndexRoot();
	}
	OutBody = FString::Printf(
		TEXT("{\"status\":\"%s\",\"version\":\"%s\",\"index_root\":\"%s\",\"token_required\":true}"),
		bAvailable ? TEXT("ready") : TEXT("unavailable"),
		*Version, *IndexRoot.ReplaceCharWithEscapedChar());
	OutStatus = 200;
}

void FIISMcpEndpoint::HandleToolsList(const FString& AuthToken, int32& OutStatus, FString& OutBody) const
{
	if (!IsAuthorized(AuthToken))
	{
		OutStatus = 401;
		OutBody = TEXT("{\"error\":\"unauthorized\"}");
		return;
	}

	FString ContractsPath;
	FIISAgentAccessService::WriteAgentToolContracts(ContractsPath);

	const FString ManifestPath = FIISStoragePaths::GetAgentContractsDir() / TEXT("mcp_tool_manifest.json");
	FString Manifest;
	if (!FFileHelper::LoadFileToString(Manifest, *ManifestPath))
	{
		OutStatus = 503;
		OutBody = TEXT("{\"error\":\"manifest_unavailable\"}");
		return;
	}
	OutStatus = 200;
	OutBody = Manifest;
}

void FIISMcpEndpoint::HandleToolsCall(const FString& Body, const FString& AuthToken, int32& OutStatus, FString& OutBody)
{
	const double StartSeconds = FPlatformTime::Seconds();
	FString ToolName;
	FString ResponsePath;
	FString ErrorCode;

	auto LogAndReturn = [&]()
	{
		AppendRequestLog(ToolName, OutStatus, (FPlatformTime::Seconds() - StartSeconds) * 1000.0, ResponsePath, ErrorCode);
	};

	if (!IsAuthorized(AuthToken))
	{
		OutStatus = 401;
		OutBody = TEXT("{\"error\":\"unauthorized\"}");
		ErrorCode = ErrorCodeForHttpStatus(OutStatus);
		LogAndReturn();
		return;
	}

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		OutStatus = 400;
		OutBody = TEXT("{\"error\":\"invalid_json\"}");
		ErrorCode = ErrorCodeForHttpStatus(OutStatus);
		LogAndReturn();
		return;
	}

	ToolName = Root->GetStringField(TEXT("tool"));
	const EIISAgentToolKind Kind = ParseToolKind(ToolName);
	if (Kind == EIISAgentToolKind::Unknown)
	{
		OutStatus = 400;
		OutBody = FString::Printf(TEXT("{\"error\":\"unknown_tool\",\"tool\":\"%s\"}"), *ToolName);
		ErrorCode = TEXT("unknown_tool");
		LogAndReturn();
		return;
	}

	if (!FInternalIndexServiceModule::IsAvailable())
	{
		OutStatus = 503;
		OutBody = TEXT("{\"error\":\"service_unavailable\"}");
		ErrorCode = ErrorCodeForHttpStatus(OutStatus);
		LogAndReturn();
		return;
	}

	const TSharedPtr<FJsonObject>* Args = nullptr;
	Root->TryGetObjectField(TEXT("arguments"), Args);

	FIISAgentToolRequest Request;
	Request.ToolKind = Kind;
	if (Args && Args->IsValid())
	{
		(*Args)->TryGetStringField(TEXT("query_text"), Request.QueryText);
		(*Args)->TryGetStringField(TEXT("chunk_id"), Request.ChunkId);
		(*Args)->TryGetStringField(TEXT("asset_path"), Request.AssetPath);
		(*Args)->TryGetStringField(TEXT("symbol_name"), Request.SymbolName);
		int32 MaxResults = Request.MaxResults;
		if ((*Args)->TryGetNumberField(TEXT("max_results"), MaxResults))
		{
			Request.MaxResults = MaxResults;
		}
	}

	IInternalIndexService& Service = FInternalIndexServiceModule::Get().GetService();
	FIISAgentToolResponse Response;
	Service.ExecuteAgentTool(Request, Response);

	ResponsePath = FIISAgentAccessService::GetLatestResponsePath(Kind);
	FString ResponseJson;
	if (FFileHelper::LoadFileToString(ResponseJson, *ResponsePath) && !ResponseJson.IsEmpty())
	{
		OutBody = ResponseJson;
	}
	else
	{
		OutBody = FString::Printf(TEXT("{\"tool\":\"%s\",\"status\":\"%d\"}"),
			*ToolName, static_cast<int32>(Response.Status));
	}
	OutStatus = (Response.Status == EIISAgentToolStatus::Error) ? 500 : 200;
	ErrorCode = ErrorCodeForHttpStatus(OutStatus);
	LogAndReturn();
}

bool FIISMcpEndpoint::Start(int32 Port)
{
	if (bIsListening)
	{
		return true;
	}

	FHttpServerModule& HttpServerModule = FHttpServerModule::Get();
	Router = HttpServerModule.GetHttpRouter(static_cast<uint32>(Port));
	if (!Router.IsValid())
	{
		return false;
	}

	if (SessionToken.IsEmpty())
	{
		InitializeSessionToken();
	}

	RouteHandles.Add(Router->BindRoute(
		FHttpPath(TEXT("/mcp/health")),
		EHttpServerRequestVerbs::VERB_GET,
		FHttpRequestHandler::CreateLambda([this](const FHttpServerRequest& Req, const FHttpResultCallback& OnComplete)
		{
			int32 Status = 0;
			FString Body;
			HandleHealth(Status, Body);
			OnComplete(MakeJsonResponse(Status, Body));
			return true;
		})));

	RouteHandles.Add(Router->BindRoute(
		FHttpPath(TEXT("/mcp/tools/list")),
		EHttpServerRequestVerbs::VERB_GET,
		FHttpRequestHandler::CreateLambda([this](const FHttpServerRequest& Req, const FHttpResultCallback& OnComplete)
		{
			int32 Status = 0;
			FString Body;
			HandleToolsList(GetRequestToken(Req), Status, Body);
			OnComplete(MakeJsonResponse(Status, Body));
			return true;
		})));

	RouteHandles.Add(Router->BindRoute(
		FHttpPath(TEXT("/mcp/tools/call")),
		EHttpServerRequestVerbs::VERB_POST,
		FHttpRequestHandler::CreateLambda([this](const FHttpServerRequest& Req, const FHttpResultCallback& OnComplete)
		{
			const FString RequestBody = RequestBodyToString(Req);
			int32 Status = 0;
			FString Body;
			HandleToolsCall(RequestBody, GetRequestToken(Req), Status, Body);
			OnComplete(MakeJsonResponse(Status, Body));
			return true;
		})));

	HttpServerModule.StartAllListeners();
	BoundPort = Port;
	bIsListening = true;
	WriteHandshakeFile(Port);
	return true;
}

void FIISMcpEndpoint::Stop()
{
	if (!bIsListening)
	{
		return;
	}
	if (Router.IsValid())
	{
		for (const FHttpRouteHandle& Handle : RouteHandles)
		{
			if (Handle.IsValid())
			{
				Router->UnbindRoute(Handle);
			}
		}
	}
	RouteHandles.Reset();
	Router.Reset();
	bIsListening = false;
	BoundPort = 0;
}
