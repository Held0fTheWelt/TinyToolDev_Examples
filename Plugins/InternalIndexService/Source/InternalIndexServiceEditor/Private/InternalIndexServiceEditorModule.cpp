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

#include "InternalIndexServiceEditorModule.h"

#include "IISMcpServerManager.h"
#include "IISPythonBridge.h"
#include "IISStoragePaths.h"
#include "IISToolMenuRegistration.h"
#include "SIISControlPanel.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "HAL/PlatformProcess.h"
#include "Logging/LogMacros.h"
#include "Misc/Paths.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"

DEFINE_LOG_CATEGORY_STATIC(LogInternalIndexServiceEditor, Log, All);

const FName FInternalIndexServiceEditorModule::IISControlPanelTabId(TEXT("IISControlPanel"));

void FInternalIndexServiceEditorModule::StartupModule()
{
	UE_LOG(LogInternalIndexServiceEditor, Log, TEXT("InternalIndexServiceEditor started."));

	TArray<FString> Warnings;
	FIISStoragePaths::EnsureDefaultFolders(nullptr, &Warnings);
	for (const FString& Warning : Warnings)
	{
		UE_LOG(LogInternalIndexServiceEditor, Warning, TEXT("%s"), *Warning);
	}

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		IISControlPanelTabId,
		FOnSpawnTab::CreateRaw(this, &FInternalIndexServiceEditorModule::SpawnControlPanelTab))
		.SetDisplayName(NSLOCTEXT("IIS", "IISPanelTitle", "Internal Index Service"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	if (UToolMenus::IsToolMenuUIEnabled())
	{
		RegisterMenus();
	}
	else
	{
		UToolMenus::RegisterStartupCallback(
			FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FInternalIndexServiceEditorModule::RegisterMenus));
	}

	if (FIISMcpEndpoint::IsEnabledByConfig())
	{
		McpManager = MakeUnique<FIISMcpServerManager>();
		McpManager->Start();
		if (McpManager->IsRunning())
		{
			const FIISMcpStatus Status = McpManager->GetStatus();
			UE_LOG(LogInternalIndexServiceEditor, Display, TEXT("IIS MCP endpoint listening on 127.0.0.1:%d"), Status.Port);
		}
		else
		{
			UE_LOG(LogInternalIndexServiceEditor, Warning, TEXT("IIS MCP endpoint failed to bind on 127.0.0.1:%d"), FIISMcpEndpoint::GetConfiguredPort());
			McpManager.Reset();
		}
	}
}

void FInternalIndexServiceEditorModule::ShutdownModule()
{
	if (McpManager.IsValid())
	{
		McpManager->Stop();
		McpManager.Reset();
	}

	if (FSlateApplication::IsInitialized())
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(IISControlPanelTabId);
	}

	UE_LOG(LogInternalIndexServiceEditor, Log, TEXT("InternalIndexServiceEditor shut down."));

	UToolMenus::UnRegisterStartupCallback(this);
	if (UToolMenus::IsToolMenuUIEnabled())
	{
		FIISToolMenuRegistration::UnregisterMenus(this);
	}
}

FIISMcpStatus FInternalIndexServiceEditorModule::GetMcpStatus() const
{
	if (McpManager.IsValid())
	{
		return McpManager->GetStatus();
	}

	FIISMcpStatus Status;
	Status.bRunning = false;
	Status.Port = FIISMcpEndpoint::GetConfiguredPort();
	Status.bTokenPresent = false;
	Status.RequestCount = 0;
	return Status;
}

void FInternalIndexServiceEditorModule::StartMcp()
{
	if (McpManager.IsValid() && McpManager->IsRunning())
	{
		return;
	}
	McpManager = MakeUnique<FIISMcpServerManager>();
	McpManager->Start();
	if (!McpManager->IsRunning())
	{
		UE_LOG(LogInternalIndexServiceEditor, Warning, TEXT("IIS MCP endpoint failed to bind on 127.0.0.1:%d"), FIISMcpEndpoint::GetConfiguredPort());
		McpManager.Reset();
	}
	else
	{
		const FIISMcpStatus Status = McpManager->GetStatus();
		UE_LOG(LogInternalIndexServiceEditor, Display, TEXT("IIS MCP endpoint started on 127.0.0.1:%d"), Status.Port);
	}
}

