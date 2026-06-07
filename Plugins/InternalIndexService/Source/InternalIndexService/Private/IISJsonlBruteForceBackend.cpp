/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#include "IISJsonlBruteForceBackend.h"

float FIISVectorIndexMath::ComputeCosineSimilarity(const TArray<float>& Left, const TArray<float>& Right)
{
	const int32 Count = FMath::Min(Left.Num(), Right.Num());
	if (Count <= 0)
	{
		return 0.0f;
	}

	double Dot = 0.0;
	double LeftNorm = 0.0;
	double RightNorm = 0.0;
	for (int32 Index = 0; Index < Count; ++Index)
	{
		const double L = static_cast<double>(Left[Index]);
		const double R = static_cast<double>(Right[Index]);
		Dot += L * R;
		LeftNorm += L * L;
		RightNorm += R * R;
	}

	if (LeftNorm <= KINDA_SMALL_NUMBER || RightNorm <= KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	return static_cast<float>(Dot / FMath::Sqrt(LeftNorm * RightNorm));
}

void FIISJsonlBruteForceBackend::Upsert(const FIISVectorRecordIn& Record)
{
	for (FIISVectorRecordIn& Existing : Records)
	{
		if (Existing.ChunkId == Record.ChunkId)
		{
			Existing = Record;
			return;
		}
	}
	Records.Add(Record);
}

void FIISJsonlBruteForceBackend::Remove(const FString& ChunkId)
{
	Records.RemoveAll([&ChunkId](const FIISVectorRecordIn& Record)
	{
		return Record.ChunkId == ChunkId;
	});
}

bool FIISJsonlBruteForceBackend::Rebuild(const TArray<FIISVectorRecordIn>& All, TArray<FString>& OutWarnings)
{
	Records = All;
	return true;
}

TArray<FIISVectorHit> FIISJsonlBruteForceBackend::Search(
	const TArray<float>& Query,
	const int32 K,
	TArray<FString>& OutWarnings) const
{
	TArray<FIISVectorHit> Hits;
	if (K <= 0 || Query.Num() == 0)
	{
		return Hits;
	}

	for (const FIISVectorRecordIn& Record : Records)
	{
		if (Record.Vector.Num() != Query.Num())
		{
			continue;
		}

		FIISVectorHit Hit;
		Hit.ChunkId = Record.ChunkId;
		Hit.Score = FIISVectorIndexMath::ComputeCosineSimilarity(Query, Record.Vector);
		Hits.Add(Hit);
	}

	Hits.Sort([](const FIISVectorHit& Left, const FIISVectorHit& Right)
	{
		if (!FMath::IsNearlyEqual(Left.Score, Right.Score))
		{
			return Left.Score > Right.Score;
		}
		return Left.ChunkId < Right.ChunkId;
	});

	if (Hits.Num() > K)
	{
		Hits.SetNum(K);
	}

	return Hits;
}
