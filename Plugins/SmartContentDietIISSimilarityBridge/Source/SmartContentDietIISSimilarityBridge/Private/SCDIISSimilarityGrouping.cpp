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

#include "SCDIISSimilarityGrouping.h"

#include "IISSearchTypes.h"
#include "SmartSimilarityContracts.h"

namespace SCDIISSimilarityGrouping
{
bool BuildSemanticGroup(
	const FString& SelfAssetPath,
	const TSet<FString>& KnownAssetPaths,
	const FIISSearchResponse& Response,
	const float MinScore,
	FSmartSimilarityGroup& OutGroup)
{
	OutGroup = FSmartSimilarityGroup();
	OutGroup.Members.Add(SelfAssetPath);

	TSet<FString> Seen;
	Seen.Add(SelfAssetPath);
	float BestPeerScore = 0.0f;

	for (const FIISSearchResult& Hit : Response.Results)
	{
		if (Hit.Score < MinScore)
		{
			continue;
		}

		// Only adopt a hit whose id is one of the request's own assets, so the
		// group stays addressable in SCD's asset namespace (never raw IIS ids).
		if (!KnownAssetPaths.Contains(Hit.ChunkId) || Seen.Contains(Hit.ChunkId))
		{
			continue;
		}

		OutGroup.Members.Add(Hit.ChunkId);
		Seen.Add(Hit.ChunkId);
		BestPeerScore = FMath::Max(BestPeerScore, Hit.Score);
	}

	if (OutGroup.Members.Num() < 2)
	{
		OutGroup = FSmartSimilarityGroup();
		return false;
	}

	OutGroup.Signals.Add(ESmartSimilaritySignal::Semantic);
	OutGroup.bApproximate = true;
	OutGroup.Confidence = BestPeerScore;
	OutGroup.Rationale = TEXT("Semantically similar per Internal Index Service (vector search).");
	return true;
}
}
