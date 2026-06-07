/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#pragma once

#include "CoreMinimal.h"
#include "IISBlueprintExplanationTypes.h"

class INTERNALINDEXSERVICE_API FIISBlueprintExplanationService
{
public:
	/** Deterministic assembly from a Blueprint-IR JSON document. No LLM. */
	static bool AssembleFromIRJson(
		const FString& IRJson,
		const FString& SourceFilePath,
		FIISBlueprintExplanation& Out);

	/** Resolve and load Blueprint IR JSON for an asset path or blueprint name query. */
	static bool TryLoadBlueprintIRJson(
		const FString& AssetPathOrQuery,
		FString& OutIRJson,
		FString& OutSourceFilePath,
		TArray<FString>& OutWarnings);
};
