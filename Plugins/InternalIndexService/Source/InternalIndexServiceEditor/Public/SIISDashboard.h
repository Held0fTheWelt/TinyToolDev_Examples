/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

struct FIISPanelStatusSnapshot;

/** Dashboard landing tab: health cards (Index, MCP, Integrations) + Rebuild/Start/Stop quick actions. */
class SIISDashboard : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SIISDashboard) {}
		SLATE_EVENT(FSimpleDelegate, OnImportShortcut)
		SLATE_EVENT(FSimpleDelegate, OnOpenIndexRoot)
		SLATE_EVENT(FSimpleDelegate, OnQuickSearch)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Refresh card text from a fresh status snapshot. */
	void Refresh(bool bForce = false);

	FString GetQuickSearchQuery() const;

private:
	TSharedPtr<class SHorizontalBox> CardRow;
	TSharedPtr<class SHorizontalBox> WorkRow;
	TSharedPtr<class SEditableTextBox> QuickSearchBox;
	FString LastCardFingerprint;
	// Card texts are held as widget handles and updated via SetText() (not Text_Lambda),
	// so the blocks stay non-volatile. The visible idle flicker came from AutoWrapText
	// inside the fixed-width SBox cards: AutoWrapText derives its wrap width from the
	// allotted geometry, which ping-pongs between prepass and arrange every frame. The
	// blocks below use a fixed WrapTextAt() instead -- geometry-independent, so the wrap
	// width is constant and the layout cannot oscillate.
	TSharedPtr<class STextBlock> IndexStateBlock;
	TSharedPtr<class STextBlock> IndexCatalogBlock;
	TSharedPtr<class STextBlock> IndexChunksBlock;
	TSharedPtr<class STextBlock> IndexVectorsBlock;
	TSharedPtr<class STextBlock> McpStateBlock;
	TSharedPtr<class STextBlock> McpEndpointBlock;
	TSharedPtr<class STextBlock> McpTokenBlock;
	TSharedPtr<class STextBlock> McpRequestsBlock;
	TSharedPtr<class STextBlock> McpActionBlock;
	TSharedPtr<class STextBlock> IntegrationsStateBlock;
	TSharedPtr<class STextBlock> IntegrationsBodyBlock;
	bool bCachedMcpRunning = false;
	FSimpleDelegate ImportShortcutDelegate;
	FSimpleDelegate OpenIndexRootDelegate;
	FSimpleDelegate QuickSearchDelegate;
	void BuildCardRow();
	void UpdateCardTexts(const FIISPanelStatusSnapshot& Snapshot);
	void RebuildWorkRow();

	void OnStartMcp();
	void OnStopMcp();
	void OnRebuildCatalog();
};
