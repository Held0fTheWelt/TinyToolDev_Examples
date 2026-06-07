/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#include "SIISControlPanel.h"

#include "SIISDashboard.h"
#include "SIISGovernancePanel.h"
#include "SIISSettingsPanel.h"
#include "Misc/App.h"
#include "Engine/Engine.h"
#include "IISChunkCatalog.h"
#include "IISMcpEndpoint.h"
#include "IISPythonBridge.h"
#include "IISStoragePaths.h"
#include "IISSubsystem.h"
#include "InternalIndexServiceEditorModule.h"
#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SIISControlPanel"

namespace IISControlPanelUtil
{
	static UIISSubsystem* ResolveSubsystem()
	{
		return GEngine ? GEngine->GetEngineSubsystem<UIISSubsystem>() : nullptr;
	}

	static bool TryLoadJsonRoot(const FString& Path, TSharedPtr<FJsonObject>& OutRoot)
	{
		OutRoot.Reset();
		FString JsonText;
		if (!FPaths::FileExists(Path) || !FFileHelper::LoadFileToString(JsonText, *Path))
		{
			return false;
		}

		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		return FJsonSerializer::Deserialize(Reader, OutRoot) && OutRoot.IsValid();
	}

	static FString GetJsonStringField(const TSharedPtr<FJsonObject>& Root, const FString& FieldName)
	{
		if (!Root.IsValid())
		{
			return FString();
		}

		FString Value;
		Root->TryGetStringField(FieldName, Value);
		return Value;
	}

	static FString GetJsonNumberFieldAsString(const TSharedPtr<FJsonObject>& Root, const FString& FieldName)
	{
		if (!Root.IsValid())
		{
			return FString();
		}

		double Number = 0.0;
		if (Root->TryGetNumberField(FieldName, Number))
		{
			return FString::Printf(TEXT("%.0f"), Number);
		}
		return FString();
	}

	static FString GetSummaryObjectField(
		const TSharedPtr<FJsonObject>& Root,
		const FString& ObjectName,
		const FString& FieldName)
	{
		const TSharedPtr<FJsonObject>* SummaryObj = nullptr;
		if (!Root.IsValid() || !Root->TryGetObjectField(ObjectName, SummaryObj) || !SummaryObj)
		{
			return FString();
		}

		FString Value;
		(*SummaryObj)->TryGetStringField(FieldName, Value);
		return Value;
	}

	static FString FormatReportSummary(const FString& ReportPath, const TArray<FString>& FieldNames)
	{
		if (!FPaths::FileExists(ReportPath))
		{
			return FString::Printf(TEXT("no data yet (%s)"), *FPaths::GetCleanFilename(ReportPath));
		}

		TSharedPtr<FJsonObject> Root;
		if (!TryLoadJsonRoot(ReportPath, Root))
		{
			return FString::Printf(TEXT("report present but unreadable: %s"), *ReportPath);
		}

		TArray<FString> Lines;
		Lines.Add(FString::Printf(TEXT("Report: %s"), *ReportPath));
		for (const FString& FieldName : FieldNames)
		{
			const FString TopLevel = GetJsonStringField(Root, FieldName);
			const FString FromSummary = GetSummaryObjectField(Root, TEXT("summary"), FieldName);
			const FString Value = !TopLevel.IsEmpty() ? TopLevel : FromSummary;
			if (!Value.IsEmpty())
			{
				Lines.Add(FString::Printf(TEXT("%s: %s"), *FieldName, *Value));
			}
		}

		const FString CountFromSummary = GetSummaryObjectField(Root, TEXT("summary"), TEXT("chunk_count"));
		if (!CountFromSummary.IsEmpty())
		{
			Lines.Add(FString::Printf(TEXT("chunk_count: %s"), *CountFromSummary));
		}

		const FString CountTop = GetJsonNumberFieldAsString(Root, TEXT("chunk_count"));
		if (!CountTop.IsEmpty())
		{
			Lines.Add(FString::Printf(TEXT("chunk_count: %s"), *CountTop));
		}

		return FString::Join(Lines, TEXT("\n"));
	}

