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

#include "SmartContentDietIISSimilarityBridgeModule.h"

#include "SCDHealthEvidenceIISBridgeCommon.h"
#include "Engine/Engine.h"
#include "IISSearchTypes.h"
#include "IISServiceInterface.h"
#include "ISmartContentDietRegistry.h"
#include "SCDIISSimilarityGrouping.h"
#include "Modules/ModuleManager.h"
#include "Subsystems/EngineSubsystem.h"
#include "UObject/UObjectGlobals.h"

IMPLEMENT_MODULE(FSmartContentDietIISSimilarityBridgeModule, SmartContentDietIISSimilarityBridge)

namespace
{
	ISmartContentDietRegistry* ResolveSCDRegistry()
	{
		if (!GEngine)
		{
			return nullptr;
		}

		UClass* SubsystemClass =
			FindObject<UClass>(nullptr, TEXT("/Script/SmartContentDiet.SmartContentDietSubsystem"));
		if (!SubsystemClass)
		{
			SubsystemClass =
				LoadObject<UClass>(nullptr, TEXT("/Script/SmartContentDiet.SmartContentDietSubsystem"));
		}
		if (!SubsystemClass)
		{
			return nullptr;
		}

		return Cast<ISmartContentDietRegistry>(GEngine->GetEngineSubsystemBase(SubsystemClass));
	}
}

bool FIISSimilarityProvider::ComputeSimilarityGroups(
	const FSmartSimilarityRequest& Request,
	FSmartSimilarityResult& OutResult)
{
	IInternalIndexService* IIS = SCDHealthEvidenceIISBridgeCommon::ResolveIIS();
	if (!IIS || !IIS->IsAvailable())
	{
		OutResult.Warnings.Add(TEXT("Internal Index Service is not available; semantic similarity skipped."));
		return false;
	}

	// The provider advertises ESmartSimilarityMode::Semantic, so it must run a
	// vector search — not the default lexical mode — to honestly label results.
	static constexpr float MinSemanticScore = 0.85f;

	TSet<FString> KnownAssetPaths;
	KnownAssetPaths.Reserve(Request.Assets.Num());
	for (const FSmartAssetDescriptor& Desc : Request.Assets)
	{
		KnownAssetPaths.Add(Desc.AssetPath);
	}

	for (const FSmartAssetDescriptor& Desc : Request.Assets)
	{
		// NOTE (limitation, B1-d): FSmartAssetDescriptor carries no semantic text
		// beyond the path (only AssetClass + derived Features such as sha256/width),
		// so the vector query keys on the asset path and depends on IIS having
		// indexed those assets. Richer semantic queries need a fuller descriptor
		// from SCD; until then this is the strongest honest signal available.
		FIISSearchQuery Query;
		Query.QueryText = Desc.AssetPath;
		Query.SearchMode = EIISSearchMode::Vector;
		FIISSearchResponse Response;
		if (!IIS->Search(Query, Response))
		{
			continue;
		}

		FSmartSimilarityGroup Group;
		if (SCDIISSimilarityGrouping::BuildSemanticGroup(
				Desc.AssetPath, KnownAssetPaths, Response, MinSemanticScore, Group))
		{
			OutResult.Groups.Add(Group);
		}
	}
	return true;
}

bool FIISSimilarityProvider::IsImplementationAvailable() const
{
	IInternalIndexService* IIS = SCDHealthEvidenceIISBridgeCommon::ResolveIIS();
	return IIS && IIS->IsAvailable();
}

void FSmartContentDietIISSimilarityBridgeModule::StartupModule()
{
	if (ISmartContentDietRegistry* Registry = ResolveSCDRegistry())
	{
		TSharedRef<FIISSimilarityProvider> Candidate = MakeShared<FIISSimilarityProvider>();
		if (Candidate->IsImplementationAvailable())
		{
			Provider = Candidate;
			Registry->RegisterSimilarityProvider(Provider.ToSharedRef(), ESmartProviderTier::FirstParty);
		}
	}
}

void FSmartContentDietIISSimilarityBridgeModule::ShutdownModule()
{
	if (Provider.IsValid())
	{
		if (ISmartContentDietRegistry* Registry = ResolveSCDRegistry())
		{
			Registry->UnregisterSimilarityProvider(Provider.ToSharedRef());
		}
		Provider.Reset();
	}
}
