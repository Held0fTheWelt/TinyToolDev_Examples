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
#include "IISEmbeddingTypes.generated.h"

UENUM(BlueprintType)
enum class EIISEmbeddingJobStatus : uint8
{
	Unknown,
	Pending,
	Running,
	Completed,
	Skipped,
	Failed,
	BlockedByGovernance
};

UENUM(BlueprintType)
enum class EIISEmbeddingTaskKind : uint8
{
	Unknown,
	Code,
	Blueprint,
	Asset,
	Documentation,
	Review,
	Planning,
	Generic
};

USTRUCT(BlueprintType)
struct INTERNALINDEXSERVICEINTERFACE_API FIISEmbeddingRoute
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString RouteId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString TaskKind;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString ProviderId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString ModelId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 Dimensions = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	bool bLocalOnly = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	bool bAllowFallback = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FString> FallbackRouteIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FString> AllowedRuntimeModes;
};

USTRUCT(BlueprintType)
struct INTERNALINDEXSERVICEINTERFACE_API FIISEmbeddingRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString RouteId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString TaskKind;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString InputText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TMap<FString, FString> Metadata;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 RequestedDimensions = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	bool bNormalize = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	bool bLocalOnly = true;
};

USTRUCT(BlueprintType)
struct INTERNALINDEXSERVICEINTERFACE_API FIISEmbeddingResponse
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString RouteId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString ProviderId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString ModelId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<float> Vector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 Dimensions = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString ErrorCode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString ErrorMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TMap<FString, FString> Metadata;
};

class INTERNALINDEXSERVICEINTERFACE_API IIISEmbeddingRouteExecutor
{
public:
	virtual ~IIISEmbeddingRouteExecutor() = default;

	virtual FString GetExecutorId() const = 0;

	virtual bool ResolveEmbeddingRoute(
		const FString& TaskKind,
		FIISEmbeddingRoute& OutRoute,
		TArray<FString>& OutWarnings) = 0;

	virtual bool ExecuteEmbeddingRoute(
		const FIISEmbeddingRequest& Request,
		FIISEmbeddingResponse& OutResponse) = 0;
};

class INTERNALINDEXSERVICEINTERFACE_API FIISEmbeddingRouteExecutorRegistry
{
public:
	static bool RegisterExecutor(TSharedRef<IIISEmbeddingRouteExecutor> Executor);
	static bool UnregisterExecutor(const FString& ExecutorId);
	static TSharedPtr<IIISEmbeddingRouteExecutor> GetExecutor(const FString& PreferredExecutorId = FString());
	static TArray<FString> GetExecutorIds();

	static bool ResolveEmbeddingRoute(
		const FString& TaskKind,
		FIISEmbeddingRoute& OutRoute,
		TArray<FString>& OutWarnings,
		const FString& PreferredExecutorId = FString());

	static bool ExecuteEmbeddingRoute(
		const FIISEmbeddingRequest& Request,
		FIISEmbeddingResponse& OutResponse,
		const FString& PreferredExecutorId = FString());
};

USTRUCT(BlueprintType)
struct INTERNALINDEXSERVICEINTERFACE_API FIISEmbeddingJob
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString JobId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString ChunkId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString RouteId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString TaskKind;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	EIISEmbeddingJobStatus Status = EIISEmbeddingJobStatus::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString ProviderId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString ModelId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 Dimensions = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString ErrorCode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString ErrorMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString CreatedAtUtc;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString UpdatedAtUtc;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString TextSha256;
};

USTRUCT(BlueprintType)
struct INTERNALINDEXSERVICEINTERFACE_API FIISEmbeddingRunSummary
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 ChunkCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 JobCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 CompletedCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 SkippedCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 FailedCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 BlockedByGovernanceCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 VectorCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 WarningCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 ErrorCount = 0;
};

USTRUCT(BlueprintType)
struct INTERNALINDEXSERVICEINTERFACE_API FIISEmbeddingRunReport
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString SchemaVersion = TEXT("0.1.0");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString ToolName = TEXT("Internal Index Service");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString GeneratedAtUtc;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString RunId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString CatalogPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString VectorStorePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FIISEmbeddingRunSummary Summary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FIISEmbeddingJob> Jobs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FString> Warnings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FString> Errors;
};