	static FString FindNewestFileInDirectory(const FString& Directory, const TArray<FString>& Extensions)
	{
		TArray<FString> FoundFiles;
		IFileManager::Get().FindFiles(FoundFiles, *(Directory / TEXT("*")), true, false);

		FString BestPath;
		FDateTime BestTime;
		for (const FString& FileName : FoundFiles)
		{
			bool bMatches = Extensions.Num() == 0;
			for (const FString& Extension : Extensions)
			{
				if (FileName.EndsWith(Extension))
				{
					bMatches = true;
					break;
				}
			}
			if (!bMatches)
			{
				continue;
			}

			const FString FullPath = Directory / FileName;
			const FDateTime Timestamp = IFileManager::Get().GetTimeStamp(*FullPath);
			if (!BestPath.IsEmpty() && Timestamp < BestTime)
			{
				continue;
			}

			BestTime = Timestamp;
			BestPath = FullPath;
		}

		return BestPath;
	}
}

void SIISControlPanel::Construct(const FArguments& InArgs)
{
	OverviewBody = MakeShared<FString>();
	ImportsBody = MakeShared<FString>();
	CatalogBody = MakeShared<FString>();
	EmbeddingsBody = MakeShared<FString>();
	SearchResultsBody = MakeShared<FString>(TEXT("Enter a query and run search."));
	AgentMcpBody = MakeShared<FString>();
	ReportsBody = MakeShared<FString>();
	ActionStatus = MakeShared<FString>(TEXT("Ready."));

	RefreshOverview();
	RefreshImports();
	RefreshCatalog();
	RefreshEmbeddings();
	RefreshAgentMcp();
	RefreshReports();

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			MakeHeader()
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()[MakeTabButton(0, LOCTEXT("DashboardTab", "Dashboard"))]
			+ SHorizontalBox::Slot().AutoWidth()[MakeTabButton(1, LOCTEXT("IndexTab", "Index"))]
			+ SHorizontalBox::Slot().AutoWidth()[MakeTabButton(2, LOCTEXT("UseTab", "Use"))]
			+ SHorizontalBox::Slot().AutoWidth()[MakeTabButton(3, LOCTEXT("AgentsTab", "Agents"))]
			+ SHorizontalBox::Slot().AutoWidth()[MakeTabButton(4, LOCTEXT("GovernanceTab", "Governance"))]
			+ SHorizontalBox::Slot().AutoWidth()[MakeTabButton(5, LOCTEXT("DiagnosticsTab", "Diagnostics"))]
			+ SHorizontalBox::Slot().AutoWidth()[MakeTabButton(6, LOCTEXT("SettingsTab", "Settings"))]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.f)
		[
			MakeActionStatusRow()
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SAssignNew(Switcher, SWidgetSwitcher)
			+ SWidgetSwitcher::Slot()[MakeDashboardTab()]
			+ SWidgetSwitcher::Slot()[MakeIndexSection()]
			+ SWidgetSwitcher::Slot()[MakeUseSection()]
			+ SWidgetSwitcher::Slot()[MakeAgentsSection()]
			+ SWidgetSwitcher::Slot()[MakeGovernanceTab()]
			+ SWidgetSwitcher::Slot()[MakeDiagnosticsSection()]
			+ SWidgetSwitcher::Slot()[MakeSettingsTab()]
		]
	];

	SelectTab(0);
}

bool SIISControlPanel::IsServiceReady() const
{
	UIISSubsystem* Subsystem = IISControlPanelUtil::ResolveSubsystem();
	return Subsystem && Subsystem->IsAvailable();
}

void SIISControlPanel::SetActionStatus(const FString& Message)
{
	*ActionStatus = Message;
}

