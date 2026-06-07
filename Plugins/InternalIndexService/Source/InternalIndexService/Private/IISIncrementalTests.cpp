/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "IISEmbeddingJobQueue.h"
#include "IISEmbeddingTypes.h"
#include "IISChunkCatalog.h"
#include "IISCatalogTypes.h"
#include "IISStoragePaths.h"
#include "IISSearchTypes.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace IISIncrementalTest
{
	static bool ParseCatalogStatusFromReport(const FString& ReportPath, FString& OutStatus)
	{
		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *ReportPath))
		{
			return false;
		}
		TSharedPtr<FJsonObject> Root;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
		{
			return false;
		}
		const TSharedPtr<FJsonObject>* SummaryObj = nullptr;
		if (!Root->TryGetObjectField(TEXT("summary"), SummaryObj) || !SummaryObj)
		{
			return false;
		}
		return (*SummaryObj)->TryGetStringField(TEXT("status"), OutStatus);
	}

	static bool JsonlContainsChunkId(const FString& FilePath, const FString& ChunkId)
	{
		TArray<FString> Lines;
		if (!FFileHelper::LoadFileToStringArray(Lines, *FilePath))
		{
			return false;
		}
		for (const FString& Line : Lines)
		{
			if (Line.Contains(ChunkId))
			{
				return true;
			}
		}
		return false;
	}

	static bool FindEmbeddingJobStatus(
		const FString& ChunkId,
		FString& OutStatus)
	{
		const FString FilePath = FIISStoragePaths::GetEmbeddingsDir() / TEXT("embedding_jobs.jsonl");
		TArray<FString> Lines;
		if (!FFileHelper::LoadFileToStringArray(Lines, *FilePath))
		{
			return false;
		}
		for (const FString& Line : Lines)
		{
			TSharedPtr<FJsonObject> Obj;
			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Line);
			if (!FJsonSerializer::Deserialize(Reader, Obj) || !Obj.IsValid())
			{
				continue;
			}
			FString JobChunkId;
			if (!Obj->TryGetStringField(TEXT("chunk_id"), JobChunkId) || JobChunkId != ChunkId)
			{
				continue;
			}
			return Obj->TryGetStringField(TEXT("status"), OutStatus);
		}
		return false;
	}

	static bool WriteTestVectorRecord(
		const FString& ChunkId,
		const FString& RouteId,
		const FString& TextSha256,
		const FString& ModelId,
		const int32 Dimensions)
	{
		FIISStoragePaths::EnsureDefaultFolders();
		const FString FilePath = FIISStoragePaths::GetVectorsDir() / TEXT("chunk_vectors.jsonl");
		TSharedRef<FJsonObject> Obj = MakeShared<FJsonObject>();
		Obj->SetStringField(TEXT("chunk_id"), ChunkId);
		Obj->SetStringField(TEXT("route_id"), RouteId);
		Obj->SetStringField(TEXT("text_sha256"), TextSha256);
		Obj->SetStringField(TEXT("model_id"), ModelId);
		Obj->SetNumberField(TEXT("dimensions"), static_cast<double>(Dimensions));
		TArray<TSharedPtr<FJsonValue>> VectorValues;
		for (int32 Index = 0; Index < Dimensions; ++Index)
		{
			VectorValues.Add(MakeShared<FJsonValueNumber>(0.01f * static_cast<float>(Index)));
		}
		Obj->SetArrayField(TEXT("vector"), VectorValues);
		FString Line;
		const TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
			TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Line);
		FJsonSerializer::Serialize(Obj, Writer);
		return FFileHelper::SaveStringToFile(
			Line + LINE_TERMINATOR, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}
}

class FIISTestEmbeddingRouteExecutor : public IIISEmbeddingRouteExecutor
{
public:
	virtual FString GetExecutorId() const override
	{
		return TEXT("iis.test.incremental.embedding");
	}

	virtual bool ResolveEmbeddingRoute(
		const FString& TaskKind,
		FIISEmbeddingRoute& OutRoute,
		TArray<FString>& OutWarnings) override
	{
		(void)OutWarnings;
		OutRoute = FIISEmbeddingRoute();
		OutRoute.RouteId = TEXT("iis.embedding.code");
		OutRoute.TaskKind = TaskKind;
		OutRoute.ModelId = TEXT("model-b");
		OutRoute.Dimensions = 768;
		OutRoute.bEnabled = true;
		return true;
	}

