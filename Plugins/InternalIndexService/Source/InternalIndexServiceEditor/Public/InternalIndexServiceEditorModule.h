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

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "IISMcpServerManager.h"
#include "Templates/UniquePtr.h"

class INTERNALINDEXSERVICEEDITOR_API FInternalIndexServiceEditorModule : public IModuleInterface
{
public:
	static const FName IISControlPanelTabId;

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	FIISMcpStatus GetMcpStatus() const;

	/** Start the loopback MCP endpoint if not already running. */
	void StartMcp();
	/** Stop the loopback MCP endpoint. Safe to call when not running. */
	void StopMcp();
	/** Rotate the MCP session bearer token (no-op if not running). */
	void RotateMcpToken();

private:
	void RegisterMenus();
	void OpenControlPanelTab();
	TSharedRef<class SDockTab> SpawnControlPanelTab(const class FSpawnTabArgs& Args);
	void SmokeTestIIS();
	void OpenIndexRoot();
	void ImportPreparedChunksJsonl();
	void BuildChunkCatalog();
	void SearchChunksLexical();
	void SearchChunksHybrid();
	void BuildContextPack();
	void AgentSearch();
	void AgentContextPack();
	void WriteAgentToolContracts();
	void OpenAgentOutputFolder();
	void BuildEmbeddingJobs();
	void ExecuteEmbeddingJobs();

	TUniquePtr<class FIISMcpServerManager> McpManager;
};