TSharedRef<SWidget> SIISControlPanel::MakeActionStatusRow()
{
	return SNew(STextBlock)
		.AutoWrapText(true)
		.Text_Lambda([this]()
		{
			return FText::FromString(ActionStatus.IsValid() ? *ActionStatus : FString());
		});
}

TSharedRef<SWidget> SIISControlPanel::MakeHeader()
{
	const FString ProjectLabel = FApp::GetProjectName();
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(
				TEXT("Internal Index Service - %s - Retrieval-only"),
				*ProjectLabel)))
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f, 0.f, 0.f)
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("Settings")))
			.ToolTipText(LOCTEXT("SettingsGearTip", "Open Settings tab"))
			.OnClicked_Lambda([this]()
			{
				SelectTab(SettingsTabIndex);
				return FReply::Handled();
			})
		]
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("RefreshAll", "Refresh"))
			.OnClicked_Lambda([this]()
			{
				OnRefreshAll();
				return FReply::Handled();
			})
		];
}

void SIISControlPanel::OnRefreshAll()
{
	RefreshOverview();
	RefreshImports();
	RefreshCatalog();
	RefreshEmbeddings();
	RefreshAgentMcp();
	RefreshReports();
	if (Dashboard.IsValid())
	{
		Dashboard->Refresh(true);
	}
	if (Governance.IsValid())
	{
		Governance->Refresh();
	}
}

void SIISControlPanel::OnQuickSearchFromDashboard()
{
	if (Dashboard.IsValid() && SearchQueryBox.IsValid())
	{
		SearchQueryBox->SetText(FText::FromString(Dashboard->GetQuickSearchQuery()));
	}
	SelectTab(2);
	OnRunSearch();
}

void SIISControlPanel::FocusImportsSection()
{
	SelectTab(1);
}

void SIISControlPanel::SelectTab(const int32 Index)
{
	ActiveTab = Index;
	if (Switcher.IsValid())
	{
		Switcher->SetActiveWidgetIndex(Index);
	}

	for (int32 i = 0; i < TabButtons.Num(); ++i)
	{
		if (TabButtons[i].IsValid())
		{
			TabButtons[i]->SetColorAndOpacity(
				i == Index ? FLinearColor::White : FLinearColor(0.6f, 0.6f, 0.6f, 1.f));
		}
	}
}

TSharedRef<SWidget> SIISControlPanel::MakeTabButton(const int32 Index, const FText& Label)
{
	TSharedRef<SButton> Button = SNew(SButton)
		.Text(Label)
		.OnClicked_Lambda([this, Index]()
		{
			SelectTab(Index);
			return FReply::Handled();
		});
	TabButtons.Add(Button);
	return Button;
}

TSharedRef<SWidget> SIISControlPanel::MakeDashboardTab()
{
	return SAssignNew(Dashboard, SIISDashboard)
		.OnImportShortcut(FSimpleDelegate::CreateSP(this, &SIISControlPanel::FocusImportsSection))
		.OnOpenIndexRoot(FSimpleDelegate::CreateLambda([this]()
		{
			OnOpenIndexRoot();
		}))
		.OnQuickSearch(FSimpleDelegate::CreateSP(this, &SIISControlPanel::OnQuickSearchFromDashboard));
}

TSharedRef<SWidget> SIISControlPanel::MakeIndexSection()
{
	return SNew(SScrollBox)
		+ SScrollBox::Slot().Padding(4.f)[MakeImportsTab()]
		+ SScrollBox::Slot().Padding(4.f)[MakeCatalogTab()]
		+ SScrollBox::Slot().Padding(4.f)[MakeEmbeddingsTab()];
}

TSharedRef<SWidget> SIISControlPanel::MakeUseSection()
{
	return MakeSearchTab();
}

TSharedRef<SWidget> SIISControlPanel::MakeAgentsSection()
{
	return MakeAgentMcpTab();
}

TSharedRef<SWidget> SIISControlPanel::MakeGovernanceTab()
{
	return SAssignNew(Governance, SIISGovernancePanel);
}

