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

#include "SCDHealthEvidenceToIISImporter.h"

#include "SCDHealthEvidenceIISBridgeCommon.h"
#include "SCDHealthEvidenceJsonlTranslator.h"
#include "HAL/FileManager.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"

bool FSCDHealthEvidenceToIISImporter::ImportHealthEvidenceJsonl(const FString& ScdJsonlPath, FString& OutError)
{
	if (!FPaths::FileExists(ScdJsonlPath))
	{
		OutError = FString::Printf(TEXT("JSONL not found: %s"), *ScdJsonlPath);
		return false;
	}

	const FString ImportRoot = FPaths::Combine(
		FPaths::ProjectSavedDir(),
		TEXT("SmartContentDiet"),
		TEXT("HealthIISImport"),
		FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens));
	const FString PreparedPath = FPaths::Combine(ImportRoot, TEXT("prepared_chunks.jsonl"));

	if (!SCDHealthEvidenceJsonlTranslator::TranslateScdHealthJsonlToPreparedJsonl(
			ScdJsonlPath, PreparedPath, OutError))
	{
		return false;
	}

	IInternalIndexService* IIS = SCDHealthEvidenceIISBridgeCommon::ResolveIIS();
	if (!IIS)
	{
		OutError = TEXT("Internal Index Service is not available (graceful no-op).");
		return false;
	}

	FString ImportReportPath;
	TArray<FString> Warnings;
	if (!IIS->ImportPreparedChunksJsonl(PreparedPath, ImportReportPath, Warnings))
	{
		if (Warnings.Num() > 0)
		{
			OutError = FString::Printf(TEXT("IIS rejected the health-evidence import: %s"), *Warnings[0]);
		}
		else
		{
			OutError = TEXT("IIS rejected the health-evidence import.");
		}
		return false;
	}

	return true;
}
