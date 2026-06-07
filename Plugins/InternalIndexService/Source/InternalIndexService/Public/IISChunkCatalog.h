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
#include "IISCatalogTypes.h"
#include "IISContextPackTypes.h"
#include "IISIndexTypes.h"
#include "IISSearchTypes.h"
#include "IISVectorIndexBackend.h"

enum class EIISChunkUpsertAction : uint8
{
	Insert,
	NoOp,
	Conflict
};

struct FIISImportConflict
{
	FString ChunkId;
	FString OldSha;
	FString NewSha;
	FString SourceId;
	FString Policy;
};

class INTERNALINDEXSERVICE_API FIISChunkCatalog
{
public:
	static EIISChunkUpsertAction ClassifyUpsert(
		bool bExists,
		const FString& ExistingSha,
		const FString& NewSha);

	/** active or empty (legacy rows). */
	static bool IsActiveLifecycleState(const FString& LifecycleState);

	/** Chunks eligible for lexical/vector search (excludes stale and tombstoned). */
	static bool IsRetrievableLifecycleState(const FString& LifecycleState);

	static FString GetCatalogPath();

	static bool BuildOrUpdateCatalogFromChunkStore(
		FString& OutReportPath,
		TArray<FString>& OutWarnings);

	static bool BuildOrUpdateCatalogFromChunkStorePath(
		const FString& ChunkStoreJsonlPath,
		FString& OutReportPath,
		TArray<FString>& OutWarnings);

	static bool SearchCatalog(
		const FIISSearchQuery& Query,
		FIISSearchResponse& OutResponse);

	static bool LoadChunkById(
		const FString& ChunkId,
		FIISIndexChunk& OutChunk,
		TArray<FString>& OutWarnings);

	static bool WriteSearchReport(
		const FIISSearchQuery& Query,
		const FIISSearchResponse& Response,
		const FIISContextPack* ContextPack,
		FString& OutSearchReportPath);

	static bool WriteContextPackReport(
		const FIISContextPack& ContextPack,
		FString& OutContextPackReportPath);

#if WITH_DEV_AUTOMATION_TESTS
	/** Forces the next upsert of ChunkId to fail (transaction rollback tests). */
	static void SetForceUpsertFailureChunkIdForTest(const FString& ChunkId);

	static void ClearForceUpsertFailureChunkIdForTest();

	/** Runs catalog vector scoring via the configured index backend without LLM query embedding. */
	static bool SearchVectorDelegatedForTest(
		const FString& CatalogDatabasePath,
		const TArray<FIISVectorRecordIn>& VectorRecords,
		const TMap<FString, TArray<float>>& QueryVectorsByRoute,
		const FIISSearchQuery& Query,
		FIISSearchResponse& OutResponse);
#endif
};