TSharedRef<SWidget> SIISControlPanel::MakeDiagnosticsSection()
{
	return SNew(SScrollBox)
		+ SScrollBox::Slot().Padding(4.f)[MakeOverviewTab()]
		+ SScrollBox::Slot().Padding(4.f)[MakeReportsTab()];
}

TSharedRef<SWidget> SIISControlPanel::MakeSettingsTab()
{
	return SNew(SIISSettingsPanel);
}

void SIISControlPanel::RefreshOverview()
{
	FIISStoragePaths::EnsureDefaultFolders();

	UIISSubsystem* Subsystem = IISControlPanelUtil::ResolveSubsystem();
	const bool bAvailable = Subsystem && Subsystem->IsAvailable();
	const FString Version = bAvailable ? Subsystem->GetServiceVersion() : TEXT("n/a");
	const FString IndexRoot = bAvailable ? Subsystem->GetDefaultIndexRoot() : TEXT("(unavailable)");

	const FString CatalogPath = FIISChunkCatalog::GetCatalogPath();
	const FString VectorPath = FIISStoragePaths::GetVectorsDir() / TEXT("chunk_vectors.jsonl");
	const bool bCatalogExists = FPaths::FileExists(CatalogPath);
	const bool bVectorsExist = FPaths::FileExists(VectorPath);

	const bool bMcpEnabled = FIISMcpEndpoint::IsEnabledByConfig();
	const FIISMcpStatus McpStatus = FModuleManager::LoadModuleChecked<FInternalIndexServiceEditorModule>(
		TEXT("InternalIndexServiceEditor")).GetMcpStatus();

	TArray<FString> Lines;
	Lines.Add(FString::Printf(TEXT("IIS available: %s"), bAvailable ? TEXT("yes") : TEXT("no")));
	Lines.Add(FString::Printf(TEXT("Service version: %s"), *Version));
	Lines.Add(FString::Printf(TEXT("Index root: %s"), *IndexRoot));
	Lines.Add(FString::Printf(TEXT("Catalog DB: %s (%s)"), *CatalogPath, bCatalogExists ? TEXT("present") : TEXT("missing")));
	Lines.Add(FString::Printf(
		TEXT("Vector store: %s (%s)"),
		*VectorPath,
		bVectorsExist ? TEXT("present") : TEXT("missing")));
	Lines.Add(FString::Printf(
		TEXT("MCP endpoint: %s (configured port %d, running=%s, requests=%lld)"),
		bMcpEnabled ? TEXT("enabled") : TEXT("disabled"),
		McpStatus.Port,
		McpStatus.bRunning ? TEXT("yes") : TEXT("no"),
		static_cast<long long>(McpStatus.RequestCount)));

	*OverviewBody = FString::Join(Lines, TEXT("\n"));
}

void SIISControlPanel::RefreshImports()
{
	const FString ReportPath = FIISStoragePaths::GetReportsDir() / TEXT("uii_iis_docking_report.json");
	*ImportsBody = IISControlPanelUtil::FormatReportSummary(
		ReportPath,
		{TEXT("status"), TEXT("handoff_contract_path"), TEXT("import_report_path"), TEXT("catalog_report_path")});
}

void SIISControlPanel::RefreshCatalog()
{
	const FString IndexReportPath = FIISStoragePaths::GetIndexesDir() / TEXT("catalog_build_report.json");
	const FString LegacyReportPath = FIISStoragePaths::GetReportsDir() / TEXT("catalog_build_report.json");
	const FString ReportPath = FPaths::FileExists(IndexReportPath) ? IndexReportPath : LegacyReportPath;
	*CatalogBody = IISControlPanelUtil::FormatReportSummary(
		ReportPath,
		{TEXT("status"), TEXT("catalog_path"), TEXT("generated_at_utc")});
}

