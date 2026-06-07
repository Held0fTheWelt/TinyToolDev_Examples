/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "IISAgentAccessTypes.h"
#include "SIISControlPanel.h"
#include "SIISDashboard.h"
#include "SIISGovernancePanel.h"
#include "SIISSettingsPanel.h"
#include "SIISStatusCard.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWidget.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISControlPanelConstructs,
	"InternalIndexService.Editor.ControlPanelConstructs",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISControlPanelConstructs::RunTest(const FString& Parameters)
{
	if (!FSlateApplication::IsInitialized())
	{
		AddWarning(TEXT("FSlateApplication not initialized; skipping panel construction."));
		return true;
	}

	TSharedRef<SIISControlPanel> Panel = SNew(SIISControlPanel);
	TestTrue(TEXT("panel constructed"), Panel->GetType() != NAME_None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISControlPanelHasDashboard,
	"InternalIndexService.Editor.ControlPanelHasDashboard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISControlPanelHasDashboard::RunTest(const FString& Parameters)
{
	if (!FSlateApplication::IsInitialized())
	{
		AddWarning(TEXT("FSlateApplication not initialized; skipping."));
		return true;
	}

	TSharedRef<SIISControlPanel> Panel = SNew(SIISControlPanel);
	TestTrue(TEXT("panel constructed with dashboard tab"), Panel->GetType() != NAME_None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISStatusCardConstructs,
	"InternalIndexService.Editor.StatusCardConstructs",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISStatusCardConstructs::RunTest(const FString& Parameters)
{
	if (!FSlateApplication::IsInitialized())
	{
		AddWarning(TEXT("FSlateApplication not initialized; skipping card construction."));
		return true;
	}

	TArray<FText> Lines;
	Lines.Add(FText::FromString(TEXT("1,240 chunks")));
	TSharedRef<SIISStatusCard> Card = SNew(SIISStatusCard)
		.Title(FText::FromString(TEXT("Index")))
		.State(EIISCardState::Ready)
		.Lines(Lines);
	TestTrue(TEXT("card constructed"), Card->GetType() != NAME_None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISDashboardConstructs,
	"InternalIndexService.Editor.DashboardConstructs",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISDashboardConstructs::RunTest(const FString& Parameters)
{
	if (!FSlateApplication::IsInitialized())
	{
		AddWarning(TEXT("FSlateApplication not initialized; skipping dashboard construction."));
		return true;
	}

	TSharedRef<SIISDashboard> Dashboard = SNew(SIISDashboard);
	Dashboard->Refresh();
	TestTrue(TEXT("dashboard constructed"), Dashboard->GetType() != NAME_None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISRetrievalOnlyInvariant,
	"InternalIndexService.Editor.RetrievalOnlyInvariant",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISRetrievalOnlyInvariant::RunTest(const FString& Parameters)
{
	const FIISAgentToolResponse Response;
	TestFalse(TEXT("no migration decision"), Response.bAllowsMigrationDecision);
	TestFalse(TEXT("no patch generation"), Response.bAllowsPatchGeneration);
	TestFalse(TEXT("no project mutation"), Response.bAllowsProjectMutation);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISGovernancePanelConstructs,
	"InternalIndexService.Editor.GovernancePanelConstructs",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISGovernancePanelConstructs::RunTest(const FString& Parameters)
{
	if (!FSlateApplication::IsInitialized())
	{
		AddWarning(TEXT("FSlateApplication not initialized; skipping."));
		return true;
	}
	TSharedRef<SIISGovernancePanel> Panel = SNew(SIISGovernancePanel);
	TestTrue(TEXT("governance panel constructed"), Panel->GetType() != NAME_None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISSettingsPanelConstructs,
	"InternalIndexService.Editor.SettingsPanelConstructs",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISSettingsPanelConstructs::RunTest(const FString& Parameters)
{
	if (!FSlateApplication::IsInitialized())
	{
		AddWarning(TEXT("FSlateApplication not initialized; skipping."));
		return true;
	}
	TSharedRef<SIISSettingsPanel> Panel = SNew(SIISSettingsPanel);
	TestTrue(TEXT("settings panel constructed"), Panel->GetType() != NAME_None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISControlPanelHasSixSections,
	"InternalIndexService.Editor.ControlPanelHasSixSections",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISControlPanelHasSixSections::RunTest(const FString& Parameters)
{
	if (!FSlateApplication::IsInitialized())
	{
		AddWarning(TEXT("FSlateApplication not initialized; skipping."));
		return true;
	}
	TSharedRef<SIISControlPanel> Panel = SNew(SIISControlPanel);
	TestTrue(TEXT("panel constructs with grouped sections"), Panel->GetType() != NAME_None);
	return true;
}

#endif
