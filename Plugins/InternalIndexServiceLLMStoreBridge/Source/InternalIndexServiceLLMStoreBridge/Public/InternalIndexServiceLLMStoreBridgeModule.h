/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA
 *
 * This file is part of the "IIS LLM Store Bridge" Unreal Engine plugin.
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
#include "IISEmbeddingTypes.h"
#include "Modules/ModuleManager.h"

/**
 * Optional runtime bridge between IIS and LLM Store.
 *
 * The module owns only the lifetime of the IIS embedding executor. IIS still
 * owns embedding job orchestration and vector persistence; LLM Store still owns
 * provider/model routing and route governance.
 */
class FInternalIndexServiceLLMStoreBridgeModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	// Kept alive for as long as the bridge module is loaded because the IIS
	// executor registry stores and calls the shared executor instance.
	TSharedPtr<IIISEmbeddingRouteExecutor> EmbeddingExecutor;
};