void SIISControlPanel::RefreshEmbeddings()
{
	const FString ReportPath = FIISStoragePaths::GetEmbeddingsDir() / TEXT("embedding_run_report.json");
	*EmbeddingsBody = IISControlPanelUtil::FormatReportSummary(
		ReportPath,
		{TEXT("status"), TEXT("jobs_executed"), TEXT("jobs_failed"), TEXT("vectors_written")});
}

void SIISControlPanel::RefreshAgentMcp()
{
	FIISStoragePaths::EnsureDefaultFolders();

	const FString ContractsDir = FIISStoragePaths::GetAgentContractsDir();
	const FString ManifestPath = ContractsDir / TEXT("mcp_tool_manifest.json");
	const bool bManifestExists = FPaths::FileExists(ManifestPath);

	const FIISMcpStatus McpStatus = FModuleManager::LoadModuleChecked<FInternalIndexServiceEditorModule>(
		TEXT("InternalIndexServiceEditor")).GetMcpStatus();

	TArray<FString> Lines;
	Lines.Add(FString::Printf(TEXT("Agent contracts dir: %s"), *ContractsDir));
	Lines.Add(FString::Printf(
		TEXT("MCP tool manifest: %s (%s)"),
		*ManifestPath,
		bManifestExists ? TEXT("present") : TEXT("missing")));
	Lines.Add(FString::Printf(
		TEXT("MCP running: %s on port %d (token present: %s, requests: %lld)"),
		McpStatus.bRunning ? TEXT("yes") : TEXT("no"),
		McpStatus.Port,
		McpStatus.bTokenPresent ? TEXT("yes") : TEXT("no"),
		static_cast<long long>(McpStatus.RequestCount)));
	Lines.Add(TEXT("External stdio proxy: Plugins/InternalIndexService/Tools/mcp_proxy (see README in that folder)."));

	*AgentMcpBody = FString::Join(Lines, TEXT("\n"));
}

void SIISControlPanel::RefreshReports()
{
	FIISStoragePaths::EnsureDefaultFolders();

	const FString ReportsDir = FIISStoragePaths::GetReportsDir();
	const FString LogsDir = FIISStoragePaths::GetLogsDir();
	const FString SmokeReport = IISControlPanelUtil::FindNewestFileInDirectory(ReportsDir, {TEXT(".json")});
	const FString SearchReport = ReportsDir / TEXT("search_report.json");
	const FString McpLog = LogsDir / TEXT("mcp_requests.jsonl");

	TArray<FString> Lines;
	Lines.Add(FString::Printf(TEXT("Reports dir: %s"), *ReportsDir));
	Lines.Add(FString::Printf(
		TEXT("Latest JSON report: %s"),
		SmokeReport.IsEmpty() ? TEXT("none") : *SmokeReport));
	Lines.Add(FString::Printf(
		TEXT("search_report.json: %s"),
		FPaths::FileExists(SearchReport) ? *SearchReport : TEXT("missing")));
	Lines.Add(FString::Printf(
		TEXT("mcp_requests.jsonl: %s"),
		FPaths::FileExists(McpLog) ? *McpLog : TEXT("missing")));

	*ReportsBody = FString::Join(Lines, TEXT("\n"));
}

FReply SIISControlPanel::OnOpenIndexRoot()
{
	FIISStoragePaths::EnsureDefaultFolders();
	const FString IndexRoot = FIISStoragePaths::GetDefaultIndexRoot();
	FPlatformProcess::ExploreFolder(*IndexRoot);
	SetActionStatus(FString::Printf(TEXT("Opened index root: %s"), *IndexRoot));
	return FReply::Handled();
}

