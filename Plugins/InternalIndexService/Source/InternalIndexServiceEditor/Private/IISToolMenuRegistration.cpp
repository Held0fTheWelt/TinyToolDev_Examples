/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA
 *
 * This file is part of the "Internal Index Service" Unreal Engine plugin.
 * Use of this software is governed by the Fab Standard End User License Agreement
 * (EULA) applicable to this product, available at:
 * https://www.fab.com/eula
 *
 * Except as expressly permitted by the Fab Standard EULA, any reproduction,
 * distribution, modification, or use of this software, in whole or in part,
 * is strictly prohibited.
 *
 * This software is provided on an "AS IS" basis, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied, including but not
 * limited to warranties of merchantability, fitness for a particular purpose,
 * and non-infringement.
 * available at: https://www.fab.com/eula.  */

#include "IISToolMenuRegistration.h"

#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "FIISToolMenuRegistration"

void FIISToolMenuRegistration::RegisterMenus(
	void* Owner,
	FSimpleDelegate OpenControlPanelAction,
	FSimpleDelegate BuildCatalogAction,
	FSimpleDelegate StartMcpAction,
	FSimpleDelegate StopMcpAction,
	FSimpleDelegate OpenIndexRootAction)
{
	UToolMenus* ToolMenus = UToolMenus::Get();
	if (!ToolMenus)
	{
		return;
	}

	FToolMenuOwnerScoped OwnerScoped(Owner);
	UToolMenu* Menu = ToolMenus->ExtendMenu(TEXT("LevelEditor.MainMenu.Tools"));
	if (!Menu)
	{
		return;
	}

	FToolMenuSection& Section = Menu->FindOrAddSection(TEXT("InternalIndexService"));
	Section.AddSubMenu(
		TEXT("InternalIndexService"),
		LOCTEXT("InternalIndexServiceMenuLabel", "Internal Index Service"),
		LOCTEXT("InternalIndexServiceMenuTooltip", "Open the Internal Index Service panel and quick actions."),
		FNewToolMenuDelegate::CreateStatic(
			&FIISToolMenuRegistration::FillIISMenu,
			OpenControlPanelAction,
			BuildCatalogAction,
			StartMcpAction,
			StopMcpAction,
			OpenIndexRootAction));
}

void FIISToolMenuRegistration::UnregisterMenus(void* Owner)
{
	UToolMenus::UnregisterOwner(Owner);
}

void FIISToolMenuRegistration::FillIISMenu(
	UToolMenu* Menu,
	FSimpleDelegate OpenControlPanelAction,
	FSimpleDelegate BuildCatalogAction,
	FSimpleDelegate StartMcpAction,
	FSimpleDelegate StopMcpAction,
	FSimpleDelegate OpenIndexRootAction)
{
	if (!Menu)
	{
		return;
	}

	{
		FToolMenuSection& Primary = Menu->FindOrAddSection(TEXT("IISPrimary"));
		Primary.AddMenuEntry(
			TEXT("OpenControlPanel"),
			LOCTEXT("OpenControlPanelLabel", "Open Internal Index Service"),
			LOCTEXT("OpenControlPanelTooltip", "Open the Internal Index Service control panel."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([OpenControlPanelAction]()
			{
				OpenControlPanelAction.ExecuteIfBound();
			})));
	}

	{
		FToolMenuSection& Quick = Menu->FindOrAddSection(TEXT("IISQuickActions"));
		Quick.AddMenuEntry(
			TEXT("BuildIndex"),
			LOCTEXT("BuildIndexLabel", "Build Index"),
			LOCTEXT("BuildIndexTooltip", "Build or update the local chunk catalog."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([BuildCatalogAction]()
			{
				BuildCatalogAction.ExecuteIfBound();
			})));
		Quick.AddMenuEntry(
			TEXT("StartMcp"),
			LOCTEXT("StartMcpLabel", "Start MCP Server"),
			LOCTEXT("StartMcpTooltip", "Start the loopback MCP endpoint."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([StartMcpAction]()
			{
				StartMcpAction.ExecuteIfBound();
			})));
		Quick.AddMenuEntry(
			TEXT("StopMcp"),
			LOCTEXT("StopMcpLabel", "Stop MCP Server"),
			LOCTEXT("StopMcpTooltip", "Stop the loopback MCP endpoint."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([StopMcpAction]()
			{
				StopMcpAction.ExecuteIfBound();
			})));
		Quick.AddMenuEntry(
			TEXT("OpenIndexRoot"),
			LOCTEXT("OpenIndexRootLabel", "Open Index Folder"),
			LOCTEXT("OpenIndexRootTooltip", "Open the Internal Index Service Saved folder."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([OpenIndexRootAction]()
			{
				OpenIndexRootAction.ExecuteIfBound();
			})));
	}
}

#undef LOCTEXT_NAMESPACE
