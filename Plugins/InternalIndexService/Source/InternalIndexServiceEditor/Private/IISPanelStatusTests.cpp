/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "IISPanelStatus.h"
#include "IISEmbeddingTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISPanelStatusMirrorsRegistry,
	"InternalIndexService.Editor.PanelStatusMirrorsRegistry",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISPanelStatusMirrorsRegistry::RunTest(const FString& Parameters)
{
	const FIISPanelStatusSnapshot Snapshot = IISCapturePanelStatus();

	TestEqual(
		TEXT("integration count matches registry"),
		Snapshot.Integrations.Num(),
		FIISEmbeddingRouteExecutorRegistry::GetExecutorIds().Num());

	for (const FIISIntegrationStatus& Integration : Snapshot.Integrations)
	{
		TestTrue(
			FString::Printf(TEXT("executor %s registered"), *Integration.ExecutorId),
			Integration.bRegistered);
	}
	return true;
}

#endif
