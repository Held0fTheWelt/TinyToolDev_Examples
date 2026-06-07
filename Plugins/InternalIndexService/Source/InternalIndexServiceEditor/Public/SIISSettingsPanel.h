/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/** In-panel mirror of UIISSettings (details view) + Open Project Settings shortcut. */
class SIISSettingsPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SIISSettingsPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FReply OnOpenProjectSettings();
};
