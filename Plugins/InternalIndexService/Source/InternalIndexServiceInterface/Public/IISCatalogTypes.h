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
#include "IISCatalogTypes.generated.h"

UENUM(BlueprintType)
enum class EIISCatalogStatus : uint8
{
	Unknown,
	Ready,
	Empty,
	Warning,
	Error
};

USTRUCT(BlueprintType)
struct INTERNALINDEXSERVICEINTERFACE_API FIISCatalogBuildSummary
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	EIISCatalogStatus Status = EIISCatalogStatus::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 SourceChunkCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 CatalogChunkCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 InsertedChunkCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 UpdatedChunkCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 SkippedChunkCount = 0;

	/** Chunks unchanged on re-import (same text_sha256); does not affect Warning status. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 UnchangedChunkCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 LabelCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 GroupCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 WarningCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 ErrorCount = 0;
};

USTRUCT(BlueprintType)
struct INTERNALINDEXSERVICEINTERFACE_API FIISCatalogBuildReport
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString SchemaVersion = TEXT("0.1.0");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString ToolName = TEXT("Internal Index Service");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString GeneratedAtUtc;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString CatalogPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString SourceChunkStorePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FIISCatalogBuildSummary Summary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FString> Warnings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FString> Errors;
};