FReply SIISControlPanel::OnBuildCatalog()
{
	if (!IsServiceReady())
	{
		SetActionStatus(TEXT("IIS is not available; cannot build catalog."));
		return FReply::Handled();
	}

	FString ReportPath;
	TArray<FString> Warnings;
	const bool bSuccess = UIISPythonBridge::BuildChunkCatalogWithWarnings(ReportPath, Warnings);
	RefreshCatalog();

	FString Message = FString::Printf(
		TEXT("Build catalog %s. Report: %s"),
		bSuccess ? TEXT("succeeded") : TEXT("failed"),
		*ReportPath);
	if (Warnings.Num() > 0)
	{
		Message += FString::Printf(TEXT(" (%d warning(s))"), Warnings.Num());
	}
	SetActionStatus(Message);
	return FReply::Handled();
}

FReply SIISControlPanel::OnBuildEmbeddingJobs()
{
	UIISSubsystem* Subsystem = IISControlPanelUtil::ResolveSubsystem();
	if (!IsServiceReady() || !Subsystem)
	{
		SetActionStatus(TEXT("IIS is not available; cannot build embedding jobs."));
		return FReply::Handled();
	}

	FString ReportPath;
	TArray<FString> Warnings;
	const bool bSuccess = Subsystem->BuildEmbeddingJobs(ReportPath, Warnings);
	RefreshEmbeddings();

	SetActionStatus(FString::Printf(
		TEXT("Build embedding jobs %s. Report: %s"),
		bSuccess ? TEXT("succeeded") : TEXT("failed"),
		*ReportPath));
	return FReply::Handled();
}

FReply SIISControlPanel::OnExecuteEmbeddingJobs()
{
	UIISSubsystem* Subsystem = IISControlPanelUtil::ResolveSubsystem();
	if (!IsServiceReady() || !Subsystem)
	{
		SetActionStatus(TEXT("IIS is not available; cannot execute embedding jobs."));
		return FReply::Handled();
	}

	FString ReportPath;
	TArray<FString> Warnings;
	const bool bSuccess = Subsystem->ExecuteEmbeddingJobs(EmbeddingMaxJobs, ReportPath, Warnings);
	RefreshEmbeddings();
	RefreshOverview();

	SetActionStatus(FString::Printf(
		TEXT("Execute embedding jobs (max %d) %s. Report: %s"),
		EmbeddingMaxJobs,
		bSuccess ? TEXT("succeeded") : TEXT("failed"),
		*ReportPath));
	return FReply::Handled();
}

FReply SIISControlPanel::OnRunSearch()
{
	UIISSubsystem* Subsystem = IISControlPanelUtil::ResolveSubsystem();
	if (!IsServiceReady() || !Subsystem)
	{
		SetActionStatus(TEXT("IIS is not available; cannot search."));
		return FReply::Handled();
	}

	FString QueryText;
	if (SearchQueryBox.IsValid())
	{
		QueryText = SearchQueryBox->GetText().ToString().TrimStartAndEnd();
	}

	if (QueryText.IsEmpty())
	{
		SetActionStatus(TEXT("Search query is empty."));
		return FReply::Handled();
	}

	FIISSearchQuery Query;
	Query.QueryText = QueryText;
	Query.SearchMode = SelectedSearchMode;
	Query.MaxResults = 10;

	FIISSearchResponse Response;
	const bool bSuccess = Subsystem->Search(Query, Response);

	TArray<FString> Lines;
	Lines.Add(FString::Printf(TEXT("Status: %s"), bSuccess ? TEXT("ok") : TEXT("failed")));
	Lines.Add(FString::Printf(TEXT("Results: %d"), Response.Results.Num()));
	for (const FIISSearchResult& Result : Response.Results)
	{
		Lines.Add(FString::Printf(TEXT("- %s (score %.4f)"), *Result.ChunkId, Result.Score));
	}
	if (Response.Warnings.Num() > 0)
	{
		Lines.Add(TEXT("Warnings:"));
		for (const FString& Warning : Response.Warnings)
		{
			Lines.Add(FString::Printf(TEXT("  %s"), *Warning));
		}
	}
	if (Response.Errors.Num() > 0)
	{
		Lines.Add(TEXT("Errors:"));
		for (const FString& Error : Response.Errors)
		{
			Lines.Add(FString::Printf(TEXT("  %s"), *Error));
		}
	}

	*SearchResultsBody = FString::Join(Lines, TEXT("\n"));
	SetActionStatus(FString::Printf(TEXT("Search finished with %d result(s)."), Response.Results.Num()));
	return FReply::Handled();
}

