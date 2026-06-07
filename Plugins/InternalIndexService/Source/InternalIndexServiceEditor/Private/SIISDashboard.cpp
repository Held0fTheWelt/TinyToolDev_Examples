/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#include "SIISDashboard.h"

#include "IISPanelStatus.h"
#include "IISPythonBridge.h"
#include "InternalIndexServiceEditorModule.h"
#include "SIISStatusCard.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "SIISDashboard"

namespace
{
	FInternalIndexServiceEditorModule& EditorModule()
	{
		return FModuleManager::LoadModuleChecked<FInternalIndexServiceEditorModule>(TEXT("InternalIndexServiceEditor"));
	}

	FString BuildDashboardFingerprint(const FIISPanelStatusSnapshot& Snapshot)
	{
		TArray<FString> Parts;
		Parts.Reserve(8 + Snapshot.Integrations.Num());
		Parts.Add(Snapshot.bServiceAvailable ? TEXT("service=1") : TEXT("service=0"));
		Parts.Add(FString::Printf(TEXT("version=%s"), *Snapshot.ServiceVersion));
		Parts.Add(Snapshot.bCatalogPresent ? TEXT("catalog=1") : TEXT("catalog=0"));
		Parts.Add(Snapshot.bVectorsPresent ? TEXT("vectors=1") : TEXT("vectors=0"));
		Parts.Add(FString::Printf(TEXT("chunks=%d"), Snapshot.ChunkCount));
		Parts.Add(Snapshot.Mcp.bRunning ? TEXT("mcp_running=1") : TEXT("mcp_running=0"));
		Parts.Add(Snapshot.Mcp.bTokenPresent ? TEXT("mcp_token=1") : TEXT("mcp_token=0"));
		Parts.Add(FString::Printf(TEXT("mcp_port=%d"), Snapshot.Mcp.Port));

		for (const FIISIntegrationStatus& Integration : Snapshot.Integrations)
		{
			Parts.Add(FString::Printf(
				TEXT("integration=%s:%s"),
				*Integration.ExecutorId,
				Integration.bRegistered ? TEXT("1") : TEXT("0")));
		}

		return FString::Join(Parts, TEXT("|"));
	}
}

void SIISDashboard::Construct(const FArguments& InArgs)
{
	ImportShortcutDelegate = InArgs._OnImportShortcut;
	OpenIndexRootDelegate = InArgs._OnOpenIndexRoot;
	QuickSearchDelegate = InArgs._OnQuickSearch;

	ChildSlot
	[
		SNew(SScrollBox)
		+ SScrollBox::Slot().Padding(4.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SAssignNew(CardRow, SHorizontalBox)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
			[
				SAssignNew(WorkRow, SHorizontalBox)
			]
		]
	];

	BuildCardRow();
	Refresh(true);
	RebuildWorkRow();
}

void SIISDashboard::Refresh(const bool bForce)
{
	const FIISPanelStatusSnapshot Snapshot = IISCapturePanelStatus();
	const FString Fingerprint = BuildDashboardFingerprint(Snapshot);
	if (!bForce && Fingerprint == LastCardFingerprint)
	{
		return;
	}

	LastCardFingerprint = Fingerprint;
	UpdateCardTexts(Snapshot);
}

FString SIISDashboard::GetQuickSearchQuery() const
{
	return QuickSearchBox.IsValid() ? QuickSearchBox->GetText().ToString().TrimStartAndEnd() : FString();
}

void SIISDashboard::OnStartMcp()
{
	EditorModule().StartMcp();
	Refresh(true);
}

void SIISDashboard::OnStopMcp()
{
	EditorModule().StopMcp();
	Refresh(true);
}

void SIISDashboard::OnRebuildCatalog()
{
	FString ReportPath;
	TArray<FString> Warnings;
	UIISPythonBridge::BuildChunkCatalogWithWarnings(ReportPath, Warnings);
	Refresh(true);
}

void SIISDashboard::RebuildWorkRow()
{
	if (!WorkRow.IsValid())
	{
		return;
	}
	WorkRow->ClearChildren();

	{
		TArray<FText> Lines;
		Lines.Add(LOCTEXT("ImportHint", "Prepared chunks JSONL import"));
		TArray<TPair<FText, FSimpleDelegate>> Actions;
		if (ImportShortcutDelegate.IsBound())
		{
			Actions.Add({LOCTEXT("GoImports", "Go to Imports"), ImportShortcutDelegate});
		}
		WorkRow->AddSlot().AutoWidth().Padding(4.f)
		[
			SNew(SIISStatusCard)
			.Title(LOCTEXT("ImportCard", "Import"))
			.State(EIISCardState::Idle)
			.Lines(Lines)
			.Actions(Actions)
		];
	}

	{
		TArray<FText> Lines;
		Lines.Add(LOCTEXT("SearchHint", "Quick lexical search"));
		WorkRow->AddSlot().AutoWidth().Padding(4.f)
		[
			SNew(SBorder).Padding(8.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock).Text(LOCTEXT("SearchCard", "Search"))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
				[
					SAssignNew(QuickSearchBox, SEditableTextBox)
					.HintText(FText::FromString(TEXT("Query...")))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
				[
					SNew(SButton)
					.Text(LOCTEXT("QuickSearchGo", "Go"))
					.OnClicked_Lambda([this]()
					{
						if (QuickSearchDelegate.IsBound())
						{
							QuickSearchDelegate.Execute();
						}
						return FReply::Handled();
					})
				]
			]
		];
	}

	{
		TArray<FText> Lines;
		Lines.Add(LOCTEXT("PrivacyHint", "Local index data on disk"));
		TArray<TPair<FText, FSimpleDelegate>> Actions;
		if (OpenIndexRootDelegate.IsBound())
		{
			Actions.Add({LOCTEXT("OpenRoot", "Open folder"), OpenIndexRootDelegate});
		}
		WorkRow->AddSlot().AutoWidth().Padding(4.f)
		[
			SNew(SIISStatusCard)
			.Title(LOCTEXT("DataCard", "Data & Privacy"))
			.State(EIISCardState::Idle)
			.Lines(Lines)
			.Actions(Actions)
		];
	}
}

