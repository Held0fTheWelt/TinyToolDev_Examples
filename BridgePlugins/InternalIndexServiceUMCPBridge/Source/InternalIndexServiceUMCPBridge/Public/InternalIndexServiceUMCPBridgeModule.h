/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Templates/UniquePtr.h"

class FIISUmcpToolProvider;

/**
 * Registers the IIS tool provider as a UMCP modular feature for the lifetime of
 * the module. Registration is the only runtime side effect of the bridge.
 */
class FInternalIndexServiceUMCPBridgeModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TUniquePtr<FIISUmcpToolProvider> Provider;
};