FReply SIISControlPanel::OnOpenAgentFolder()
{
	FIISStoragePaths::EnsureDefaultFolders();
	const FString AgentRoot = FIISStoragePaths::GetAgentDir();
	FPlatformProcess::ExploreFolder(*AgentRoot);
	SetActionStatus(FString::Printf(TEXT("Opened agent folder: %s"), *AgentRoot));
	return FReply::Handled();
}

FReply SIISControlPanel::OnOpenReportsFolder()
{
	FIISStoragePaths::EnsureDefaultFolders();
	const FString ReportsDir = FIISStoragePaths::GetReportsDir();
	FPlatformProcess::ExploreFolder(*ReportsDir);
	SetActionStatus(FString::Printf(TEXT("Opened reports folder: %s"), *ReportsDir));
	return FReply::Handled();
}

FReply SIISControlPanel::OnOpenLogsFolder()
{
	FIISStoragePaths::EnsureDefaultFolders();
	const FString LogsDir = FIISStoragePaths::GetLogsDir();
	FPlatformProcess::ExploreFolder(*LogsDir);
	SetActionStatus(FString::Printf(TEXT("Opened logs folder: %s"), *LogsDir));
	return FReply::Handled();
}

TSharedRef<SWidget> SIISControlPanel::MakeOverviewTab()
{
	return SNew(SScrollBox)
		+ SScrollBox::Slot()
		.Padding(4.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.Text(LOCTEXT("OpenIndexRoot", "Open Index Root"))
				.IsEnabled_Lambda([this]() { return IsServiceReady(); })
				.OnClicked(this, &SIISControlPanel::OnOpenIndexRoot)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 8.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text_Lambda([this]()
				{
					return FText::FromString(OverviewBody.IsValid() ? *OverviewBody : FString());
				})
			]
		];
}

TSharedRef<SWidget> SIISControlPanel::MakeImportsTab()
{
	return SNew(SScrollBox)
		+ SScrollBox::Slot()
		.Padding(4.f)
		[
			SNew(STextBlock)
			.AutoWrapText(true)
			.Text_Lambda([this]()
			{
				return FText::FromString(ImportsBody.IsValid() ? *ImportsBody : FString());
			})
		];
}

TSharedRef<SWidget> SIISControlPanel::MakeCatalogTab()
{
	return SNew(SScrollBox)
		+ SScrollBox::Slot()
		.Padding(4.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.Text(LOCTEXT("BuildCatalog", "Build Catalog"))
				.IsEnabled_Lambda([this]() { return IsServiceReady(); })
				.OnClicked(this, &SIISControlPanel::OnBuildCatalog)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 8.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text_Lambda([this]()
				{
					return FText::FromString(CatalogBody.IsValid() ? *CatalogBody : FString());
				})
			]
		];
}

TSharedRef<SWidget> SIISControlPanel::MakeEmbeddingsTab()
{
	return SNew(SScrollBox)
		+ SScrollBox::Slot()
		.Padding(4.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("BuildEmbeddingJobs", "Build Embedding Jobs"))
					.IsEnabled_Lambda([this]() { return IsServiceReady(); })
					.OnClicked(this, &SIISControlPanel::OnBuildEmbeddingJobs)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(8.f, 0.f, 0.f, 0.f)
				[
					SNew(SButton)
					.Text(LOCTEXT("ExecuteEmbeddingJobs", "Execute Max Jobs"))
					.IsEnabled_Lambda([this]() { return IsServiceReady(); })
					.OnClicked(this, &SIISControlPanel::OnExecuteEmbeddingJobs)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(8.f, 0.f, 0.f, 0.f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock).Text(LOCTEXT("MaxJobsLabel", "Max:"))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(4.f, 0.f, 0.f, 0.f)
				[
					SNew(SSpinBox<int32>)
					.MinValue(1)
					.MaxValue(100)
					.Value_Lambda([this]() { return EmbeddingMaxJobs; })
					.OnValueChanged_Lambda([this](const int32 NewValue)
					{
						EmbeddingMaxJobs = FMath::Clamp(NewValue, 1, 100);
					})
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 8.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text_Lambda([this]()
				{
					return FText::FromString(EmbeddingsBody.IsValid() ? *EmbeddingsBody : FString());
				})
			]
		];
}

