/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "IISSettings.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISSettingsDefaults,
	"InternalIndexService.Settings.Defaults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISSettingsDefaults::RunTest(const FString& Parameters)
{
	const UIISSettings* Settings = GetDefault<UIISSettings>();
	TestNotNull(TEXT("settings CDO resolves"), Settings);
	if (!Settings)
	{
		return false;
	}

	TestEqual(TEXT("default vector backend"), Settings->VectorBackend, FString(TEXT("jsonl_bruteforce")));
	TestFalse(TEXT("MCP disabled by default"), Settings->bEnableMcpEndpoint);
	TestEqual(TEXT("default MCP port"), Settings->McpPort, 8731);
	TestTrue(TEXT("index root empty by default"), Settings->IndexRoot.IsEmpty());
	return true;
}

#endif
