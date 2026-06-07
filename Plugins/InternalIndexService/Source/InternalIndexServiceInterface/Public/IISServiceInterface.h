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
#include "IISIndexTypes.h"
#include "IISImportTypes.h"
#include "IISSearchTypes.h"
#include "IISContextPackTypes.h"
#include "IISEmbeddingTypes.h"
#include "IISAgentAccessTypes.h"

class IInternalIndexService
{
public:
	virtual ~IInternalIndexService() = default;

	virtual bool IsAvailable() const = 0;
	virtual FString GetServiceVersion() const = 0;
	virtual FString GetDefaultIndexRoot() const = 0;

	virtual bool ImportPreparedChunksJsonl(
		const FString& PreparedChunksJsonlPath,
		FString& OutImportReportPath,
		TArray<FString>& OutWarnings
	) = 0;

	virtual bool Search(
		const FIISSearchQuery& Query,
		FIISSearchResponse& OutResponse
	) = 0;

	virtual bool BuildContextPack(
		const FIISSearchQuery& Query,
		FIISContextPack& OutContextPack
	) = 0;

	virtual bool BuildEmbeddingJobs(
		FString& OutReportPath,
		TArray<FString>& OutWarnings
	) = 0;

	virtual bool ExecuteEmbeddingJobs(
		int32 MaxJobs,
		FString& OutReportPath,
		TArray<FString>& OutWarnings
	) = 0;

	virtual bool ExecuteAgentTool(
		const FIISAgentToolRequest& Request,
		FIISAgentToolResponse& OutResponse
	) = 0;

	virtual bool GetChunk(
		const FString& ChunkId,
		FIISAgentToolResponse& OutResponse
	) = 0;

	virtual bool GetSourceReferences(
		const FString& ChunkId,
		FIISAgentToolResponse& OutResponse
	) = 0;

	virtual bool FindUsages(
		const FIISAgentToolRequest& Request,
		FIISAgentToolResponse& OutResponse
	) = 0;

	virtual bool ExplainBlueprint(
		const FIISAgentToolRequest& Request,
		FIISAgentToolResponse& OutResponse
	) = 0;
};