void FInternalIndexServiceEditorModule::StopMcp()
{
	if (McpManager.IsValid())
	{
		McpManager->Stop();
		McpManager.Reset();
		UE_LOG(LogInternalIndexServiceEditor, Display, TEXT("IIS MCP endpoint stopped."));
	}
}

void FInternalIndexServiceEditorModule::RotateMcpToken()
{
	if (McpManager.IsValid() && McpManager->IsRunning())
	{
		McpManager->RotateToken();
		UE_LOG(LogInternalIndexServiceEditor, Display, TEXT("IIS MCP endpoint token rotated."));
	}
}

void FInternalIndexServiceEditorModule::OpenControlPanelTab()
{
	FGlobalTabmanager::Get()->TryInvokeTab(IISControlPanelTabId);
}

TSharedRef<SDockTab> FInternalIndexServiceEditorModule::SpawnControlPanelTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SIISControlPanel)
		];
}

void FInternalIndexServiceEditorModule::RegisterMenus()
{
	FIISToolMenuRegistration::RegisterMenus(
		this,
		FSimpleDelegate::CreateRaw(this, &FInternalIndexServiceEditorModule::OpenControlPanelTab),
		FSimpleDelegate::CreateRaw(this, &FInternalIndexServiceEditorModule::BuildChunkCatalog),
		FSimpleDelegate::CreateRaw(this, &FInternalIndexServiceEditorModule::StartMcp),
		FSimpleDelegate::CreateRaw(this, &FInternalIndexServiceEditorModule::StopMcp),
		FSimpleDelegate::CreateRaw(this, &FInternalIndexServiceEditorModule::OpenIndexRoot));
}

void FInternalIndexServiceEditorModule::SmokeTestIIS()
{
	FString ReportPath;
	const bool bSuccess = UIISPythonBridge::SmokeTestService(ReportPath);
	UE_LOG(
		LogInternalIndexServiceEditor,
		Log,
		TEXT("Internal Index Service smoke test %s. Report: %s"),
		bSuccess ? TEXT("succeeded") : TEXT("failed"),
		*ReportPath);
}

void FInternalIndexServiceEditorModule::OpenIndexRoot()
{
	FIISStoragePaths::EnsureDefaultFolders();
	const FString IndexRoot = FIISStoragePaths::GetDefaultIndexRoot();
	UE_LOG(LogInternalIndexServiceEditor, Log, TEXT("Opening Internal Index Service root: %s"), *IndexRoot);
	FPlatformProcess::ExploreFolder(*IndexRoot);
}

void FInternalIndexServiceEditorModule::ImportPreparedChunksJsonl()
{
	UE_LOG(
		LogInternalIndexServiceEditor,
		Log,
		TEXT("IIS prepared chunks import is available through UIISPythonBridge.import_prepared_chunks_jsonl(path) or the C++ service interface."));
}

void FInternalIndexServiceEditorModule::BuildChunkCatalog()
{
	FString ReportPath;
	TArray<FString> Warnings;
	const bool bSuccess = UIISPythonBridge::BuildChunkCatalogWithWarnings(ReportPath, Warnings);
	UE_LOG(
		LogInternalIndexServiceEditor,
		Log,
		TEXT("IIS chunk catalog build %s. Report: %s"),
		bSuccess ? TEXT("succeeded") : TEXT("failed"),
		*ReportPath);
	for (const FString& Warning : Warnings)
	{
		UE_LOG(LogInternalIndexServiceEditor, Warning, TEXT("%s"), *Warning);
	}
}

void FInternalIndexServiceEditorModule::SearchChunksLexical()
{
	FString ReportPath;
	const bool bSuccess = UIISPythonBridge::SearchChunksLexical(TEXT("guardrail"), 10, ReportPath);
	UE_LOG(
		LogInternalIndexServiceEditor,
		Log,
		TEXT("IIS lexical search diagnostic %s. Report: %s"),
		bSuccess ? TEXT("succeeded") : TEXT("failed"),
		*ReportPath);
}

