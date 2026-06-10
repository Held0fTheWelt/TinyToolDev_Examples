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

struct FIISSearchResponse;
struct FSmartSimilarityGroup;

namespace SCDIISSimilarityGrouping
{
	/**
	 * Build one advisory similarity group for SelfAssetPath from an IIS search
	 * response. Group members are restricted to asset paths the caller actually
	 * asked about (KnownAssetPaths): an IIS hit's ChunkId is only adopted as a
	 * member when it resolves to one of those request assets and clears MinScore,
	 * so a group never references opaque IIS chunk ids that the SCD redundancy UI
	 * cannot map back to assets. SelfAssetPath is always excluded from the peer
	 * scan. Returns true (and fills OutGroup with Semantic signal, bApproximate,
	 * Confidence = best peer score, and a rationale) only when at least one peer
	 * joins the self member; otherwise OutGroup is reset empty and it returns false.
	 */
	bool BuildSemanticGroup(
		const FString& SelfAssetPath,
		const TSet<FString>& KnownAssetPaths,
		const FIISSearchResponse& Response,
		float MinScore,
		FSmartSimilarityGroup& OutGroup);
}
