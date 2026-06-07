/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#include "SIISGovernancePanel.h"

#include "IISAgentAccessService.h"
#include "IISAgentAccessTypes.h"
#include "IISPanelStatus.h"
#include "IISStoragePaths.h"
#include "InternalIndexServiceEditorModule.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SIISGovernancePanel"

namespace
{
	FInternalIndexServiceEditorModule& EditorModule()
	{
		return FModuleManager::LoadModuleChecked<FInternalIndexServiceEditorModule>(TEXT("InternalIndexServiceEditor"));
	}
}

void SIISGovernancePanel::Construct(const FArguments& InArgs)
{
	Body = MakeShared<FString>();
	Rebuild();

	ChildSlot
	[
		SNew(SScrollBox)
		+ SScrollBox::Slot().Padding(4.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("WriteContracts", "Write/Refresh Agent Contracts"))
					.OnClicked(this, &SIISGovernancePanel::OnWriteContracts)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f)
				[
					SNew(SButton)
					.Text(LOCTEXT("OpenAgentFolder", "Open Agent Folder"))
					.OnClicked(this, &SIISGovernancePanel::OnOpenAgentFolder)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f)
				[
					SNew(SButton)
					.Text(LOCTEXT("RotateToken", "Rotate MCP Token"))
					.OnClicked(this, &SIISGovernancePanel::OnRotateMcpToken)
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text_Lambda([this]() { return FText::FromString(Body.IsValid() ? *Body : FString()); })
			]
		]
	];
}

void SIISGovernancePanel::Refresh()
{
	Rebuild();
}

void SIISGovernancePanel::Rebuild()
{
	const FIISPanelStatusSnapshot S = IISCapturePanelStatus();
	const FIISAgentToolResponse Guarantee;

	const FString ContractsDir = FIISStoragePaths::GetAgentContractsDir();
	const bool bManifest = FPaths::FileExists(ContractsDir / TEXT("mcp_tool_manifest.json"));
	const FString EmbeddingReport = FIISStoragePaths::GetEmbeddingsDir() / TEXT("embedding_run_report.json");
	const bool bRanEmbeddings = FPaths::FileExists(EmbeddingReport);

	const bool bBridgeModuleLoaded =
		FModuleManager::Get().IsModuleLoaded(TEXT("InternalIndexServiceLLMStoreBridge"));

	TArray<FString> Lines;
	Lines.Add(TEXT("== Retrieval-only guarantee =="));
	Lines.Add(FString::Printf(TEXT("  migration decisions: %s"), Guarantee.bAllowsMigrationDecision ? TEXT("ALLOWED (!)") : TEXT("disabled")));
	Lines.Add(FString::Printf(TEXT("  patch generation:    %s"), Guarantee.bAllowsPatchGeneration ? TEXT("ALLOWED (!)") : TEXT("disabled")));
	Lines.Add(FString::Printf(TEXT("  project mutation:    %s"), Guarantee.bAllowsProjectMutation ? TEXT("ALLOWED (!)") : TEXT("disabled")));
	Lines.Add(TEXT(""));
	Lines.Add(TEXT("== MCP security =="));
	Lines.Add(FString::Printf(TEXT("  endpoint: 127.0.0.1:%d (loopback only)"), S.Mcp.Port));
	Lines.Add(FString::Printf(TEXT("  running: %s   token present: %s   requests: %lld"),
		S.Mcp.bRunning ? TEXT("yes") : TEXT("no"),
		S.Mcp.bTokenPresent ? TEXT("yes") : TEXT("no"),
		static_cast<long long>(S.Mcp.RequestCount)));
	Lines.Add(TEXT(""));
	Lines.Add(TEXT("== Data locality =="));
	Lines.Add(FString::Printf(TEXT("  index root: %s"), *S.IndexRoot));
	Lines.Add(FString::Printf(TEXT("  catalog present: %s   vectors present: %s"),
		S.bCatalogPresent ? TEXT("yes") : TEXT("no"),
		S.bVectorsPresent ? TEXT("yes") : TEXT("no")));
	Lines.Add(TEXT(""));
	Lines.Add(TEXT("== Integrations / bridges =="));
	if (S.Integrations.Num() == 0)
	{
		Lines.Add(TEXT("  none registered"));
	}
	for (const FIISIntegrationStatus& I : S.Integrations)
	{
		FString Detail = I.bRegistered ? TEXT("registered") : TEXT("inactive");
		if (I.ExecutorId.Equals(TEXT("llmstore"), ESearchCase::IgnoreCase))
		{
			if (!bBridgeModuleLoaded)
			{
				Detail += TEXT(" - bridge plugin module not loaded (enable InternalIndexServiceLLMStoreBridge in the project)");
			}
			else if (!I.bRegistered)
			{
				Detail += TEXT(" - bridge loaded but executor not registered");
			}
		}
		Lines.Add(FString::Printf(TEXT("  %s: %s"), *I.ExecutorId, *Detail));
	}
	Lines.Add(FString::Printf(TEXT("  last embedding run: %s (evidence the executor functioned)"),
		bRanEmbeddings ? TEXT("present") : TEXT("none yet")));
	Lines.Add(TEXT(""));
	Lines.Add(TEXT("== Agent access transparency =="));
	Lines.Add(FString::Printf(TEXT("  contracts dir: %s"), *ContractsDir));
	Lines.Add(FString::Printf(TEXT("  MCP tool manifest: %s"), bManifest ? TEXT("present") : TEXT("missing")));
	Lines.Add(TEXT("  exposed tools: iis_search, iis_get_context_pack, iis_get_chunk,"));
	Lines.Add(TEXT("                 iis_get_source_references, iis_find_usages, iis_explain_blueprint"));

	*Body = FString::Join(Lines, TEXT("\n"));
}

FReply SIISGovernancePanel::OnWriteContracts()
{
	FString ContractsPath;
	FIISAgentAccessService::WriteAgentToolContracts(ContractsPath);
	Rebuild();
	return FReply::Handled();
}

FReply SIISGovernancePanel::OnOpenAgentFolder()
{
	FIISStoragePaths::EnsureDefaultFolders();
	FPlatformProcess::ExploreFolder(*FIISStoragePaths::GetAgentDir());
	return FReply::Handled();
}

FReply SIISGovernancePanel::OnRotateMcpToken()
{
	EditorModule().RotateMcpToken();
	Rebuild();
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
