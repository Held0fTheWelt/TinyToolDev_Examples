/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#pragma once

#include "CoreMinimal.h"
#include "IISSearchTypes.h"
#include "Widgets/SCompoundWidget.h"

class SButton;
class SEditableTextBox;
class SIISDashboard;
class SIISGovernancePanel;
class SWidgetSwitcher;

class SIISControlPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SIISControlPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedPtr<SWidgetSwitcher> Switcher;
	int32 ActiveTab = 0;

	TSharedPtr<FString> OverviewBody;
	TSharedPtr<FString> ImportsBody;
	TSharedPtr<FString> CatalogBody;
	TSharedPtr<FString> EmbeddingsBody;
	TSharedPtr<FString> SearchResultsBody;
	TSharedPtr<FString> AgentMcpBody;
	TSharedPtr<FString> ReportsBody;
	TSharedPtr<FString> ActionStatus;

	TSharedPtr<SEditableTextBox> SearchQueryBox;
	TSharedPtr<SIISDashboard> Dashboard;
	TSharedPtr<SIISGovernancePanel> Governance;
	TArray<TSharedPtr<SButton>> TabButtons;
	static constexpr int32 SettingsTabIndex = 6;
	EIISSearchMode SelectedSearchMode = EIISSearchMode::Lexical;
	int32 EmbeddingMaxJobs = 10;

	bool IsServiceReady() const;
	void OnRefreshAll();
	void SelectTab(int32 Index);
	void OnQuickSearchFromDashboard();
	void FocusImportsSection();
	void SetActionStatus(const FString& Message);

	void RefreshOverview();
	void RefreshImports();
	void RefreshCatalog();
	void RefreshEmbeddings();
	void RefreshAgentMcp();
	void RefreshReports();

	FReply OnOpenIndexRoot();
	FReply OnBuildCatalog();
	FReply OnBuildEmbeddingJobs();
	FReply OnExecuteEmbeddingJobs();
	FReply OnRunSearch();
	FReply OnOpenAgentFolder();
	FReply OnOpenReportsFolder();
	FReply OnOpenLogsFolder();

	TSharedRef<SWidget> MakeHeader();
	TSharedRef<SWidget> MakeTabButton(int32 Index, const FText& Label);
	TSharedRef<SWidget> MakeDashboardTab();
	TSharedRef<SWidget> MakeIndexSection();
	TSharedRef<SWidget> MakeUseSection();
	TSharedRef<SWidget> MakeAgentsSection();
	TSharedRef<SWidget> MakeGovernanceTab();
	TSharedRef<SWidget> MakeDiagnosticsSection();
	TSharedRef<SWidget> MakeSettingsTab();
	TSharedRef<SWidget> MakeOverviewTab();
	TSharedRef<SWidget> MakeImportsTab();
	TSharedRef<SWidget> MakeCatalogTab();
	TSharedRef<SWidget> MakeEmbeddingsTab();
	TSharedRef<SWidget> MakeSearchTab();
	TSharedRef<SWidget> MakeAgentMcpTab();
	TSharedRef<SWidget> MakeReportsTab();
	TSharedRef<SWidget> MakeActionStatusRow();
};
