/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#pragma once

#include "IISVectorIndexBackend.h"

class FIISHnswVectorIndexBackend final : public IIISVectorIndexBackend
{
public:
	FIISHnswVectorIndexBackend();
	virtual ~FIISHnswVectorIndexBackend() override;

	virtual void Upsert(const FIISVectorRecordIn& Record) override;
	virtual void Remove(const FString& ChunkId) override;
	virtual bool Rebuild(const TArray<FIISVectorRecordIn>& All, TArray<FString>& OutWarnings) override;
	virtual TArray<FIISVectorHit> Search(const TArray<float>& Query, int32 K, TArray<FString>& OutWarnings) const override;
	virtual FString GetBackendId() const override { return TEXT("hnsw"); }

	bool ReadManifest(int32& OutCount, int32& OutDimensions, FString& OutRouteId, FString& OutModelId) const;

private:
	struct FIISHnswIndexState;

	mutable TArray<FIISVectorRecordIn> CachedRecords;
	mutable TUniquePtr<FIISHnswIndexState> ActiveIndex;
	mutable int32 IndexDimensions = 0;
	mutable FString IndexRouteId;
	mutable FString IndexModelId;
	mutable bool bIndexReady = false;

	static void NormalizeVector(TArray<float>& Vector);
	static FString GetVectorHnswDir();
	static FString GetIndexBinPath();
	static FString GetManifestPath();
	bool BuildIndexInMemory(const TArray<FIISVectorRecordIn>& All, TArray<FString>& OutWarnings) const;
	bool SaveIndexAndManifest(const TArray<FIISVectorRecordIn>& All, TArray<FString>& OutWarnings) const;
	bool LoadIndexFromDisk(TArray<FString>& OutWarnings) const;
};

TUniquePtr<IIISVectorIndexBackend> CreateHnswVectorIndexBackend();