TSharedRef<SWidget> SIISControlPanel::MakeSearchTab()
{
	return SNew(SScrollBox)
		+ SScrollBox::Slot()
		.Padding(4.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(SearchQueryBox, SEditableTextBox)
				.HintText(LOCTEXT("SearchQueryHint", "Search query"))
				.IsEnabled_Lambda([this]() { return IsServiceReady(); })
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 4.f, 0.f, 0.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("LexicalMode", "Lexical"))
					.IsEnabled_Lambda([this]() { return IsServiceReady(); })
					.OnClicked_Lambda([this]()
					{
						SelectedSearchMode = EIISSearchMode::Lexical;
						return FReply::Handled();
					})
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f, 0.f, 0.f)
				[
					SNew(SButton)
					.Text(LOCTEXT("VectorMode", "Vector"))
					.IsEnabled_Lambda([this]() { return IsServiceReady(); })
					.OnClicked_Lambda([this]()
					{
						SelectedSearchMode = EIISSearchMode::Vector;
						return FReply::Handled();
					})
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(4.f, 0.f, 0.f, 0.f)
				[
					SNew(SButton)
					.Text(LOCTEXT("HybridMode", "Hybrid"))
					.IsEnabled_Lambda([this]() { return IsServiceReady(); })
					.OnClicked_Lambda([this]()
					{
						SelectedSearchMode = EIISSearchMode::Hybrid;
						return FReply::Handled();
					})
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f)
				[
					SNew(SButton)
					.Text(LOCTEXT("RunSearch", "Search"))
					.IsEnabled_Lambda([this]() { return IsServiceReady(); })
					.OnClicked(this, &SIISControlPanel::OnRunSearch)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 8.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text_Lambda([this]()
				{
					return FText::FromString(SearchResultsBody.IsValid() ? *SearchResultsBody : FString());
				})
			]
		];
}

TSharedRef<SWidget> SIISControlPanel::MakeAgentMcpTab()
{
	return SNew(SScrollBox)
		+ SScrollBox::Slot()
		.Padding(4.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.Text(LOCTEXT("OpenAgentFolder", "Open Agent Output Folder"))
				.OnClicked(this, &SIISControlPanel::OnOpenAgentFolder)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 8.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text_Lambda([this]()
				{
					return FText::FromString(AgentMcpBody.IsValid() ? *AgentMcpBody : FString());
				})
			]
		];
}

TSharedRef<SWidget> SIISControlPanel::MakeReportsTab()
{
	return SNew(SScrollBox)
		+ SScrollBox::Slot()
		.Padding(4.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("OpenReports", "Open Reports"))
					.OnClicked(this, &SIISControlPanel::OnOpenReportsFolder)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f)
				[
					SNew(SButton)
					.Text(LOCTEXT("OpenLogs", "Open Logs"))
					.OnClicked(this, &SIISControlPanel::OnOpenLogsFolder)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f)
				[
					SNew(SButton)
					.Text(LOCTEXT("OpenIndexRootReports", "Open Index Root"))
					.OnClicked(this, &SIISControlPanel::OnOpenIndexRoot)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 8.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text_Lambda([this]()
				{
					return FText::FromString(ReportsBody.IsValid() ? *ReportsBody : FString());
				})
			]
		];
}

#undef LOCTEXT_NAMESPACE
