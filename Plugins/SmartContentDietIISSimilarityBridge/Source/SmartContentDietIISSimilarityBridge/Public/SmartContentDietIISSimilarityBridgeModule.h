/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA
 *
 * This file is part of the "SmartContentDiet IIS Similarity Bridge" Unreal Engine plugin.
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
#include "Modules/ModuleInterface.h"
#include "ISmartSimilarityProvider.h"

/** Adapter that serves SCD semantic similarity via the IIS public interface. */
class FIISSimilarityProvider : public ISmartSimilarityProvider
{
public:
	virtual FString GetProviderId() const override { return TEXT("iis-similarity"); }
	virtual bool SupportsMode(ESmartSimilarityMode Mode) const override
	{
		return Mode == ESmartSimilarityMode::Semantic;
	}
	virtual bool SupportsAssetClass(FName) const override { return true; }
	virtual bool ComputeSimilarityGroups(
		const FSmartSimilarityRequest& Request,
		FSmartSimilarityResult& OutResult) override;
	bool IsImplementationAvailable() const;
};

class FSmartContentDietIISSimilarityBridgeModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TSharedPtr<FIISSimilarityProvider> Provider;
};
