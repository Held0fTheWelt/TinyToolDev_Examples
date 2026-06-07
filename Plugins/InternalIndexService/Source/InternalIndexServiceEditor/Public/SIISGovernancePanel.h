/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/** Consolidated governance view: retrieval-only guarantee, MCP security,
 *  data locality, agent-access transparency, integration/bridge health. */
class SIISGovernancePanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SIISGovernancePanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void Refresh();

private:
	TSharedPtr<FString> Body;
	FReply OnWriteContracts();
	FReply OnOpenAgentFolder();
	FReply OnRotateMcpToken();
	void Rebuild();
};
