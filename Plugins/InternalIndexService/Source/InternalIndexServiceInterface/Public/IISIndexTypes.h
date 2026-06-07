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
#include "IISIndexTypes.generated.h"

UENUM(BlueprintType)
enum class EIISChunkKind : uint8
{
	Unknown,
	Code,
	Blueprint,
	Asset,
	Module,
	Reflection,
	Network,
	ArchitectureFinding,
	CandidateReview,
	PlanningIntake,
	RagPreparedChunk,
	Documentation,
	Guardrail
};

UENUM(BlueprintType)
enum class EIISChunkSensitivity : uint8
{
	Unknown,
	PublicProductSafe,
	ProjectLocal,
	SourceEvidence,
	PrivateReview,
	Restricted
};

UENUM(BlueprintType)
enum class EIISIndexSourceKind : uint8
{
	Unknown,
	EvidenceRun,
	RagExport,
	PreparedChunks,
	ManualJsonl,
	External
};

USTRUCT(BlueprintType)
struct INTERNALINDEXSERVICEINTERFACE_API FIISSourceReference
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString ArtifactKind;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString RelativePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString JsonPointer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString Fingerprint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString Explanation;
};

USTRUCT(BlueprintType)
struct INTERNALINDEXSERVICEINTERFACE_API FIISIndexChunk
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString ChunkId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString SourceChunkId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	EIISChunkKind ChunkKind = EIISChunkKind::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	EIISChunkSensitivity Sensitivity = EIISChunkSensitivity::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString ModuleName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString SourceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString SourceRunId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString DestinationRunId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FString> RetrievalLabels;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FString> RetrievalGroups;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FIISSourceReference> SourceReferences;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	bool bIsAiGenerated = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	bool bAllowsMigrationDecision = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	bool bAllowsPatchGeneration = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString TextSha256;

	/** active | stale | tombstoned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString LifecycleState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString SourceRefsHash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FString> Warnings;
};

USTRUCT(BlueprintType)
struct INTERNALINDEXSERVICEINTERFACE_API FIISIndexSymbol
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString SymbolId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString QualifiedName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString SymbolKind;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString ModuleName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString RelativePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 LineNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FString> Tags;
};

USTRUCT(BlueprintType)
struct INTERNALINDEXSERVICEINTERFACE_API FIISBlueprintGraphRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString AssetPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString BlueprintName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString ParentClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 GraphCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 NodeCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 PinCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 LinkCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FString> UnsupportedNodeClasses;
};

USTRUCT(BlueprintType)
struct INTERNALINDEXSERVICEINTERFACE_API FIISAssetRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString ObjectPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString PackageName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString PackagePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString AssetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString AssetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FString> Tags;
};