void SIISDashboard::BuildCardRow()
{
	if (!CardRow.IsValid())
	{
		return;
	}

	CardRow->AddSlot().AutoWidth().Padding(4.f)
	[
		SNew(SBox).WidthOverride(124.f)
		[
			SNew(SBorder).Padding(8.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
					[
						SNew(STextBlock).Text(LOCTEXT("IndexCard", "Index"))
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SAssignNew(IndexStateBlock, STextBlock)
					]
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SAssignNew(IndexCatalogBlock, STextBlock).WrapTextAt(102.f)
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SAssignNew(IndexChunksBlock, STextBlock).WrapTextAt(102.f)
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SAssignNew(IndexVectorsBlock, STextBlock).WrapTextAt(102.f)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
				[
					SNew(SButton)
					.Text(LOCTEXT("Rebuild", "Rebuild"))
					.OnClicked_Lambda([this]()
					{
						OnRebuildCatalog();
						return FReply::Handled();
					})
				]
			]
		]
	];

	CardRow->AddSlot().AutoWidth().Padding(4.f)
	[
		SNew(SBox).WidthOverride(124.f)
		[
			SNew(SBorder).Padding(8.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
					[
						SNew(STextBlock).Text(LOCTEXT("McpCard", "MCP Server"))
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SAssignNew(McpStateBlock, STextBlock)
					]
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SAssignNew(McpEndpointBlock, STextBlock).WrapTextAt(102.f)
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SAssignNew(McpTokenBlock, STextBlock).WrapTextAt(102.f)
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SAssignNew(McpRequestsBlock, STextBlock).WrapTextAt(102.f)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.OnClicked_Lambda([this]()
					{
						if (bCachedMcpRunning)
						{
							OnStopMcp();
						}
						else
						{
							OnStartMcp();
						}
						return FReply::Handled();
					})
					[
						SAssignNew(McpActionBlock, STextBlock).Text(LOCTEXT("McpStart", "Start"))
					]
				]
			]
		]
	];

	CardRow->AddSlot().AutoWidth().Padding(4.f)
	[
		SNew(SBox).WidthOverride(152.f)
		[
			SNew(SBorder).Padding(8.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
					[
						SNew(STextBlock).Text(LOCTEXT("IntegrationsCard", "Integrations"))
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SAssignNew(IntegrationsStateBlock, STextBlock)
					]
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SAssignNew(IntegrationsBodyBlock, STextBlock)
					.WrapTextAt(130.f)
				]
			]
		]
	];
}

void SIISDashboard::UpdateCardTexts(const FIISPanelStatusSnapshot& S)
{
	const auto SetBlockText = [](const TSharedPtr<STextBlock>& Block, const FString& Value)
	{
		if (Block.IsValid())
		{
			Block->SetText(FText::FromString(Value));
		}
	};

	SetBlockText(IndexStateBlock, S.bCatalogPresent ? TEXT("ready") : TEXT("idle"));
	SetBlockText(IndexCatalogBlock, FString::Printf(TEXT("catalog: %s"), S.bCatalogPresent ? TEXT("present") : TEXT("missing")));
	SetBlockText(
		IndexChunksBlock,
		S.ChunkCount >= 0 ? FString::Printf(TEXT("chunks: %d"), S.ChunkCount) : FString(TEXT("chunks: report unavailable")));
	SetBlockText(IndexVectorsBlock, FString::Printf(TEXT("vectors: %s"), S.bVectorsPresent ? TEXT("present") : TEXT("missing")));

	bCachedMcpRunning = S.Mcp.bRunning;
	SetBlockText(McpStateBlock, S.Mcp.bRunning ? TEXT("ready") : TEXT("idle"));
	SetBlockText(McpEndpointBlock, FString::Printf(TEXT("127.0.0.1:%d"), S.Mcp.Port));
	SetBlockText(McpTokenBlock, FString::Printf(TEXT("token: %s"), S.Mcp.bTokenPresent ? TEXT("set") : TEXT("none")));
	SetBlockText(McpRequestsBlock, FString::Printf(TEXT("requests: %lld"), static_cast<long long>(S.Mcp.RequestCount)));
	SetBlockText(McpActionBlock, S.Mcp.bRunning ? TEXT("Stop") : TEXT("Start"));

	TArray<FString> IntegrationLines;
	if (S.Integrations.Num() == 0)
	{
		IntegrationLines.Add(TEXT("no bridges registered"));
	}
	for (const FIISIntegrationStatus& Integration : S.Integrations)
	{
		IntegrationLines.Add(FString::Printf(
			TEXT("%s: %s"),
			*Integration.ExecutorId,
			Integration.bRegistered ? TEXT("registered") : TEXT("inactive")));
	}

	SetBlockText(IntegrationsStateBlock, S.Integrations.Num() > 0 ? TEXT("ready") : TEXT("idle"));
	SetBlockText(IntegrationsBodyBlock, FString::Join(IntegrationLines, TEXT("\n")));
}

#undef LOCTEXT_NAMESPACE
