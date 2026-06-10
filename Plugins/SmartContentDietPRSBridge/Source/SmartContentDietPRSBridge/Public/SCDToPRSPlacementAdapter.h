/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA
 *
 * This file is part of the "SmartContentDiet PRS Bridge" Unreal Engine plugin.
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
#include "PRSPlacementTypes.h"
#include "PRSResultTypes.h"

/** A single SCD-side move proposal such as review-folder consolidation. */
struct FSCDPlacementMove
{
	FString SourceAssetPath;
	FString DestinationFolder;
	FString Rationale;
};

/**
 * Forwards SCD move proposals to PRS. SCD owns the decision: callers build a
 * plan, preview it, show the diff, and call Apply only after explicit user
 * confirmation. This adapter never auto-applies.
 */
class SMARTCONTENTDIETPRSBRIDGE_API FSCDToPRSPlacementAdapter
{
public:
	static FPRSPlacementPlan BuildPlan(const FString& TargetProjectRoot, const TArray<FSCDPlacementMove>& Moves);

	/** Validate + dry-run diff for preview. False if PRS is unavailable or validation fails. */
	static bool Preview(
		const FPRSPlacementPlan& Plan,
		FPRSValidationReport& OutReport,
		FPRSDiff& OutDiff,
		FString& OutError);

	/** Apply only after SCD has shown the preview and the user confirmed. */
	static bool Apply(
		const FPRSPlacementPlan& Plan,
		const FPRSApplyOptions& Options,
		FPRSApplyResult& OutResult,
		FString& OutError);

	static bool Rollback(const FString& RunId, FPRSRollbackResult& OutResult, FString& OutError);
};
