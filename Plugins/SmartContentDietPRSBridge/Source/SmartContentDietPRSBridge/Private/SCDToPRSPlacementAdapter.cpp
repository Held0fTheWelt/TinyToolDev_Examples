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

#include "SCDToPRSPlacementAdapter.h"

#include "Misc/Guid.h"
#include "PRSServiceInterface.h"
#include "PRSServiceRegistry.h"

namespace
{
	FString NewBridgeId(const TCHAR* Prefix)
	{
		return FString::Printf(TEXT("%s-%s"), Prefix, *FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens));
	}

	IProjectRestructureService* ResolvePRSService(FString& OutError)
	{
		IProjectRestructureService* Service = FPRSServiceRegistry::Resolve();
		if (!Service)
		{
			OutError = TEXT("Project Restructure Service is not available.");
		}
		return Service;
	}
}

FPRSPlacementPlan FSCDToPRSPlacementAdapter::BuildPlan(
	const FString& TargetProjectRoot,
	const TArray<FSCDPlacementMove>& Moves)
{
	FPRSPlacementPlan Plan;
	Plan.PlanId = NewBridgeId(TEXT("scd-prs-plan"));
	Plan.TargetProjectRoot = TargetProjectRoot;

	for (const FSCDPlacementMove& Move : Moves)
	{
		FPRSPlacementEntry Entry;
		Entry.EntryId = NewBridgeId(TEXT("scd-prs-entry"));
		Entry.EntryKind = EPRSEntryKind::AssetSet;
		Entry.Source.Add(Move.SourceAssetPath);
		Entry.Destination = Move.DestinationFolder;
		Entry.DestinationKind = EPRSDestinationKind::PluginDir;
		Entry.DecisionSource = EPRSDecisionSource::Manual;
		Entry.Confidence = 1.0f;
		Entry.Rationale = Move.Rationale;
		Plan.Entries.Add(Entry);
	}

	return Plan;
}

bool FSCDToPRSPlacementAdapter::Preview(
	const FPRSPlacementPlan& Plan,
	FPRSValidationReport& OutReport,
	FPRSDiff& OutDiff,
	FString& OutError)
{
	OutError.Reset();
	OutReport = FPRSValidationReport();
	OutDiff = FPRSDiff();

	IProjectRestructureService* Service = ResolvePRSService(OutError);
	if (!Service)
	{
		return false;
	}

	if (!Service->ValidatePlan(Plan, OutReport))
	{
		OutError = TEXT("Project Restructure Service rejected the placement plan during validation.");
		return false;
	}

	if (!Service->BuildDryRunDiff(Plan, OutDiff))
	{
		OutError = TEXT("Project Restructure Service could not build a dry-run diff.");
		return false;
	}

	return true;
}

bool FSCDToPRSPlacementAdapter::Apply(
	const FPRSPlacementPlan& Plan,
	const FPRSApplyOptions& Options,
	FPRSApplyResult& OutResult,
	FString& OutError)
{
	OutError.Reset();
	OutResult = FPRSApplyResult();

	IProjectRestructureService* Service = ResolvePRSService(OutError);
	if (!Service)
	{
		return false;
	}

	if (!Service->ApplyPlan(Plan, Options, OutResult))
	{
		OutError = TEXT("Project Restructure Service did not apply the placement plan.");
		return false;
	}

	return true;
}

bool FSCDToPRSPlacementAdapter::Rollback(
	const FString& RunId,
	FPRSRollbackResult& OutResult,
	FString& OutError)
{
	OutError.Reset();
	OutResult = FPRSRollbackResult();

	IProjectRestructureService* Service = ResolvePRSService(OutError);
	if (!Service)
	{
		return false;
	}

	if (!Service->Rollback(RunId, OutResult))
	{
		OutError = TEXT("Project Restructure Service rollback failed.");
		return false;
	}

	return true;
}
