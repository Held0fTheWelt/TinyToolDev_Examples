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
#include "IISContextPackTypes.h"
#include "IISIndexTypes.h"
#include "IISSearchTypes.h"
#include "IISBlueprintExplanationTypes.h"
#include "IISUsageGraphTypes.h"
#include "IISAgentAccessTypes.generated.h"

UENUM(BlueprintType)
enum class EIISAgentToolStatus : uint8
{
	Unknown,
	Ready,
	Empty,
	Warning,
	Error,
	Forbidden
};

UENUM(BlueprintType)
enum class EIISAgentToolKind : uint8
{
	Unknown,
	Search,
	GetContextPack,
	GetChunk,
	GetSourceReferences,
	FindUsages,
	ExplainBlueprint
};

USTRUCT(BlueprintType)
struct INTERNALINDEXSERVICEINTERFACE_API FIISAgentGuardrail
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString GuardrailId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString Severity;
};

USTRUCT(BlueprintType)
struct INTERNALINDEXSERVICEINTERFACE_API FIISAgentToolRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	EIISAgentToolKind ToolKind = EIISAgentToolKind::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString QueryText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString ChunkId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString AssetPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString SymbolName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	EIISSearchMode SearchMode = EIISSearchMode::Hybrid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 MaxResults = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FString> RequiredLabels;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FString> PreferredGroups;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FString> ExcludedSensitivities;
};

USTRUCT(BlueprintType)
struct INTERNALINDEXSERVICEINTERFACE_API FIISAgentToolResponse
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
	EIISAgentToolKind ToolKind = EIISAgentToolKind::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	EIISAgentToolStatus Status = EIISAgentToolStatus::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FIISAgentToolRequest Request;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FIISSearchResponse SearchResponse;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FIISContextPack ContextPack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FIISIndexChunk> Chunks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FIISSourceReference> SourceReferences;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FIISAgentGuardrail> Guardrails;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	bool bAllowsMigrationDecision = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	bool bAllowsPatchGeneration = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	bool bAllowsProjectMutation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FString> Warnings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FString> Errors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FIISUsageQueryResult UsageGraph;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FIISBlueprintExplanation BlueprintExplanation;
};
