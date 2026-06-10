/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA
 *
 * This file is part of the "SmartContentDiet PRS Bridge" Unreal Engine plugin.
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
#include "PRSPlacementTypes.h"
#include "SCDHealthForPRS.h"
#include "SCDToPRSPlacementAdapter.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSCDToPRSPlanBuildTest,
	"SmartContentDietBridge.PRS.PlanBuild",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSCDToPRSPlanBuildTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	TArray<FSCDPlacementMove> Moves;
	FSCDPlacementMove Move;
	Move.SourceAssetPath = TEXT("/Game/Old/SM_Tree");
	Move.DestinationFolder = TEXT("/Game/Reviewed/Trees");
	Move.Rationale = TEXT("Consolidate duplicates");
	Moves.Add(Move);

	const FPRSPlacementPlan Plan = FSCDToPRSPlacementAdapter::BuildPlan(TEXT("/Game"), Moves);
	TestEqual(TEXT("One entry per move."), Plan.Entries.Num(), 1);
	TestEqual(TEXT("Target project root preserved."), Plan.TargetProjectRoot, FString(TEXT("/Game")));
	TestEqual(TEXT("Destination preserved."), Plan.Entries[0].Destination, FString(TEXT("/Game/Reviewed/Trees")));
	TestEqual(TEXT("Source preserved."), Plan.Entries[0].Source[0], FString(TEXT("/Game/Old/SM_Tree")));
	TestEqual(TEXT("Rationale preserved."), Plan.Entries[0].Rationale, FString(TEXT("Consolidate duplicates")));
	TestFalse(TEXT("Plan id is set."), Plan.PlanId.IsEmpty());
	TestFalse(TEXT("Entry id is set."), Plan.Entries[0].EntryId.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSCDHealthForPRSTest,
	"SmartContentDietBridge.PRS.HealthQuery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSCDHealthForPRSTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	bool bHasSnapshot = true;
	FSCDHealthForPRS::SetQueryAssetSeverityForTests(
		[&bHasSnapshot](const FString& AssetPath, ESmartHealthSeverity& OutSeverity)
		{
			if (!bHasSnapshot || AssetPath != TEXT("/Game/SM_Tree.SM_Tree"))
			{
				return false;
			}

			OutSeverity = ESmartHealthSeverity::High;
			return true;
		});

	ESmartHealthSeverity Severity = ESmartHealthSeverity::Info;
	TestTrue(
		TEXT("PRS can read SCD health for a known asset."),
		FSCDHealthForPRS::QueryAssetSeverity(TEXT("/Game/SM_Tree.SM_Tree"), Severity));
	TestEqual(TEXT("Severity passed through."), Severity, ESmartHealthSeverity::High);

	bHasSnapshot = false;
	TestFalse(
		TEXT("No snapshot => honest false."),
		FSCDHealthForPRS::QueryAssetSeverity(TEXT("/Game/SM_Tree.SM_Tree"), Severity));

	FSCDHealthForPRS::ResetQueryAssetSeverityForTests();
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
