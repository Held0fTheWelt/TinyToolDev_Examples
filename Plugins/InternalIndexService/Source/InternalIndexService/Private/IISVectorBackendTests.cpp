/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "IISJsonlBruteForceBackend.h"
#include "IISHnswVectorIndexBackend.h"
#include "IISChunkCatalog.h"
#include "IISSettings.h"
#include "IISVectorIndexBackend.h"
#include "IISSearchTypes.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISBruteForceTopK,
	"InternalIndexService.Vector.BruteForceTopK",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISBruteForceTopK::RunTest(const FString& Parameters)
{
	FIISJsonlBruteForceBackend Backend;
	TArray<FString> Warnings;

	FIISVectorRecordIn RecordA;
	RecordA.ChunkId = TEXT("a");
	RecordA.RouteId = TEXT("r");
	RecordA.ModelId = TEXT("m");
	RecordA.Dimensions = 2;
	RecordA.Vector = {1.f, 0.f};

	FIISVectorRecordIn RecordB;
	RecordB.ChunkId = TEXT("b");
	RecordB.RouteId = TEXT("r");
	RecordB.ModelId = TEXT("m");
	RecordB.Dimensions = 2;
	RecordB.Vector = {0.f, 1.f};

	Backend.Rebuild({RecordA, RecordB}, Warnings);
	const TArray<FIISVectorHit> Hits = Backend.Search({1.f, 0.f}, 1, Warnings);
	TestEqual(TEXT("one hit"), Hits.Num(), 1);
	TestEqual(TEXT("nearest is a"), Hits[0].ChunkId, FString(TEXT("a")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISVectorBackendDelegationParity,
	"InternalIndexService.Vector.DelegationParity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISVectorBackendDelegationParity::RunTest(const FString& Parameters)
{
	const FString FixturePath = FPaths::ProjectSavedDir()
		/ TEXT("IISValidation")
		/ TEXT("vector_delegation_parity_fixture.jsonl");

	IFileManager::Get().MakeDirectory(*FPaths::GetPath(FixturePath), true);

	const FString ChunkLineA = TEXT(
		"{\"chunk_id\":\"a\",\"chunk_kind\":\"code\",\"title\":\"Chunk A\","
		"\"text\":\"alpha\",\"source_id\":\"src-vector-parity\",\"source_run_id\":\"run-1\","
		"\"destination_run_id\":\"dest-1\",\"retrieval_labels\":[],\"retrieval_groups\":[],"
		"\"source_references\":[]}");
	const FString ChunkLineB = TEXT(
		"{\"chunk_id\":\"b\",\"chunk_kind\":\"code\",\"title\":\"Chunk B\","
		"\"text\":\"beta\",\"source_id\":\"src-vector-parity\",\"source_run_id\":\"run-1\","
		"\"destination_run_id\":\"dest-1\",\"retrieval_labels\":[],\"retrieval_groups\":[],"
		"\"source_references\":[]}");

	TestTrue(TEXT("write fixture"), FFileHelper::SaveStringToFile(ChunkLineA + LINE_TERMINATOR + ChunkLineB, *FixturePath));

	FString ReportPath;
	TArray<FString> Warnings;
	TestTrue(TEXT("build catalog"), FIISChunkCatalog::BuildOrUpdateCatalogFromChunkStorePath(FixturePath, ReportPath, Warnings));

	FIISVectorRecordIn RecordA;
	RecordA.ChunkId = TEXT("a");
	RecordA.RouteId = TEXT("r");
	RecordA.ModelId = TEXT("m");
	RecordA.Dimensions = 2;
	RecordA.Vector = {1.f, 0.f};

	FIISVectorRecordIn RecordB;
	RecordB.ChunkId = TEXT("b");
	RecordB.RouteId = TEXT("r");
	RecordB.ModelId = TEXT("m");
	RecordB.Dimensions = 2;
	RecordB.Vector = {0.f, 1.f};

	TArray<FString> BackendWarnings;
	FIISJsonlBruteForceBackend BruteBackend;
	BruteBackend.Rebuild({RecordA, RecordB}, BackendWarnings);
	const TArray<FIISVectorHit> BruteHits = BruteBackend.Search({1.f, 0.f}, 1, BackendWarnings);
	TestEqual(TEXT("brute one hit"), BruteHits.Num(), 1);

	FIISSearchQuery Query;
	Query.QueryText = TEXT("alpha");
	Query.SearchMode = EIISSearchMode::Vector;
	Query.MaxResults = 1;

	TMap<FString, TArray<float>> QueryVectorsByRoute;
	QueryVectorsByRoute.Add(TEXT("r"), {1.f, 0.f});

	FIISSearchResponse DelegatedResponse;
	TestTrue(
		TEXT("delegated search"),
		FIISChunkCatalog::SearchVectorDelegatedForTest(
			FIISChunkCatalog::GetCatalogPath(),
			{RecordA, RecordB},
			QueryVectorsByRoute,
			Query,
			DelegatedResponse));

	TestEqual(TEXT("delegated one result"), DelegatedResponse.Results.Num(), 1);
	TestEqual(TEXT("parity top-1 chunk_id"), DelegatedResponse.Results[0].ChunkId, BruteHits[0].ChunkId);
	return true;
}

namespace IISVectorHnswTest
{
	static TArray<FIISVectorRecordIn> MakeRandomCorpus(const int32 Count, const int32 Dimensions, FRandomStream& Rng)
	{
		TArray<FIISVectorRecordIn> Records;
		Records.Reserve(Count);
		for (int32 Index = 0; Index < Count; ++Index)
		{
			FIISVectorRecordIn Record;
			Record.ChunkId = FString::Printf(TEXT("chunk_%d"), Index);
			Record.RouteId = TEXT("route");
			Record.ModelId = TEXT("model");
			Record.Dimensions = Dimensions;
			Record.Vector.SetNum(Dimensions);
			for (int32 Dim = 0; Dim < Dimensions; ++Dim)
			{
				Record.Vector[Dim] = Rng.FRandRange(-1.f, 1.f);
			}
			Records.Add(MoveTemp(Record));
		}
		return Records;
	}

	static float ComputeRecallAtK(
		const TArray<FIISVectorHit>& ApproximateHits,
		const TArray<FIISVectorHit>& ExactHits,
		const int32 K)
	{
		TSet<FString> ExactIds;
		const int32 ExactK = FMath::Min(K, ExactHits.Num());
		for (int32 Index = 0; Index < ExactK; ++Index)
		{
			ExactIds.Add(ExactHits[Index].ChunkId);
		}

		int32 Overlap = 0;
		const int32 ApproxK = FMath::Min(K, ApproximateHits.Num());
		for (int32 Index = 0; Index < ApproxK; ++Index)
		{
			if (ExactIds.Contains(ApproximateHits[Index].ChunkId))
			{
				++Overlap;
			}
		}

		return ExactK > 0 ? static_cast<float>(Overlap) / static_cast<float>(ExactK) : 0.f;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISHnswRecall,
	"InternalIndexService.Vector.HnswRecall",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISHnswRecall::RunTest(const FString& Parameters)
{
	FRandomStream Rng(42);
	const TArray<FIISVectorRecordIn> Records = IISVectorHnswTest::MakeRandomCorpus(500, 64, Rng);

	TArray<FString> Warnings;
	FIISHnswVectorIndexBackend HnswBackend;
	FIISJsonlBruteForceBackend BruteBackend;
	TestTrue(TEXT("hnsw rebuild"), HnswBackend.Rebuild(Records, Warnings));
	TestTrue(TEXT("brute rebuild"), BruteBackend.Rebuild(Records, Warnings));

	double RecallSum = 0.0;
	constexpr int32 QueryCount = 50;
	constexpr int32 K = 10;
	for (int32 QueryIndex = 0; QueryIndex < QueryCount; ++QueryIndex)
	{
		const int32 RecordIndex = Rng.RandRange(0, Records.Num() - 1);
		const TArray<float>& QueryVector = Records[RecordIndex].Vector;
		const TArray<FIISVectorHit> BruteHits = BruteBackend.Search(QueryVector, K, Warnings);
		const TArray<FIISVectorHit> HnswHits = HnswBackend.Search(QueryVector, K, Warnings);
		RecallSum += IISVectorHnswTest::ComputeRecallAtK(HnswHits, BruteHits, K);
	}

	const float MeanRecall = static_cast<float>(RecallSum / static_cast<double>(QueryCount));
	TestTrue(TEXT("mean recall@10 >= 0.95"), MeanRecall >= 0.95f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISHnswRebuildIdempotent,
	"InternalIndexService.Vector.HnswRebuildIdempotent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISHnswRebuildIdempotent::RunTest(const FString& Parameters)
{
	FRandomStream Rng(7);
	const TArray<FIISVectorRecordIn> Records = IISVectorHnswTest::MakeRandomCorpus(128, 32, Rng);

	TArray<FString> Warnings;
	FIISHnswVectorIndexBackend Backend;
	TestTrue(TEXT("first rebuild"), Backend.Rebuild(Records, Warnings));

	int32 CountFirst = 0;
	int32 DimensionsFirst = 0;
	FString RouteFirst;
	FString ModelFirst;
	TestTrue(TEXT("read manifest first"), Backend.ReadManifest(CountFirst, DimensionsFirst, RouteFirst, ModelFirst));

	TestTrue(TEXT("second rebuild"), Backend.Rebuild(Records, Warnings));

	int32 CountSecond = 0;
	int32 DimensionsSecond = 0;
	FString RouteSecond;
	FString ModelSecond;
	TestTrue(TEXT("read manifest second"), Backend.ReadManifest(CountSecond, DimensionsSecond, RouteSecond, ModelSecond));

	TestEqual(TEXT("count stable"), CountSecond, CountFirst);
	TestEqual(TEXT("dimensions stable"), DimensionsSecond, DimensionsFirst);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISHnswDimensionMismatch,
	"InternalIndexService.Vector.HnswDimensionMismatch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISHnswDimensionMismatch::RunTest(const FString& Parameters)
{
	FIISVectorRecordIn Record;
	Record.ChunkId = TEXT("only");
	Record.RouteId = TEXT("route");
	Record.ModelId = TEXT("model");
	Record.Dimensions = 4;
	Record.Vector = {1.f, 0.f, 0.f, 0.f};

	TArray<FString> Warnings;
	FIISHnswVectorIndexBackend Backend;
	TestTrue(TEXT("rebuild"), Backend.Rebuild({Record}, Warnings));

	const TArray<FIISVectorHit> Hits = Backend.Search({1.f, 0.f}, 3, Warnings);
	TestEqual(TEXT("no hits on mismatch"), Hits.Num(), 0);
	TestTrue(TEXT("warning emitted"), Warnings.ContainsByPredicate([](const FString& Warning)
	{
		return Warning.Contains(TEXT("dimension mismatch"), ESearchCase::IgnoreCase);
	}));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISVectorBackendResolverReadsSettings,
	"InternalIndexService.VectorBackend.ResolverReadsSettings",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISVectorBackendResolverReadsSettings::RunTest(const FString& Parameters)
{
	UIISSettings* Settings = GetMutableDefault<UIISSettings>();
	const FString Original = Settings->VectorBackend;

	Settings->VectorBackend = TEXT("HNSW");
	TestEqual(TEXT("resolver lowercases and reads hnsw"), IISResolveConfiguredVectorBackendId(), FString(TEXT("hnsw")));

	Settings->VectorBackend = TEXT("jsonl_bruteforce");
	TestEqual(TEXT("resolver reads bruteforce"), IISResolveConfiguredVectorBackendId(), FString(TEXT("jsonl_bruteforce")));

	Settings->VectorBackend = Original;
	return true;
}

#endif
