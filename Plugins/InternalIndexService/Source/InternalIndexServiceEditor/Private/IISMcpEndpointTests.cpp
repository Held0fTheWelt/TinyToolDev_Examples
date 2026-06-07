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

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Dom/JsonObject.h"
#include "IISAgentAccessService.h"
#include "IISStoragePaths.h"
#include "IISSettings.h"
#include "IISMcpEndpoint.h"
#include "IISMcpServerManager.h"
#include "InternalIndexServiceEditorModule.h"
#include "Modules/ModuleManager.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISMcpEndpointAuthTest,
	"InternalIndexService.McpEndpoint.RejectsBadToken",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISMcpEndpointAuthTest::RunTest(const FString& Parameters)
{
	FIISMcpEndpoint Endpoint;
	Endpoint.InitializeSessionToken();

	int32 Status = 0;
	FString Body;
	Endpoint.HandleToolsList(TEXT("wrong-token"), Status, Body);
	TestEqual(TEXT("Bad token to tools/list is 401"), Status, 401);

	Status = 0;
	Body.Reset();
	Endpoint.HandleToolsCall(TEXT("{\"tool\":\"iis_search\"}"), TEXT("wrong-token"), Status, Body);
	TestEqual(TEXT("Bad token to tools/call is 401"), Status, 401);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISMcpEndpointHealthTest,
	"InternalIndexService.McpEndpoint.HealthOk",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISMcpEndpointHealthTest::RunTest(const FString& Parameters)
{
	FIISMcpEndpoint Endpoint;
	int32 Status = 0;
	FString Body;
	Endpoint.HandleHealth(Status, Body);
	TestEqual(TEXT("Health returns 200"), Status, 200);
	TestTrue(TEXT("Health body has status field"), Body.Contains(TEXT("\"status\"")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISMcpEndpointUnknownToolTest,
	"InternalIndexService.McpEndpoint.UnknownTool",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISMcpEndpointUnknownToolTest::RunTest(const FString& Parameters)
{
	FIISMcpEndpoint Endpoint;
	Endpoint.InitializeSessionToken();
	const FString Token = Endpoint.GetSessionToken();

	int32 Status = 0;
	FString Body;
	Endpoint.HandleToolsCall(TEXT("{\"tool\":\"not_a_tool\"}"), Token, Status, Body);
	TestEqual(TEXT("Unknown tool is 400"), Status, 400);
	TestTrue(TEXT("Body names unknown_tool"), Body.Contains(TEXT("unknown_tool")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISMcpTokenRotationInvalidatesOld,
	"InternalIndexService.Mcp.TokenRotation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISMcpTokenRotationInvalidatesOld::RunTest(const FString& Parameters)
{
	FIISStoragePaths::EnsureDefaultFolders();
	FString ContractsPath;
	FIISAgentAccessService::WriteAgentToolContracts(ContractsPath);

	FIISMcpEndpoint Endpoint;
	Endpoint.InitializeSessionToken();
	const FString Old = Endpoint.GetSessionToken();

	int32 Status = 0;
	FString Body;
	Endpoint.HandleToolsList(Old, Status, Body);
	TestTrue(TEXT("old token authorized before rotation"), Status == 200 || Status == 503);

	Endpoint.RotateToken();
	const FString New = Endpoint.GetSessionToken();
	TestNotEqual(TEXT("token changed"), Old, New);

	Endpoint.HandleToolsList(Old, Status, Body);
	TestEqual(TEXT("old token rejected after rotation"), Status, 401);
	Endpoint.HandleToolsList(New, Status, Body);
	TestTrue(TEXT("new token accepted after rotation"), Status == 200 || Status == 503);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISMcpRequestLogWritesEntry,
	"InternalIndexService.Mcp.RequestLog",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISMcpRequestLogWritesEntry::RunTest(const FString& Parameters)
{
	FIISStoragePaths::EnsureDefaultFolders();
	const FString RequestLogPath = FIISStoragePaths::GetLogsDir() / TEXT("mcp_requests.jsonl");
	IFileManager::Get().Delete(*RequestLogPath);

	FIISMcpEndpoint::AppendRequestLog(TEXT("iis_search"), 200, 12.3, TEXT("/path/to/response.json"), FString());

	if (!TestTrue(TEXT("log file exists"), FPaths::FileExists(RequestLogPath)))
	{
		return false;
	}

	FString LogText;
	if (!TestTrue(TEXT("log readable"), FFileHelper::LoadFileToString(LogText, *RequestLogPath)))
	{
		return false;
	}

	TArray<FString> Lines;
	LogText.ParseIntoArrayLines(Lines, true);
	if (!TestTrue(TEXT("at least one line"), Lines.Num() > 0))
	{
		return false;
	}

	TSharedPtr<FJsonObject> Entry;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Lines.Last());
	if (!TestTrue(TEXT("last line is json"), FJsonSerializer::Deserialize(Reader, Entry) && Entry.IsValid()))
	{
		return false;
	}

	FString Tool;
	double LatencyMs = 0.0;
	int32 Status = 0;
	if (!TestTrue(TEXT("tool field"), Entry->TryGetStringField(TEXT("tool"), Tool)))
	{
		return false;
	}
	TestEqual(TEXT("tool value"), Tool, FString(TEXT("iis_search")));
	if (!TestTrue(TEXT("status field"), Entry->TryGetNumberField(TEXT("status"), Status)))
	{
		return false;
	}
	TestEqual(TEXT("status value"), Status, 200);
	if (!TestTrue(TEXT("latency field"), Entry->TryGetNumberField(TEXT("latency_ms"), LatencyMs)))
	{
		return false;
	}
	TestTrue(TEXT("latency near expected"), FMath::IsNearlyEqual(LatencyMs, 12.3, 0.1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISMcpManagerLifecycle,
	"InternalIndexService.Mcp.ManagerLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISMcpManagerLifecycle::RunTest(const FString& Parameters)
{
	FIISMcpServerManager Manager;
	TestFalse(TEXT("initially stopped"), Manager.IsRunning());
	Manager.Start();
	TestTrue(TEXT("running after start"), Manager.IsRunning());
	Manager.Restart();
	TestTrue(TEXT("running after restart"), Manager.IsRunning());
	Manager.Stop();
	TestFalse(TEXT("stopped after stop"), Manager.IsRunning());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISMcpEndpointReadsSettings,
	"InternalIndexService.Editor.McpEndpointReadsSettings",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISMcpEndpointReadsSettings::RunTest(const FString& Parameters)
{
	UIISSettings* Settings = GetMutableDefault<UIISSettings>();
	const bool bOrigEnabled = Settings->bEnableMcpEndpoint;
	const int32 OrigPort = Settings->McpPort;

	Settings->bEnableMcpEndpoint = true;
	Settings->McpPort = 9123;
	TestTrue(TEXT("endpoint reports enabled from settings"), FIISMcpEndpoint::IsEnabledByConfig());
	TestEqual(TEXT("endpoint reports configured port from settings"), FIISMcpEndpoint::GetConfiguredPort(), 9123);

	Settings->bEnableMcpEndpoint = false;
	Settings->McpPort = 0;
	TestFalse(TEXT("endpoint reports disabled from settings"), FIISMcpEndpoint::IsEnabledByConfig());
	TestEqual(TEXT("invalid port falls back to default"), FIISMcpEndpoint::GetConfiguredPort(), 8731);

	Settings->bEnableMcpEndpoint = bOrigEnabled;
	Settings->McpPort = OrigPort;
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISEditorModuleStopMcpIsSafe,
	"InternalIndexService.Editor.StopMcpIsSafe",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISEditorModuleStopMcpIsSafe::RunTest(const FString& Parameters)
{
	FInternalIndexServiceEditorModule& Module =
		FModuleManager::LoadModuleChecked<FInternalIndexServiceEditorModule>(TEXT("InternalIndexServiceEditor"));

	Module.StopMcp();
	TestFalse(TEXT("MCP not running after StopMcp"), Module.GetMcpStatus().bRunning);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
