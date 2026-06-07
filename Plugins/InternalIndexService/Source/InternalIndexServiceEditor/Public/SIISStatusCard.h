/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/** Action buttons for a status card (label + on-click). */
using FIISStatusCardActions = TArray<TPair<FText, FSimpleDelegate>>;

/** Visual state of a status card. */
enum class EIISCardState : uint8
{
	Ready,
	Idle,
	Working,
	Error
};

/** A single dashboard status card: title, state dot, value lines, optional actions. */
class SIISStatusCard : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SIISStatusCard)
		: _Title()
		, _State(EIISCardState::Idle)
	{}
		SLATE_ARGUMENT(FText, Title)
		SLATE_ARGUMENT(EIISCardState, State)
		SLATE_ARGUMENT(TArray<FText>, Lines)
		SLATE_ARGUMENT(FIISStatusCardActions, Actions)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	static FText StateGlyph(EIISCardState State);
};