void FInternalIndexServiceEditorModule::SearchChunksHybrid()
{
	FString ReportPath;
	const bool bSuccess = UIISPythonBridge::SearchChunksHybrid(TEXT("guardrail"), 10, ReportPath);
	UE_LOG(
		LogInternalIndexServiceEditor,
		Log,
		TEXT("IIS hybrid search diagnostic %s. Report: %s"),
		bSuccess ? TEXT("succeeded") : TEXT("failed"),
		*ReportPath);
}

void FInternalIndexServiceEditorModule::BuildContextPack()
{
	FString ReportPath;
	const bool bSuccess = UIISPythonBridge::BuildContextPackForQuery(TEXT("planning"), TEXT("Hybrid"), 10, ReportPath);
	UE_LOG(
		LogInternalIndexServiceEditor,
		Log,
		TEXT("IIS context pack build %s. Report: %s"),
		bSuccess ? TEXT("succeeded") : TEXT("failed"),
		*ReportPath);
}

void FInternalIndexServiceEditorModule::AgentSearch()
{
	FString ReportPath;
	const bool bSuccess = UIISPythonBridge::AgentSearch(TEXT("guardrail"), TEXT("Hybrid"), 10, ReportPath);
	UE_LOG(
		LogInternalIndexServiceEditor,
		Log,
		TEXT("IIS agent search %s. Report: %s"),
		bSuccess ? TEXT("succeeded") : TEXT("failed"),
		*ReportPath);
}

void FInternalIndexServiceEditorModule::AgentContextPack()
{
	FString ReportPath;
	const bool bSuccess = UIISPythonBridge::AgentGetContextPack(TEXT("planning"), TEXT("Hybrid"), 10, ReportPath);
	UE_LOG(
		LogInternalIndexServiceEditor,
		Log,
		TEXT("IIS agent context pack %s. Report: %s"),
		bSuccess ? TEXT("succeeded") : TEXT("failed"),
		*ReportPath);
}

void FInternalIndexServiceEditorModule::WriteAgentToolContracts()
{
	FString ContractsPath;
	const bool bSuccess = UIISPythonBridge::WriteAgentToolContracts(ContractsPath);
	UE_LOG(
		LogInternalIndexServiceEditor,
		Log,
		TEXT("IIS agent tool contracts write %s. Contracts: %s"),
		bSuccess ? TEXT("succeeded") : TEXT("failed"),
		*ContractsPath);
}

void FInternalIndexServiceEditorModule::OpenAgentOutputFolder()
{
	FIISStoragePaths::EnsureDefaultFolders();
	const FString AgentRoot = FIISStoragePaths::GetAgentDir();
	UE_LOG(LogInternalIndexServiceEditor, Log, TEXT("Opening IIS agent output folder: %s"), *AgentRoot);
	FPlatformProcess::ExploreFolder(*AgentRoot);
}

void FInternalIndexServiceEditorModule::BuildEmbeddingJobs()
{
	FString ReportPath;
	const bool bSuccess = UIISPythonBridge::BuildEmbeddingJobs(ReportPath);
	UE_LOG(
		LogInternalIndexServiceEditor,
		Log,
		TEXT("IIS build embedding jobs %s. Report: %s"),
		bSuccess ? TEXT("succeeded") : TEXT("failed"),
		*ReportPath);
}

void FInternalIndexServiceEditorModule::ExecuteEmbeddingJobs()
{
	FString ReportPath;
	const bool bSuccess = UIISPythonBridge::ExecuteEmbeddingJobs(10, ReportPath);
	UE_LOG(
		LogInternalIndexServiceEditor,
		Log,
		TEXT("IIS execute embedding jobs %s. Report: %s"),
		bSuccess ? TEXT("succeeded") : TEXT("failed"),
		*ReportPath);
}

IMPLEMENT_MODULE(FInternalIndexServiceEditorModule, InternalIndexServiceEditor)
