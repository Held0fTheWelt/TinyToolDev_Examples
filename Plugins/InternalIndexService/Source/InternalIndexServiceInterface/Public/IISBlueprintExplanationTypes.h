/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#pragma once

#include "CoreMinimal.h"
#include "IISIndexTypes.h"
#include "IISBlueprintExplanationTypes.generated.h"

USTRUCT(BlueprintType)
struct INTERNALINDEXSERVICEINTERFACE_API FIISBlueprintGraphSummary
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString GraphName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString GraphKind;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	int32 NodeCount = 0;
};

USTRUCT(BlueprintType)
struct INTERNALINDEXSERVICEINTERFACE_API FIISBlueprintExplanation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString BlueprintName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString AssetPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	FString ParentClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FIISBlueprintGraphSummary> Graphs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FString> EventsAndFunctions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FString> Variables;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FString> Components;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FString> ReferencedAssets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FString> ReferencedClassesOrFunctions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FString> UnsupportedNodes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FString> NetworkAuthorityHints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FIISSourceReference> SourceReferences;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FString> Guardrails;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	bool bIRAvailable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Internal Index Service")
	TArray<FString> Warnings;
};
