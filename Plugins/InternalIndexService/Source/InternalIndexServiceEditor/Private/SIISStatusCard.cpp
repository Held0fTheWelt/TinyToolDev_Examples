/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#include "SIISStatusCard.h"

#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SIISStatusCard"

FText SIISStatusCard::StateGlyph(EIISCardState State)
{
	switch (State)
	{
	case EIISCardState::Ready:
		return FText::FromString(TEXT("ready"));
	case EIISCardState::Working:
		return FText::FromString(TEXT("working"));
	case EIISCardState::Error:
		return FText::FromString(TEXT("error"));
	case EIISCardState::Idle:
	default:
		return FText::FromString(TEXT("idle"));
	}
}

void SIISStatusCard::Construct(const FArguments& InArgs)
{
	TSharedRef<SVerticalBox> Body = SNew(SVerticalBox);

	Body->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
		[
			SNew(STextBlock).Text(InArgs._Title)
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			SNew(STextBlock).Text(StateGlyph(InArgs._State))
		]
	];

	for (const FText& Line : InArgs._Lines)
	{
		Body->AddSlot().AutoHeight()
		[
			SNew(STextBlock).AutoWrapText(true).Text(Line)
		];
	}

	if (InArgs._Actions.Num() > 0)
	{
		TSharedRef<SHorizontalBox> ActionRow = SNew(SHorizontalBox);
		for (const TPair<FText, FSimpleDelegate>& Action : InArgs._Actions)
		{
			const FSimpleDelegate Callback = Action.Value;
			ActionRow->AddSlot().AutoWidth().Padding(0.f, 0.f, 4.f, 0.f)
			[
				SNew(SButton)
				.Text(Action.Key)
				.OnClicked_Lambda([Callback]()
				{
					Callback.ExecuteIfBound();
					return FReply::Handled();
				})
			];
		}
		Body->AddSlot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)[ActionRow];
	}

	ChildSlot
	[
		SNew(SBorder)
		.Padding(8.f)
		[
			Body
		]
	];
}

#undef LOCTEXT_NAMESPACE
