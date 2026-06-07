/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA
 *
 * This file is part of the "Internal Index Service" Unreal Engine plugin.
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
#include "Kismet/BlueprintFunctionLibrary.h"
#include "IISPythonBridge.generated.h"

UCLASS()
class INTERNALINDEXSERVICEEDITOR_API UIISPythonBridge : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Internal Index Service")
	static FString GetIISVersion();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Internal Index Service")
	static FString GetDefaultIndexRoot();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Internal Index Service")
	static bool IsIISAvailable();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Internal Index Service")
	static bool SmokeTestService(FString& OutReportPath);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Internal Index Service")
	static bool ImportPreparedChunksJsonl(
		const FString& PreparedChunksJsonlPath,
		FString& OutImportReportPath);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Internal Index Service")
	static bool ImportPreparedChunksJsonlWithWarnings(
		const FString& PreparedChunksJsonlPath,
		FString& OutImportReportPath,
		TArray<FString>& OutWarnings);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Internal Index Service")
	static bool BuildChunkCatalog(FString& OutReportPath);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Internal Index Service")
	static bool BuildChunkCatalogWithWarnings(
		FString& OutReportPath,
		TArray<FString>& OutWarnings);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Internal Index Service")
	static bool SearchChunksLexical(
		const FString& QueryText,
		int32 MaxResults,
		FString& OutSearchReportPath);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Internal Index Service")
	static bool SearchChunksVector(
		const FString& QueryText,
		int32 MaxResults,
		FString& OutSearchReportPath);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Internal Index Service")
	static bool SearchChunksHybrid(
		const FString& QueryText,
		int32 MaxResults,
		FString& OutSearchReportPath);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Internal Index Service")
	static bool SearchChunksWithMode(
		const FString& QueryText,
		const FString& SearchMode,
		int32 MaxResults,
		FString& OutSearchReportPath);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Internal Index Service")
	static bool SearchChunksWithFilters(
		const FString& QueryText,
		const FString& SearchMode,
		int32 MaxResults,
		const TArray<FString>& RequiredLabels,
		const TArray<FString>& PreferredGroups,
		const TArray<FString>& ExcludedSensitivities,
		FString& OutSearchReportPath);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Internal Index Service")
	static bool BuildContextPackForQuery(
		const FString& QueryText,
		const FString& SearchMode,
		int32 MaxResults,
		FString& OutContextPackReportPath);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Internal Index Service")
	static bool BuildEmbeddingJobs(FString& OutReportPath);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Internal Index Service")
	static bool ExecuteEmbeddingJobs(int32 MaxJobs, FString& OutReportPath);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Internal Index Service")
	static bool BuildAndExecuteEmbeddingJobs(int32 MaxJobs, FString& OutReportPath);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Internal Index Service")
	static bool ExecuteEmbeddingJobsWithWarnings(
		int32 MaxJobs,
		FString& OutReportPath,
		TArray<FString>& OutWarnings);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Internal Index Service|Agent")
	static bool AgentSearch(
		const FString& QueryText,
		const FString& SearchMode,
		int32 MaxResults,
		FString& OutResponsePath);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Internal Index Service|Agent")
	static bool AgentGetContextPack(
		const FString& QueryText,
		const FString& SearchMode,
		int32 MaxResults,
		FString& OutResponsePath);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Internal Index Service|Agent")
	static bool AgentGetChunk(
		const FString& ChunkId,
		FString& OutResponsePath);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Internal Index Service|Agent")
	static bool AgentGetSourceReferences(
		const FString& ChunkId,
		FString& OutResponsePath);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Internal Index Service|Agent")
	static bool AgentFindUsages(
		const FString& SymbolName,
		FString& OutResponsePath);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Internal Index Service|Agent")
	static bool AgentExplainBlueprint(
		const FString& AssetPathOrQuery,
		FString& OutResponsePath);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Internal Index Service|Agent")
	static bool WriteAgentToolContracts(FString& OutContractsPath);

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Internal Index Service|Agent")
	static bool ExecuteAgentToolFromJson(
		const FString& InputJsonPath,
		FString& OutResponseJsonPath);
};
