/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#include "InternalIndexServiceUMCPBridgeModule.h"

#include "Features/IModularFeatures.h"
#include "IISUmcpToolProvider.h"
#include "IUmcpToolProvider.h"

DEFINE_LOG_CATEGORY_STATIC(LogIISUMCPBridge, Log, All);

void FInternalIndexServiceUMCPBridgeModule::StartupModule()
{
	Provider = MakeUnique<FIISUmcpToolProvider>();
	IModularFeatures::Get().RegisterModularFeature(
		IUmcpToolProvider::GetModularFeatureName(), Provider.Get());
	UE_LOG(LogIISUMCPBridge, Log, TEXT("Registered IIS provider with the Unified MCP Server."));
}

void FInternalIndexServiceUMCPBridgeModule::ShutdownModule()
{
	if (Provider.IsValid())
	{
		IModularFeatures::Get().UnregisterModularFeature(
			IUmcpToolProvider::GetModularFeatureName(), Provider.Get());
		Provider.Reset();
	}
	UE_LOG(LogIISUMCPBridge, Log, TEXT("Unregistered IIS provider from the Unified MCP Server."));
}

IMPLEMENT_MODULE(FInternalIndexServiceUMCPBridgeModule, InternalIndexServiceUMCPBridge)