	virtual bool ExecuteEmbeddingRoute(
		const FIISEmbeddingRequest& Request,
		FIISEmbeddingResponse& OutResponse) override
	{
		(void)Request;
		OutResponse.bSuccess = false;
		return false;
	}
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISVectorSkipKeyIncludesModelAndDims,
	"InternalIndexService.Incremental.VectorSkipKey.ModelAndDims",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISVectorSkipKeyIncludesModelAndDims::RunTest(const FString& Parameters)
{
	const FString Base = FIISEmbeddingJobQueue::MakeVectorSkipKey(
		TEXT("chunkA"), TEXT("iis.embedding.code"), TEXT("sha123"),
		TEXT("nomic-embed-text"), 768);
	const FString DiffModel = FIISEmbeddingJobQueue::MakeVectorSkipKey(
		TEXT("chunkA"), TEXT("iis.embedding.code"), TEXT("sha123"),
		TEXT("bge-m3"), 768);
	const FString DiffDims = FIISEmbeddingJobQueue::MakeVectorSkipKey(
		TEXT("chunkA"), TEXT("iis.embedding.code"), TEXT("sha123"),
		TEXT("nomic-embed-text"), 1024);

	TestNotEqual(TEXT("model change changes key"), Base, DiffModel);
	TestNotEqual(TEXT("dimensions change changes key"), Base, DiffDims);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISClassifyChunkUpsert,
	"InternalIndexService.Incremental.ClassifyUpsert",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISClassifyChunkUpsert::RunTest(const FString& Parameters)
{
	using E = EIISChunkUpsertAction;
	TestEqual(TEXT("new id -> insert"),
		FIISChunkCatalog::ClassifyUpsert(false, TEXT(""), TEXT("shaNew")), E::Insert);
	TestEqual(TEXT("same sha -> noop"),
		FIISChunkCatalog::ClassifyUpsert(true, TEXT("shaX"), TEXT("shaX")), E::NoOp);
	TestEqual(TEXT("diff sha -> conflict"),
		FIISChunkCatalog::ClassifyUpsert(true, TEXT("shaOld"), TEXT("shaNew")), E::Conflict);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISImportIsIdempotent,
	"InternalIndexService.Incremental.Idempotent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISImportIsIdempotent::RunTest(const FString& Parameters)
{
	const FString FixturePath = FPaths::ProjectSavedDir()
		/ TEXT("IISValidation")
		/ TEXT("incremental_idempotent_fixture.jsonl");

	IFileManager::Get().MakeDirectory(*FPaths::GetPath(FixturePath), true);

	const FString Line = TEXT(
		"{\"chunk_id\":\"iis.test.idempotent.1\",\"chunk_kind\":\"code\",\"title\":\"T\","
		"\"text\":\"same text\",\"source_id\":\"src-idempotent\",\"source_run_id\":\"run-1\","
		"\"destination_run_id\":\"dest-1\",\"retrieval_labels\":[],\"retrieval_groups\":[],"
		"\"source_references\":[]}");

	TestTrue(TEXT("write fixture"), FFileHelper::SaveStringToFile(Line, *FixturePath));

	FString ReportPath;
	TArray<FString> Warnings;
	TestTrue(TEXT("first import"), FIISChunkCatalog::BuildOrUpdateCatalogFromChunkStorePath(
		FixturePath, ReportPath, Warnings));

	FIISCatalogBuildReport FirstReport;
	{
		FString JsonText;
		TestTrue(TEXT("read first report"), FFileHelper::LoadFileToString(JsonText, *ReportPath));
		TSharedPtr<FJsonObject> Root;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		TestTrue(TEXT("parse first report"), FJsonSerializer::Deserialize(Reader, Root) && Root.IsValid());
		const TSharedPtr<FJsonObject>* SummaryObj = nullptr;
		TestTrue(TEXT("first summary"), Root->TryGetObjectField(TEXT("summary"), SummaryObj) && SummaryObj);
		double CatalogCount = 0.0;
		(*SummaryObj)->TryGetNumberField(TEXT("catalog_chunk_count"), CatalogCount);
		TestTrue(TEXT("first catalog has chunks"), CatalogCount >= 1.0);
		FirstReport.Summary.CatalogChunkCount = static_cast<int32>(CatalogCount);
	}

	Warnings.Reset();
	TestTrue(TEXT("second import"), FIISChunkCatalog::BuildOrUpdateCatalogFromChunkStorePath(
		FixturePath, ReportPath, Warnings));

	FString JsonText;
	TestTrue(TEXT("read second report"), FFileHelper::LoadFileToString(JsonText, *ReportPath));
	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
	TestTrue(TEXT("parse second report"), FJsonSerializer::Deserialize(Reader, Root) && Root.IsValid());
	const TSharedPtr<FJsonObject>* SummaryObj = nullptr;
	TestTrue(TEXT("second summary"), Root->TryGetObjectField(TEXT("summary"), SummaryObj) && SummaryObj);

	double SecondCatalogCount = 0.0;
	double Inserted = 0.0;
	(*SummaryObj)->TryGetNumberField(TEXT("catalog_chunk_count"), SecondCatalogCount);
	(*SummaryObj)->TryGetNumberField(TEXT("inserted_chunk_count"), Inserted);

	TestEqual(TEXT("catalog count stable"), static_cast<int32>(SecondCatalogCount), FirstReport.Summary.CatalogChunkCount);
	TestEqual(TEXT("second run inserts nothing"), static_cast<int32>(Inserted), 0);

	FString SecondStatus;
	TestTrue(TEXT("read second status"), IISIncrementalTest::ParseCatalogStatusFromReport(ReportPath, SecondStatus));
	TestEqual(TEXT("second import status is ready"), SecondStatus, FString(TEXT("ready")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISStaleChunkReactivatedOnIdenticalReimport,
	"InternalIndexService.Incremental.StaleReactivate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISStaleChunkReactivatedOnIdenticalReimport::RunTest(const FString& Parameters)
{
	const FString Dir = FPaths::ProjectSavedDir() / TEXT("IISValidation");
	IFileManager::Get().MakeDirectory(*Dir, true);

	const FString FixtureA = Dir / TEXT("incremental_stale_a.jsonl");
	const FString FixtureB = Dir / TEXT("incremental_stale_b.jsonl");
	const FString ChunkId = TEXT("iis.test.stale.reactivate.1");
	const FString SourceId = TEXT("src-stale-reactivate");

	const FString LineA = FString::Printf(
		TEXT("{\"chunk_id\":\"%s\",\"chunk_kind\":\"code\",\"title\":\"A\",\"text\":\"stable text\",")
		TEXT("\"source_id\":\"%s\",\"source_run_id\":\"run-1\",\"destination_run_id\":\"dest-1\",")
		TEXT("\"retrieval_labels\":[],\"retrieval_groups\":[],\"source_references\":[]}"),
		*ChunkId, *SourceId);
	const FString LineB = FString::Printf(
		TEXT("{\"chunk_id\":\"iis.test.stale.reactivate.other\",\"chunk_kind\":\"code\",\"title\":\"B\",\"text\":\"other\",")
		TEXT("\"source_id\":\"%s\",\"source_run_id\":\"run-2\",\"destination_run_id\":\"dest-1\",")
		TEXT("\"retrieval_labels\":[],\"retrieval_groups\":[],\"source_references\":[]}"),
		*SourceId);

	TestTrue(TEXT("write fixture A"), FFileHelper::SaveStringToFile(LineA, *FixtureA));
	TestTrue(TEXT("write fixture B"), FFileHelper::SaveStringToFile(LineB, *FixtureB));

	FString ReportPath;
	TArray<FString> Warnings;
	TestTrue(TEXT("import A"), FIISChunkCatalog::BuildOrUpdateCatalogFromChunkStorePath(FixtureA, ReportPath, Warnings));
	Warnings.Reset();
	TestTrue(TEXT("import B only"), FIISChunkCatalog::BuildOrUpdateCatalogFromChunkStorePath(FixtureB, ReportPath, Warnings));

	FIISIndexChunk StaleChunk;
	TestTrue(TEXT("load stale chunk"), FIISChunkCatalog::LoadChunkById(ChunkId, StaleChunk, Warnings));
	TestEqual(TEXT("chunk marked stale"), StaleChunk.LifecycleState, FString(TEXT("stale")));

	Warnings.Reset();
	TestTrue(TEXT("re-import A"), FIISChunkCatalog::BuildOrUpdateCatalogFromChunkStorePath(FixtureA, ReportPath, Warnings));

	FIISIndexChunk Reactivated;
	TestTrue(TEXT("load reactivated chunk"), FIISChunkCatalog::LoadChunkById(ChunkId, Reactivated, Warnings));
	TestEqual(TEXT("chunk active again"), Reactivated.LifecycleState, FString(TEXT("active")));
	TestTrue(TEXT("embedding eligible"), FIISEmbeddingJobQueue::ShouldIncludeChunkForEmbedding(Reactivated));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISChunkLifecycleHelpers,
	"InternalIndexService.Incremental.LifecycleHelpers",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISChunkLifecycleHelpers::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("empty is active"), FIISChunkCatalog::IsActiveLifecycleState(TEXT("")));
	TestTrue(TEXT("active is active"), FIISChunkCatalog::IsActiveLifecycleState(TEXT("active")));
	TestTrue(TEXT("empty is retrievable"), FIISChunkCatalog::IsRetrievableLifecycleState(TEXT("")));
	TestTrue(TEXT("active is retrievable"), FIISChunkCatalog::IsRetrievableLifecycleState(TEXT("active")));
	TestFalse(TEXT("stale not retrievable"), FIISChunkCatalog::IsRetrievableLifecycleState(TEXT("stale")));
	TestFalse(TEXT("tombstoned not retrievable"), FIISChunkCatalog::IsRetrievableLifecycleState(TEXT("tombstoned")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISTombstoneChunkNotEmbeddable,
	"InternalIndexService.Incremental.TombstoneNotEmbeddable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISTombstoneChunkNotEmbeddable::RunTest(const FString& Parameters)
{
	FIISIndexChunk Tombstoned;
	Tombstoned.LifecycleState = TEXT("tombstoned");
	TestFalse(TEXT("tombstoned excluded"), FIISEmbeddingJobQueue::ShouldIncludeChunkForEmbedding(Tombstoned));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISTombstoneChunkNoReactivateOnReimport,
	"InternalIndexService.Incremental.TombstoneNoReactivate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISTombstoneChunkNoReactivateOnReimport::RunTest(const FString& Parameters)
{
	const FString Dir = FPaths::ProjectSavedDir() / TEXT("IISValidation");
	IFileManager::Get().MakeDirectory(*Dir, true);

	const FString FixturePath = Dir / TEXT("incremental_tombstone.jsonl");
	const FString ChunkId = TEXT("iis.test.tombstone.no.reactivate.1");
	const FString Line = FString::Printf(
		TEXT("{\"chunk_id\":\"%s\",\"chunk_kind\":\"code\",\"title\":\"T\",\"text\":\"tombstone text\",")
		TEXT("\"lifecycle_state\":\"tombstoned\",\"source_id\":\"src-tombstone\",\"source_run_id\":\"run-1\",")
		TEXT("\"destination_run_id\":\"dest-1\",\"retrieval_labels\":[],\"retrieval_groups\":[],\"source_references\":[]}"),
		*ChunkId);

	TestTrue(TEXT("write fixture"), FFileHelper::SaveStringToFile(Line, *FixturePath));

	FString ReportPath;
	TArray<FString> Warnings;
	TestTrue(TEXT("first import"), FIISChunkCatalog::BuildOrUpdateCatalogFromChunkStorePath(FixturePath, ReportPath, Warnings));

	FIISIndexChunk First;
	TestTrue(TEXT("load tombstoned"), FIISChunkCatalog::LoadChunkById(ChunkId, First, Warnings));
	TestEqual(TEXT("tombstoned on import"), First.LifecycleState, FString(TEXT("tombstoned")));

	Warnings.Reset();
	TestTrue(TEXT("second import"), FIISChunkCatalog::BuildOrUpdateCatalogFromChunkStorePath(FixturePath, ReportPath, Warnings));

	FIISIndexChunk Second;
	TestTrue(TEXT("reload tombstoned"), FIISChunkCatalog::LoadChunkById(ChunkId, Second, Warnings));
	TestEqual(TEXT("still tombstoned"), Second.LifecycleState, FString(TEXT("tombstoned")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISStaleChunkExcludedFromSearch,
	"InternalIndexService.Incremental.StaleExcludedFromSearch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISStaleChunkExcludedFromSearch::RunTest(const FString& Parameters)
{
	const FString Dir = FPaths::ProjectSavedDir() / TEXT("IISValidation");
	IFileManager::Get().MakeDirectory(*Dir, true);

	const FString FixtureA = Dir / TEXT("incremental_search_stale_a.jsonl");
	const FString FixtureB = Dir / TEXT("incremental_search_stale_b.jsonl");
	const FString ChunkId = TEXT("iis.test.search.stale.1");
	const FString SourceId = TEXT("src-search-stale");
	const FString UniqueToken = TEXT("zzsearchstaletoken99");

	const FString LineA = FString::Printf(
		TEXT("{\"chunk_id\":\"%s\",\"chunk_kind\":\"code\",\"title\":\"A\",\"text\":\"%s\",")
		TEXT("\"source_id\":\"%s\",\"source_run_id\":\"run-1\",\"destination_run_id\":\"dest-1\",")
		TEXT("\"retrieval_labels\":[],\"retrieval_groups\":[],\"source_references\":[]}"),
		*ChunkId, *UniqueToken, *SourceId);
	const FString LineB = FString::Printf(
		TEXT("{\"chunk_id\":\"iis.test.search.stale.other\",\"chunk_kind\":\"code\",\"title\":\"B\",\"text\":\"other\",")
		TEXT("\"source_id\":\"%s\",\"source_run_id\":\"run-2\",\"destination_run_id\":\"dest-1\",")
		TEXT("\"retrieval_labels\":[],\"retrieval_groups\":[],\"source_references\":[]}"),
		*SourceId);

	TestTrue(TEXT("write A"), FFileHelper::SaveStringToFile(LineA, *FixtureA));
	TestTrue(TEXT("write B"), FFileHelper::SaveStringToFile(LineB, *FixtureB));

	FString ReportPath;
	TArray<FString> Warnings;
	TestTrue(TEXT("import A"), FIISChunkCatalog::BuildOrUpdateCatalogFromChunkStorePath(FixtureA, ReportPath, Warnings));
	Warnings.Reset();
	TestTrue(TEXT("import B"), FIISChunkCatalog::BuildOrUpdateCatalogFromChunkStorePath(FixtureB, ReportPath, Warnings));

	FIISSearchQuery Query;
	Query.QueryText = UniqueToken;
	Query.SearchMode = EIISSearchMode::Lexical;
	Query.MaxResults = 20;

	FIISSearchResponse Response;
	TestTrue(TEXT("search"), FIISChunkCatalog::SearchCatalog(Query, Response));

	for (const FIISSearchResult& Result : Response.Results)
	{
		TestNotEqual(TEXT("stale chunk not in results"), Result.ChunkId, ChunkId);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISStaleChunkVectorCleanup,
	"InternalIndexService.Incremental.StaleVectorCleanup",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISStaleChunkVectorCleanup::RunTest(const FString& Parameters)
{
	const FString Dir = FPaths::ProjectSavedDir() / TEXT("IISValidation");
	IFileManager::Get().MakeDirectory(*Dir, true);

	const FString FixtureA = Dir / TEXT("incremental_vector_stale_a.jsonl");
	const FString FixtureB = Dir / TEXT("incremental_vector_stale_b.jsonl");
	const FString ChunkId = TEXT("iis.test.vector.stale.1");
	const FString SourceId = TEXT("src-vector-stale");
	const FString ChunkText = TEXT("vector stale cleanup text");
	const FString TextSha256 = FIISEmbeddingJobQueue::ComputeSHA256(ChunkText);
	const FString RouteId = TEXT("iis.embedding.code");

	const FString LineA = FString::Printf(
		TEXT("{\"chunk_id\":\"%s\",\"chunk_kind\":\"code\",\"title\":\"A\",\"text\":\"%s\",")
		TEXT("\"source_id\":\"%s\",\"source_run_id\":\"run-1\",\"destination_run_id\":\"dest-1\",")
		TEXT("\"retrieval_labels\":[],\"retrieval_groups\":[],\"source_references\":[]}"),
		*ChunkId, *ChunkText, *SourceId);
	const FString LineB = FString::Printf(
		TEXT("{\"chunk_id\":\"iis.test.vector.stale.other\",\"chunk_kind\":\"code\",\"title\":\"B\",\"text\":\"other\",")
		TEXT("\"source_id\":\"%s\",\"source_run_id\":\"run-2\",\"destination_run_id\":\"dest-1\",")
		TEXT("\"retrieval_labels\":[],\"retrieval_groups\":[],\"source_references\":[]}"),
		*SourceId);

	TestTrue(TEXT("write A"), FFileHelper::SaveStringToFile(LineA, *FixtureA));
	TestTrue(TEXT("write B"), FFileHelper::SaveStringToFile(LineB, *FixtureB));

	FString ReportPath;
	TArray<FString> Warnings;
	TestTrue(TEXT("import A"), FIISChunkCatalog::BuildOrUpdateCatalogFromChunkStorePath(FixtureA, ReportPath, Warnings));

	const FString VectorPath = FIISStoragePaths::GetVectorsDir() / TEXT("chunk_vectors.jsonl");
	IFileManager::Get().Delete(*VectorPath);
	TestTrue(TEXT("seed vector"), IISIncrementalTest::WriteTestVectorRecord(
		ChunkId, RouteId, TextSha256, TEXT("model-a"), 768));

	Warnings.Reset();
	TestTrue(TEXT("import B stale"), FIISChunkCatalog::BuildOrUpdateCatalogFromChunkStorePath(FixtureB, ReportPath, Warnings));

	FIISIndexChunk StaleChunk;
	TestTrue(TEXT("chunk stale"), FIISChunkCatalog::LoadChunkById(ChunkId, StaleChunk, Warnings));
	TestEqual(TEXT("lifecycle stale"), StaleChunk.LifecycleState, FString(TEXT("stale")));

	Warnings.Reset();
	FString EmbedReport;
	TestTrue(TEXT("build embedding jobs"), FIISEmbeddingJobQueue::BuildEmbeddingJobsFromCatalog(EmbedReport, Warnings));

	TestFalse(TEXT("vector pruned"), IISIncrementalTest::JsonlContainsChunkId(VectorPath, ChunkId));

	FString JobStatus;
	TestFalse(TEXT("no embedding job"), IISIncrementalTest::FindEmbeddingJobStatus(ChunkId, JobStatus));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISModelChangeTriggersRebuild,
	"InternalIndexService.Incremental.ModelChangeRebuild",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISModelChangeTriggersRebuild::RunTest(const FString& Parameters)
{
	const FString Dir = FPaths::ProjectSavedDir() / TEXT("IISValidation");
	IFileManager::Get().MakeDirectory(*Dir, true);

	const FString FixturePath = Dir / TEXT("incremental_model_change.jsonl");
	const FString ChunkId = TEXT("iis.test.model.change.1");
	const FString ChunkText = TEXT("model change rebuild integration");
	const FString TextSha256 = FIISEmbeddingJobQueue::ComputeSHA256(ChunkText);
	const FString RouteId = TEXT("iis.embedding.code");

	const FString Line = FString::Printf(
		TEXT("{\"chunk_id\":\"%s\",\"chunk_kind\":\"code\",\"title\":\"M\",\"text\":\"%s\",")
		TEXT("\"source_id\":\"src-model-change\",\"source_run_id\":\"run-1\",\"destination_run_id\":\"dest-1\",")
		TEXT("\"retrieval_labels\":[],\"retrieval_groups\":[],\"source_references\":[]}"),
		*ChunkId, *ChunkText);

	TestTrue(TEXT("write fixture"), FFileHelper::SaveStringToFile(Line, *FixturePath));

	FString ReportPath;
	TArray<FString> Warnings;
	TestTrue(TEXT("import chunk"), FIISChunkCatalog::BuildOrUpdateCatalogFromChunkStorePath(FixturePath, ReportPath, Warnings));

	const FString VectorPath = FIISStoragePaths::GetVectorsDir() / TEXT("chunk_vectors.jsonl");
	IFileManager::Get().Delete(*VectorPath);
	TestTrue(TEXT("seed model-a vector"), IISIncrementalTest::WriteTestVectorRecord(
		ChunkId, RouteId, TextSha256, TEXT("model-a"), 768));

	TSharedRef<FIISTestEmbeddingRouteExecutor> TestExecutor = MakeShared<FIISTestEmbeddingRouteExecutor>();
	TestTrue(TEXT("register executor"), FIISEmbeddingRouteExecutorRegistry::RegisterExecutor(TestExecutor));

	Warnings.Reset();
	FString EmbedReport;
	const bool bBuilt = FIISEmbeddingJobQueue::BuildEmbeddingJobsFromCatalog(EmbedReport, Warnings);
	FIISEmbeddingRouteExecutorRegistry::UnregisterExecutor(TestExecutor->GetExecutorId());

	TestTrue(TEXT("build jobs"), bBuilt);

	FString JobStatus;
	TestTrue(TEXT("job exists"), IISIncrementalTest::FindEmbeddingJobStatus(ChunkId, JobStatus));
	TestEqual(TEXT("pending after model change"), JobStatus, FString(TEXT("Pending")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISImportConflictReportWritten,
	"InternalIndexService.Incremental.ConflictReport",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISImportConflictReportWritten::RunTest(const FString& Parameters)
{
	const FString Dir = FPaths::ProjectSavedDir() / TEXT("IISValidation");
	IFileManager::Get().MakeDirectory(*Dir, true);

	const FString FixturePath = Dir / TEXT("incremental_conflict.jsonl");
	const FString ChunkId = TEXT("iis.test.conflict.1");
	const FString SourceId = TEXT("src-conflict");

	const FString LineA = FString::Printf(
		TEXT("{\"chunk_id\":\"%s\",\"chunk_kind\":\"code\",\"title\":\"A\",\"text\":\"text version A\",")
		TEXT("\"source_id\":\"%s\",\"source_run_id\":\"run-1\",\"destination_run_id\":\"dest-1\",")
		TEXT("\"retrieval_labels\":[],\"retrieval_groups\":[],\"source_references\":[]}"),
		*ChunkId, *SourceId);
	const FString LineB = FString::Printf(
		TEXT("{\"chunk_id\":\"%s\",\"chunk_kind\":\"code\",\"title\":\"B\",\"text\":\"text version B\",")
		TEXT("\"source_id\":\"%s\",\"source_run_id\":\"run-2\",\"destination_run_id\":\"dest-1\",")
		TEXT("\"retrieval_labels\":[],\"retrieval_groups\":[],\"source_references\":[]}"),
		*ChunkId, *SourceId);

	TestTrue(TEXT("write line A"), FFileHelper::SaveStringToFile(LineA, *FixturePath));

	FString ReportPath;
	TArray<FString> Warnings;
	TestTrue(TEXT("import A"), FIISChunkCatalog::BuildOrUpdateCatalogFromChunkStorePath(FixturePath, ReportPath, Warnings));

	const FString ShaA = FIISEmbeddingJobQueue::ComputeSHA256(TEXT("text version A"));

	Warnings.Reset();
	TestTrue(TEXT("write line B"), FFileHelper::SaveStringToFile(LineB, *FixturePath));
	TestTrue(TEXT("import B"), FIISChunkCatalog::BuildOrUpdateCatalogFromChunkStorePath(FixturePath, ReportPath, Warnings));

	const FString ReportsDir = FIISStoragePaths::GetReportsDir();
	TArray<FString> ConflictFiles;
	IFileManager::Get().FindFiles(ConflictFiles, *(ReportsDir / TEXT("import_conflicts_*.json")), true, false);
	TestTrue(TEXT("conflict report exists"), ConflictFiles.Num() > 0);

	FString ConflictJson;
	bool bFoundConflict = false;
	for (const FString& FileName : ConflictFiles)
	{
		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *(ReportsDir / FileName)))
		{
			continue;
		}
		TSharedPtr<FJsonObject> Root;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
		{
			continue;
		}
		const TArray<TSharedPtr<FJsonValue>>* Conflicts = nullptr;
		if (!Root->TryGetArrayField(TEXT("conflicts"), Conflicts) || !Conflicts)
		{
			continue;
		}
		for (const TSharedPtr<FJsonValue>& Value : *Conflicts)
		{
			const TSharedPtr<FJsonObject> Obj = Value->AsObject();
			if (!Obj.IsValid())
			{
				continue;
			}
			FString ReportChunkId;
			FString OldSha;
			FString NewSha;
			FString Policy;
			Obj->TryGetStringField(TEXT("chunk_id"), ReportChunkId);
			Obj->TryGetStringField(TEXT("old_text_sha256"), OldSha);
			Obj->TryGetStringField(TEXT("new_text_sha256"), NewSha);
			Obj->TryGetStringField(TEXT("policy"), Policy);
			if (ReportChunkId == ChunkId && OldSha == ShaA && Policy == TEXT("overwrite-in-place"))
			{
				bFoundConflict = true;
				TestEqual(TEXT("new sha recorded"), NewSha, FIISEmbeddingJobQueue::ComputeSHA256(TEXT("text version B")));
			}
		}
	}
	TestTrue(TEXT("conflict entry found"), bFoundConflict);

	FIISIndexChunk Updated;
	TestTrue(TEXT("load updated chunk"), FIISChunkCatalog::LoadChunkById(ChunkId, Updated, Warnings));
	TestEqual(TEXT("active after conflict overwrite"), Updated.LifecycleState, FString(TEXT("active")));
	TestEqual(TEXT("text updated"), Updated.Text, FString(TEXT("text version B")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISImportRollbackOnUpsertFailure,
	"InternalIndexService.Incremental.ImportRollback",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISImportRollbackOnUpsertFailure::RunTest(const FString& Parameters)
{
	const FString Dir = FPaths::ProjectSavedDir() / TEXT("IISValidation");
	IFileManager::Get().MakeDirectory(*Dir, true);

	const FString FixturePath = Dir / TEXT("incremental_rollback.jsonl");
	const FString StableChunkId = TEXT("iis.test.rollback.stable.1");
	const FString FailingChunkId = TEXT("iis.test.rollback.fail.1");
	const FString SourceId = TEXT("src-rollback");
	const FString StableText = TEXT("rollback stable text");

	const FString InitialLine = FString::Printf(
		TEXT("{\"chunk_id\":\"%s\",\"chunk_kind\":\"code\",\"title\":\"S\",\"text\":\"%s\",")
		TEXT("\"source_id\":\"%s\",\"source_run_id\":\"run-0\",\"destination_run_id\":\"dest-1\",")
		TEXT("\"retrieval_labels\":[],\"retrieval_groups\":[],\"source_references\":[]}"),
		*StableChunkId, *StableText, *SourceId);

	TestTrue(TEXT("write initial"), FFileHelper::SaveStringToFile(InitialLine, *FixturePath));

	FString ReportPath;
	TArray<FString> Warnings;
	TestTrue(TEXT("initial import"), FIISChunkCatalog::BuildOrUpdateCatalogFromChunkStorePath(FixturePath, ReportPath, Warnings));

	const FString Batch = FString::Printf(
		TEXT("%s\n")
		TEXT("{\"chunk_id\":\"%s\",\"chunk_kind\":\"code\",\"title\":\"F\",\"text\":\"fail chunk\",")
		TEXT("\"source_id\":\"%s\",\"source_run_id\":\"run-1\",\"destination_run_id\":\"dest-1\",")
		TEXT("\"retrieval_labels\":[],\"retrieval_groups\":[],\"source_references\":[]}"),
		*InitialLine, *FailingChunkId, *SourceId);

	TestTrue(TEXT("write batch"), FFileHelper::SaveStringToFile(Batch, *FixturePath));

	FIISChunkCatalog::SetForceUpsertFailureChunkIdForTest(FailingChunkId);
	Warnings.Reset();
	TestTrue(TEXT("batch import returns"), FIISChunkCatalog::BuildOrUpdateCatalogFromChunkStorePath(FixturePath, ReportPath, Warnings));
	FIISChunkCatalog::ClearForceUpsertFailureChunkIdForTest();

	bool bForcedFailureReported = false;
	for (const FString& Message : Warnings)
	{
		if (Message.Contains(TEXT("Forced upsert failure")))
		{
			bForcedFailureReported = true;
			break;
		}
	}
	TestTrue(TEXT("forced failure reported"), bForcedFailureReported);

	FIISIndexChunk Stable;
	TestTrue(TEXT("stable still loads"), FIISChunkCatalog::LoadChunkById(StableChunkId, Stable, Warnings));
	TestEqual(TEXT("stable text unchanged"), Stable.Text, StableText);

	FIISIndexChunk Failed;
	TArray<FString> LoadWarnings;
	TestFalse(TEXT("failed chunk not inserted"), FIISChunkCatalog::LoadChunkById(FailingChunkId, Failed, LoadWarnings));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISImportPreservesCatalogOnInvalidJsonl,
	"InternalIndexService.Incremental.ImportInvalidJsonlPreserves",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISImportPreservesCatalogOnInvalidJsonl::RunTest(const FString& Parameters)
{
	const FString Dir = FPaths::ProjectSavedDir() / TEXT("IISValidation");
	const FString FixturePath = Dir / TEXT("incremental_invalid_only.jsonl");
	const FString ChunkId = TEXT("iis.test.invalid.preserve.1");

	const FString ValidLine = FString::Printf(
		TEXT("{\"chunk_id\":\"%s\",\"chunk_kind\":\"code\",\"title\":\"P\",\"text\":\"preserve me\",")
		TEXT("\"source_id\":\"src-invalid\",\"source_run_id\":\"run-1\",\"destination_run_id\":\"dest-1\",")
		TEXT("\"retrieval_labels\":[],\"retrieval_groups\":[],\"source_references\":[]}"),
		*ChunkId);

	TestTrue(TEXT("seed catalog"), FFileHelper::SaveStringToFile(ValidLine, *FixturePath));

	FString ReportPath;
	TArray<FString> Warnings;
	TestTrue(TEXT("first import"), FIISChunkCatalog::BuildOrUpdateCatalogFromChunkStorePath(FixturePath, ReportPath, Warnings));

	TestTrue(TEXT("write invalid only"), FFileHelper::SaveStringToFile(TEXT("{not valid json"), *FixturePath));
	Warnings.Reset();
	TestFalse(TEXT("invalid import fails load"), FIISChunkCatalog::BuildOrUpdateCatalogFromChunkStorePath(FixturePath, ReportPath, Warnings));

	FIISIndexChunk Chunk;
	TestTrue(TEXT("prior chunk intact"), FIISChunkCatalog::LoadChunkById(ChunkId, Chunk, Warnings));
	TestEqual(TEXT("text preserved"), Chunk.Text, FString(TEXT("preserve me")));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
