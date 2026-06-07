/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#pragma once

#include "IISVectorIndexBackend.h"

class FIISJsonlBruteForceBackend final : public IIISVectorIndexBackend
{
public:
	virtual void Upsert(const FIISVectorRecordIn& Record) override;
	virtual void Remove(const FString& ChunkId) override;
	virtual bool Rebuild(const TArray<FIISVectorRecordIn>& All, TArray<FString>& OutWarnings) override;
	virtual TArray<FIISVectorHit> Search(const TArray<float>& Query, int32 K, TArray<FString>& OutWarnings) const override;
	virtual FString GetBackendId() const override { return TEXT("jsonl_bruteforce"); }

private:
	TArray<FIISVectorRecordIn> Records;
};
