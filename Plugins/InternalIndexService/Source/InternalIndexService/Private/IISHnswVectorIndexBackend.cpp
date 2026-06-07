/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#include "IISHnswVectorIndexBackend.h"

#include "IISStoragePaths.h"
#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

THIRD_PARTY_INCLUDES_START
#include "hnswlib.h"
THIRD_PARTY_INCLUDES_END

namespace
{
	constexpr int32 GHnswM = 16;
	constexpr int32 GHnswEfConstruction = 200;
	constexpr int32 GHnswEfSearch = 64;
}

struct FIISHnswVectorIndexBackend::FIISHnswIndexState
{
	TUniquePtr<hnswlib::InnerProductSpace> Space;
	TUniquePtr<hnswlib::HierarchicalNSW<float>> Index;
	TMap<size_t, FString> LabelToChunkId;
	TArray<FString> ChunkIdsInLabelOrder;
	int32 Dimensions = 0;
};

FIISHnswVectorIndexBackend::FIISHnswVectorIndexBackend() = default;

FIISHnswVectorIndexBackend::~FIISHnswVectorIndexBackend() = default;

TUniquePtr<IIISVectorIndexBackend> CreateHnswVectorIndexBackend()
{
	return MakeUnique<FIISHnswVectorIndexBackend>();
}

void FIISHnswVectorIndexBackend::NormalizeVector(TArray<float>& Vector)
{
	double NormSq = 0.0;
	for (const float Value : Vector)
	{
		NormSq += static_cast<double>(Value) * static_cast<double>(Value);
	}
	if (NormSq <= KINDA_SMALL_NUMBER)
	{
		return;
	}
	const float InvNorm = static_cast<float>(1.0 / FMath::Sqrt(NormSq));
	for (float& Value : Vector)
	{
		Value *= InvNorm;
	}
}

FString FIISHnswVectorIndexBackend::GetVectorHnswDir()
{
	return FIISStoragePaths::GetIndexesDir() / TEXT("vector_hnsw");
}

FString FIISHnswVectorIndexBackend::GetIndexBinPath()
{
	return GetVectorHnswDir() / TEXT("index.bin");
}

FString FIISHnswVectorIndexBackend::GetManifestPath()
{
	return GetVectorHnswDir() / TEXT("vector_index_manifest.json");
}

void FIISHnswVectorIndexBackend::Upsert(const FIISVectorRecordIn& Record)
{
	for (FIISVectorRecordIn& Existing : CachedRecords)
	{
		if (Existing.ChunkId == Record.ChunkId)
		{
			Existing = Record;
			bIndexReady = false;
			return;
		}
	}
	CachedRecords.Add(Record);
	bIndexReady = false;
}

void FIISHnswVectorIndexBackend::Remove(const FString& ChunkId)
{
	const int32 Removed = CachedRecords.RemoveAll([&ChunkId](const FIISVectorRecordIn& Record)
	{
		return Record.ChunkId == ChunkId;
	});
	if (Removed > 0)
	{
		bIndexReady = false;
		IFileManager::Get().Delete(*GetIndexBinPath());
		IFileManager::Get().Delete(*GetManifestPath());
	}
}

bool FIISHnswVectorIndexBackend::ReadManifest(
	int32& OutCount,
	int32& OutDimensions,
	FString& OutRouteId,
	FString& OutModelId) const
{
	OutCount = 0;
	OutDimensions = 0;
	OutRouteId.Reset();
	OutModelId.Reset();

	FString JsonText;
	if (!FFileHelper::LoadFileToString(JsonText, *GetManifestPath()))
	{
		return false;
	}

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		return false;
	}

	double CountValue = 0.0;
	double DimensionsValue = 0.0;
	Root->TryGetNumberField(TEXT("count"), CountValue);
	Root->TryGetNumberField(TEXT("dimensions"), DimensionsValue);
	Root->TryGetStringField(TEXT("route_id"), OutRouteId);
	Root->TryGetStringField(TEXT("model_id"), OutModelId);
	OutCount = static_cast<int32>(CountValue);
	OutDimensions = static_cast<int32>(DimensionsValue);
	return true;
}

