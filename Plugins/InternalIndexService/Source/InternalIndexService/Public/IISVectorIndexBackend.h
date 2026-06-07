/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#pragma once

#include "CoreMinimal.h"

struct FIISVectorHit
{
	FString ChunkId;
	float Score = 0.f;
};

struct FIISVectorRecordIn
{
	FString ChunkId;
	FString RouteId;
	FString ModelId;
	FString TextSha256;
	int32 Dimensions = 0;
	TArray<float> Vector;
};

class IIISVectorIndexBackend
{
public:
	virtual ~IIISVectorIndexBackend() = default;

	virtual void Upsert(const FIISVectorRecordIn& Record) = 0;
	virtual void Remove(const FString& ChunkId) = 0;
	virtual bool Rebuild(const TArray<FIISVectorRecordIn>& All, TArray<FString>& OutWarnings) = 0;
	virtual TArray<FIISVectorHit> Search(const TArray<float>& Query, int32 K, TArray<FString>& OutWarnings) const = 0;
	virtual FString GetBackendId() const = 0;
};

class INTERNALINDEXSERVICE_API FIISVectorIndexMath
{
public:
	static float ComputeCosineSimilarity(const TArray<float>& Left, const TArray<float>& Right);
};

/** Returns the configured vector backend id (lowercased) from UIISSettings. */
INTERNALINDEXSERVICE_API FString IISResolveConfiguredVectorBackendId();

TUniquePtr<IIISVectorIndexBackend> CreateVectorIndexBackend();
