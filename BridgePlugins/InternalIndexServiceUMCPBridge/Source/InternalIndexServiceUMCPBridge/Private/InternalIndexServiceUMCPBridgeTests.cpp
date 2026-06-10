/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "IISUmcpToolProvider.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISUmcpProviderDescribesSixTools,
	"InternalIndexServiceUMCPBridge.Provider.DescribesTools",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISUmcpProviderDescribesSixTools::RunTest(const FString& Parameters)
{
	FIISUmcpToolProvider Provider;
	TestEqual(TEXT("provider id"), Provider.GetProviderId(), FString(TEXT("iis")));

	TArray<FUmcpToolDescriptor> Tools;
	Provider.GetToolDescriptors(Tools);
	TestTrue(TEXT("exposes the six IIS tools"), Tools.Num() >= 6);

	const bool bHasSearch = Tools.ContainsByPredicate(
		[](const FUmcpToolDescriptor& D) { return D.Name == TEXT("search"); });
	TestTrue(TEXT("local name is unprefixed 'search'"), bHasSearch);

	const bool bHasPrefixed = Tools.ContainsByPredicate(
		[](const FUmcpToolDescriptor& D) { return D.Name.StartsWith(TEXT("iis_")); });
	TestFalse(TEXT("descriptors carry no provider prefix"), bHasPrefixed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISUmcpProviderUnknownToolErrors,
	"InternalIndexServiceUMCPBridge.Provider.UnknownToolErrors",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISUmcpProviderUnknownToolErrors::RunTest(const FString& Parameters)
{
	FIISUmcpToolProvider Provider;
	FUmcpToolInvocation In;
	In.ToolName = TEXT("not_a_tool");
	In.ArgumentsJson = TEXT("{}");
	FUmcpToolResult Out;
	Provider.InvokeTool(In, Out);
	TestEqual(TEXT("unknown tool is Error"), (int32)Out.Status, (int32)EUmcpToolStatus::Error);
	TestEqual(TEXT("error code is unknown_tool"), Out.ErrorCode, FString(TEXT("unknown_tool")));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