bool FIISHnswVectorIndexBackend::BuildIndexInMemory(
	const TArray<FIISVectorRecordIn>& All,
	TArray<FString>& OutWarnings) const
{
	ActiveIndex.Reset();

	if (All.Num() == 0)
	{
		IndexDimensions = 0;
		IndexRouteId.Reset();
		IndexModelId.Reset();
		bIndexReady = true;
		return true;
	}

	const int32 Dimensions = All[0].Dimensions > 0 ? All[0].Dimensions : All[0].Vector.Num();
	IndexRouteId = All[0].RouteId;
	IndexModelId = All[0].ModelId;
	IndexDimensions = Dimensions;

	TUniquePtr<FIISHnswIndexState> State = MakeUnique<FIISHnswIndexState>();
	State->Dimensions = Dimensions;
	State->Space = MakeUnique<hnswlib::InnerProductSpace>(static_cast<size_t>(Dimensions));
	State->Index = MakeUnique<hnswlib::HierarchicalNSW<float>>(
		State->Space.Get(),
		static_cast<size_t>(All.Num()),
		GHnswM,
		GHnswEfConstruction);
	State->Index->setEf(GHnswEfSearch);

	size_t NextLabel = 0;
	for (const FIISVectorRecordIn& Record : All)
	{
		const int32 RecordDims = Record.Dimensions > 0 ? Record.Dimensions : Record.Vector.Num();
		if (RecordDims != Dimensions)
		{
			OutWarnings.Add(FString::Printf(
				TEXT("Skipping chunk '%s' during HNSW rebuild: expected %d dimensions, got %d."),
				*Record.ChunkId,
				Dimensions,
				RecordDims));
			continue;
		}

		TArray<float> Normalized = Record.Vector;
		NormalizeVector(Normalized);
		State->Index->addPoint(Normalized.GetData(), NextLabel);
		State->LabelToChunkId.Add(NextLabel, Record.ChunkId);
		State->ChunkIdsInLabelOrder.Add(Record.ChunkId);
		++NextLabel;
	}

	if (NextLabel == 0)
	{
		OutWarnings.Add(TEXT("HNSW rebuild produced an empty index."));
		bIndexReady = false;
		return false;
	}

	ActiveIndex = MoveTemp(State);
	bIndexReady = true;
	return true;
}

bool FIISHnswVectorIndexBackend::SaveIndexAndManifest(
	const TArray<FIISVectorRecordIn>& All,
	TArray<FString>& OutWarnings) const
{
	if (!ActiveIndex.IsValid() || !ActiveIndex->Index.IsValid())
	{
		return true;
	}

	IFileManager::Get().MakeDirectory(*GetVectorHnswDir(), true);
	ActiveIndex->Index->saveIndex(TCHAR_TO_UTF8(*GetIndexBinPath()));

	TSharedRef<FJsonObject> Manifest = MakeShared<FJsonObject>();
	Manifest->SetNumberField(TEXT("count"), All.Num());
	Manifest->SetNumberField(TEXT("dimensions"), IndexDimensions);
	Manifest->SetStringField(TEXT("route_id"), IndexRouteId);
	Manifest->SetStringField(TEXT("model_id"), IndexModelId);
	Manifest->SetStringField(TEXT("built_at"), FDateTime::UtcNow().ToIso8601());

	TArray<TSharedPtr<FJsonValue>> ChunkIdValues;
	if (ActiveIndex.IsValid())
	{
		for (const FString& ChunkId : ActiveIndex->ChunkIdsInLabelOrder)
		{
			ChunkIdValues.Add(MakeShared<FJsonValueString>(ChunkId));
		}
	}
	Manifest->SetArrayField(TEXT("chunk_ids"), ChunkIdValues);

	FString ManifestJson;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ManifestJson);
	if (!FJsonSerializer::Serialize(Manifest, Writer))
	{
		OutWarnings.Add(TEXT("Failed to serialize HNSW vector index manifest."));
		return false;
	}

	if (!FFileHelper::SaveStringToFile(ManifestJson, *GetManifestPath()))
	{
		OutWarnings.Add(FString::Printf(TEXT("Failed to write HNSW manifest: %s"), *GetManifestPath()));
		return false;
	}

	return true;
}

