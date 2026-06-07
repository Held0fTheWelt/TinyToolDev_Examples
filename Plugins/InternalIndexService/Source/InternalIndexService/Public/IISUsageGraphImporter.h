/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#pragma once

#include "CoreMinimal.h"
#include "IISUsageGraphTypes.h"

struct FResolvedUsageEvidencePaths
{
	FString EvidenceRoot;
	FString CppSymbolIndexPath;
	FString ReflectionSurfaceIndexPath;
	FString NetworkSurfaceIndexPath;
	FString AssetRegistryExportPath;
	FString ModuleGraphPath;
	FString BlueprintsDirectory;
};

class INTERNALINDEXSERVICE_API FIISUsageGraphImporter
{
public:
	static FString DeriveSymbolId(
		const FString& QualifiedName,
		const FString& Name,
		const FString& ModuleName,
		int32 LineNumber);

	static bool ResolveEvidencePathsFromHandoffContract(
		const FString& HandoffContractPath,
		FResolvedUsageEvidencePaths& OutPaths,
		TArray<FString>& OutWarnings);

	static bool ParseCppSymbolIndexJson(
		const FString& Json,
		TArray<FIISSymbolRecord>& OutSymbols,
		TArray<FIISUsageRecord>& OutUsages,
		TArray<FString>& OutWarnings);

	static bool ParseReflectionSurfaceIndexJson(
		const FString& Json,
		TArray<FIISSymbolRecord>& OutSymbols,
		TArray<FIISUsageRecord>& OutUsages,
		TArray<FString>& OutWarnings);

	static bool ParseNetworkSurfaceIndexJson(
		const FString& Json,
		TArray<FIISSymbolRecord>& OutSymbols,
		TArray<FIISCallEdge>& OutCallEdges,
		TArray<FString>& OutWarnings);

	static bool ParseAssetRegistryExportJson(
		const FString& Json,
		TArray<FIISAssetReference>& OutAssetRefs,
		TArray<FString>& OutWarnings);

	static bool ParseBlueprintIrJson(
		const FString& Json,
		const FString& BlueprintName,
		TArray<FIISBlueprintReference>& OutBlueprintRefs,
		TArray<FString>& OutWarnings);

	static bool ImportFromHandoff(
		const FString& HandoffContractPath,
		TArray<FString>& OutWarnings);

	/** For automation tests: import an in-memory graph payload into the catalog DB. */
	static bool ImportGraphPayloadForTest(
		const TArray<FIISSymbolRecord>& Symbols,
		const TArray<FIISUsageRecord>& Usages,
		const TArray<FIISCallEdge>& CallEdges,
		const TArray<FIISAssetReference>& AssetRefs,
		const TArray<FIISBlueprintReference>& BlueprintRefs,
		TArray<FString>& OutWarnings);

	static bool QueryUsages(const FString& Query, FIISUsageQueryResult& OutResult);
};
