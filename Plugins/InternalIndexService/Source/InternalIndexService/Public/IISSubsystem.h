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
#include "Subsystems/EngineSubsystem.h"
#include "IISServiceInterface.h"
#include "IISSubsystem.generated.h"

/**
 * Engine-subsystem discovery wrapper for IIS. Tooling or bridge plugins can
 * resolve IIS via GEngine->GetEngineSubsystem<UIISSubsystem>() and call the
 * public contract.
 *
 * This type owns NO service instance. Every method delegates to
 * FInternalIndexServiceModule::GetService(), which remains authoritative.
 */
UCLASS()
class INTERNALINDEXSERVICE_API UIISSubsystem : public UEngineSubsystem, public IInternalIndexService
{
	GENERATED_BODY()

public:
	virtual bool IsAvailable() const override;
	virtual FString GetServiceVersion() const override;
	virtual FString GetDefaultIndexRoot() const override;

	virtual bool ImportPreparedChunksJsonl(
		const FString& PreparedChunksJsonlPath,
		FString& OutImportReportPath,
		TArray<FString>& OutWarnings) override;

	virtual bool Search(
		const FIISSearchQuery& Query,
		FIISSearchResponse& OutResponse) override;

	virtual bool BuildContextPack(
		const FIISSearchQuery& Query,
		FIISContextPack& OutContextPack) override;

	virtual bool BuildEmbeddingJobs(
		FString& OutReportPath,
		TArray<FString>& OutWarnings) override;

	virtual bool ExecuteEmbeddingJobs(
		int32 MaxJobs,
		FString& OutReportPath,
		TArray<FString>& OutWarnings) override;

	virtual bool ExecuteAgentTool(
		const FIISAgentToolRequest& Request,
		FIISAgentToolResponse& OutResponse) override;

	virtual bool GetChunk(
		const FString& ChunkId,
		FIISAgentToolResponse& OutResponse) override;

	virtual bool GetSourceReferences(
		const FString& ChunkId,
		FIISAgentToolResponse& OutResponse) override;

	virtual bool FindUsages(
		const FIISAgentToolRequest& Request,
		FIISAgentToolResponse& OutResponse) override;

	virtual bool ExplainBlueprint(
		const FIISAgentToolRequest& Request,
		FIISAgentToolResponse& OutResponse) override;

private:
	static IInternalIndexService* ResolveService();
};