bool FIISHnswVectorIndexBackend::LoadIndexFromDisk(TArray<FString>& OutWarnings) const
{
	if (!FPaths::FileExists(GetIndexBinPath()))
	{
		return false;
	}

	FString ManifestJson;
	if (!FFileHelper::LoadFileToString(ManifestJson, *GetManifestPath()))
	{
		OutWarnings.Add(TEXT("HNSW index.bin exists but manifest is missing or invalid."));
		return false;
	}

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ManifestJson);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		OutWarnings.Add(TEXT("HNSW manifest is malformed."));
		return false;
	}

	double CountValue = 0.0;
	double DimensionsValue = 0.0;
	Root->TryGetNumberField(TEXT("count"), CountValue);
	Root->TryGetNumberField(TEXT("dimensions"), DimensionsValue);
	FString ManifestRouteId;
	FString ManifestModelId;
	Root->TryGetStringField(TEXT("route_id"), ManifestRouteId);
	Root->TryGetStringField(TEXT("model_id"), ManifestModelId);
	const int32 ManifestCount = static_cast<int32>(CountValue);
	const int32 ManifestDimensions = static_cast<int32>(DimensionsValue);

	if (ManifestDimensions <= 0 || ManifestCount <= 0)
	{
		OutWarnings.Add(TEXT("HNSW manifest has invalid count or dimensions."));
		return false;
	}

	TUniquePtr<FIISHnswIndexState> State = MakeUnique<FIISHnswIndexState>();
	State->Dimensions = ManifestDimensions;
	State->Space = MakeUnique<hnswlib::InnerProductSpace>(static_cast<size_t>(ManifestDimensions));
	State->Index = MakeUnique<hnswlib::HierarchicalNSW<float>>(
		State->Space.Get(),
		TCHAR_TO_UTF8(*GetIndexBinPath()));
	State->Index->setEf(GHnswEfSearch);

	const TArray<TSharedPtr<FJsonValue>>* ChunkIdValues = nullptr;
	if (Root->TryGetArrayField(TEXT("chunk_ids"), ChunkIdValues) && ChunkIdValues)
	{
		size_t Label = 0;
		for (const TSharedPtr<FJsonValue>& Value : *ChunkIdValues)
		{
			FString ChunkId;
			if (Value.IsValid() && Value->TryGetString(ChunkId))
			{
				State->LabelToChunkId.Add(Label, ChunkId);
				State->ChunkIdsInLabelOrder.Add(ChunkId);
				++Label;
			}
		}
	}

	IndexDimensions = ManifestDimensions;
	IndexRouteId = ManifestRouteId;
	IndexModelId = ManifestModelId;
	ActiveIndex = MoveTemp(State);
	bIndexReady = true;
	return true;
}

bool FIISHnswVectorIndexBackend::Rebuild(const TArray<FIISVectorRecordIn>& All, TArray<FString>& OutWarnings)
{
	CachedRecords = All;
	if (!BuildIndexInMemory(All, OutWarnings))
	{
		return false;
	}
	return SaveIndexAndManifest(All, OutWarnings);
}

TArray<FIISVectorHit> FIISHnswVectorIndexBackend::Search(
	const TArray<float>& Query,
	const int32 K,
	TArray<FString>& OutWarnings) const
{
	TArray<FIISVectorHit> Hits;
	if (K <= 0 || Query.Num() == 0)
	{
		return Hits;
	}

	if (!bIndexReady || !ActiveIndex.IsValid())
	{
		if (!LoadIndexFromDisk(OutWarnings))
		{
			if (CachedRecords.Num() > 0)
			{
				TArray<FString> RebuildWarnings;
				const_cast<FIISHnswVectorIndexBackend*>(this)->Rebuild(CachedRecords, RebuildWarnings);
				OutWarnings.Append(RebuildWarnings);
			}
			else
			{
				OutWarnings.Add(TEXT("HNSW vector index is not built; call Rebuild first."));
				return Hits;
			}
		}
	}

	if (!ActiveIndex.IsValid() || !ActiveIndex->Index.IsValid())
	{
		OutWarnings.Add(TEXT("HNSW vector index is unavailable."));
		return Hits;
	}

	if (Query.Num() != ActiveIndex->Dimensions)
	{
		OutWarnings.Add(FString::Printf(
			TEXT("HNSW query dimension mismatch: query has %d dimensions, index has %d."),
			Query.Num(),
			ActiveIndex->Dimensions));
		return Hits;
	}

	TArray<float> NormalizedQuery = Query;
	NormalizeVector(NormalizedQuery);

	const size_t QueryK = static_cast<size_t>(FMath::Min(K, static_cast<int32>(ActiveIndex->LabelToChunkId.Num())));
	if (QueryK == 0)
	{
		return Hits;
	}

	auto KnnResults = ActiveIndex->Index->searchKnn(NormalizedQuery.GetData(), QueryK);

	while (!KnnResults.empty())
	{
		const std::pair<float, hnswlib::labeltype> Top = KnnResults.top();
		KnnResults.pop();

		const FString* ChunkId = ActiveIndex->LabelToChunkId.Find(Top.second);
		if (!ChunkId)
		{
			continue;
		}

		FIISVectorHit Hit;
		Hit.ChunkId = *ChunkId;
		Hit.Score = 1.0f - Top.first;
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

	return Hits;
}
