/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#include "SIISSettingsPanel.h"

#include "IISSettings.h"
#include "ISettingsModule.h"
#include "IDetailsView.h"
#include "PropertyEditorModule.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "SIISSettingsPanel"

void SIISSettingsPanel::Construct(const FArguments& InArgs)
{
	FPropertyEditorModule& PropertyEditorModule =
		FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));

	FDetailsViewArgs DetailsArgs;
	DetailsArgs.bAllowSearch = false;
	DetailsArgs.bHideSelectionTip = true;
	DetailsArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;

	TSharedRef<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsArgs);
	DetailsView->SetObject(GetMutableDefault<UIISSettings>());

	ChildSlot
	[
		SNew(SScrollBox)
		+ SScrollBox::Slot().Padding(4.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SButton)
				.Text(LOCTEXT("OpenProjectSettings", "Open in Project Settings"))
				.OnClicked(this, &SIISSettingsPanel::OnOpenProjectSettings)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
			[
				DetailsView
			]
		]
	];
}

FReply SIISSettingsPanel::OnOpenProjectSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>(TEXT("Settings")))
	{
		SettingsModule->ShowViewer(TEXT("Project"), TEXT("Plugins"), TEXT("IISSettings"));
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
