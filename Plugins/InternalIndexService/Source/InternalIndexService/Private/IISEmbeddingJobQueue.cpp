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

#include "IISEmbeddingJobQueue.h"

#include "IISChunkCatalog.h"
#include "IISStoragePaths.h"
#include "IISVectorIndexBackend.h"
#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"
#include "SQLiteDatabase.h"
#include "SQLitePreparedStatement.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Policies/CondensedJsonPrintPolicy.h"

namespace
{
	// -------------------------------------------------------------------------
	// Section A: SHA-256 (self-contained, no OpenSSL dependency)
	// -------------------------------------------------------------------------

	static const uint32 SHA256_K[64] = {
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
		0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
		0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
		0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
		0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
		0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
		0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
		0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
		0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
	};

	static uint32 RightRotate32(const uint32 Value, const uint32 Count)
	{
		return (Value >> Count) | (Value << (32u - Count));
	}

	FString SHA256Bytes(const uint8* Data, const int64 Length)
	{
		uint32 H[8] = {
			0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
			0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u
		};

		const uint64 TotalBits = static_cast<uint64>(Length) * 8u;

		int64 PaddedLen = Length + 1;
		while (PaddedLen % 64 != 56)
		{
			++PaddedLen;
		}
		PaddedLen += 8;

		TArray<uint8> Msg;
		Msg.AddZeroed(static_cast<int32>(PaddedLen));
		FMemory::Memcpy(Msg.GetData(), Data, Length);
		Msg[Length] = 0x80u;

		for (int32 i = 0; i < 8; ++i)
		{
			Msg[PaddedLen - 8 + i] = static_cast<uint8>((TotalBits >> (56u - static_cast<uint32>(i) * 8u)) & 0xFFu);
		}

		for (int64 ChunkStart = 0; ChunkStart < PaddedLen; ChunkStart += 64)
		{
			uint32 W[64] = {};
			for (int32 i = 0; i < 16; ++i)
			{
				const int64 Base = ChunkStart + i * 4;
				W[i] = (static_cast<uint32>(Msg[Base]) << 24u)
					| (static_cast<uint32>(Msg[Base + 1]) << 16u)
					| (static_cast<uint32>(Msg[Base + 2]) << 8u)
					| static_cast<uint32>(Msg[Base + 3]);
			}
			for (int32 i = 16; i < 64; ++i)
			{
				const uint32 S0 = RightRotate32(W[i - 15], 7u) ^ RightRotate32(W[i - 15], 18u) ^ (W[i - 15] >> 3u);
				const uint32 S1 = RightRotate32(W[i - 2], 17u) ^ RightRotate32(W[i - 2], 19u) ^ (W[i - 2] >> 10u);
				W[i] = W[i - 16] + S0 + W[i - 7] + S1;
			}

			uint32 A = H[0], B = H[1], C = H[2], D = H[3];
			uint32 E = H[4], F = H[5], G = H[6], HH = H[7];

			for (int32 i = 0; i < 64; ++i)
			{
				const uint32 S1e = RightRotate32(E, 6u) ^ RightRotate32(E, 11u) ^ RightRotate32(E, 25u);
				const uint32 Ch  = (E & F) ^ (~E & G);
				const uint32 T1  = HH + S1e + Ch + SHA256_K[i] + W[i];
				const uint32 S0a = RightRotate32(A, 2u) ^ RightRotate32(A, 13u) ^ RightRotate32(A, 22u);
				const uint32 Maj = (A & B) ^ (A & C) ^ (B & C);
				const uint32 T2  = S0a + Maj;
				HH = G; G = F; F = E; E = D + T1;
				D  = C; C = B; B = A; A = T1 + T2;
			}
			H[0] += A; H[1] += B; H[2] += C; H[3] += D;
			H[4] += E; H[5] += F; H[6] += G; H[7] += HH;
		}

		return FString::Printf(
			TEXT("%08x%08x%08x%08x%08x%08x%08x%08x"),
			H[0], H[1], H[2], H[3], H[4], H[5], H[6], H[7]);
	}

	// -------------------------------------------------------------------------
	// Section B: Enum helpers
	// -------------------------------------------------------------------------

	FString EmbeddingJobStatusToString(const EIISEmbeddingJobStatus Status)
	{
		switch (Status)
		{
		case EIISEmbeddingJobStatus::Pending:             return TEXT("Pending");
		case EIISEmbeddingJobStatus::Running:             return TEXT("Running");
		case EIISEmbeddingJobStatus::Completed:           return TEXT("Completed");
		case EIISEmbeddingJobStatus::Skipped:             return TEXT("Skipped");
		case EIISEmbeddingJobStatus::Failed:              return TEXT("Failed");
		case EIISEmbeddingJobStatus::BlockedByGovernance: return TEXT("BlockedByGovernance");
		default:                                          return TEXT("Unknown");
		}
	}

	EIISEmbeddingJobStatus ParseEmbeddingJobStatus(const FString& Value)
	{
		if (Value == TEXT("Pending"))             return EIISEmbeddingJobStatus::Pending;
		if (Value == TEXT("Running"))             return EIISEmbeddingJobStatus::Running;
		if (Value == TEXT("Completed"))           return EIISEmbeddingJobStatus::Completed;
		if (Value == TEXT("Skipped"))             return EIISEmbeddingJobStatus::Skipped;
		if (Value == TEXT("Failed"))              return EIISEmbeddingJobStatus::Failed;
		if (Value == TEXT("BlockedByGovernance")) return EIISEmbeddingJobStatus::BlockedByGovernance;
		return EIISEmbeddingJobStatus::Unknown;
	}

	bool IsGovernanceErrorCode(const FString& ErrorCode)
	{
		return ErrorCode == TEXT("embedding_route_disabled")
			|| ErrorCode == TEXT("embedding_runtime_mode_disabled")
			|| ErrorCode == TEXT("embedding_route_blocked_by_governance")
			|| ErrorCode == TEXT("local_only_violation")
			|| ErrorCode == TEXT("provider_disabled");
	}

	FString SelectRouteForChunk(const EIISChunkKind Kind, TArray<FString>& OutWarnings)
	{
		switch (Kind)
		{
		case EIISChunkKind::Blueprint:
			return TEXT("iis.embedding.blueprint");
		case EIISChunkKind::Asset:
		case EIISChunkKind::Guardrail:
			return TEXT("iis.embedding.asset");
		case EIISChunkKind::Code:
		case EIISChunkKind::Module:
		case EIISChunkKind::Reflection:
		case EIISChunkKind::Network:
		case EIISChunkKind::ArchitectureFinding:
		case EIISChunkKind::CandidateReview:
		case EIISChunkKind::PlanningIntake:
		case EIISChunkKind::RagPreparedChunk:
		case EIISChunkKind::Documentation:
			return TEXT("iis.embedding.code");
		case EIISChunkKind::Unknown:
		default:
			OutWarnings.Add(TEXT("Unknown chunk kind routed through default code embedding route."));
			return TEXT("iis.embedding.code");
		}
	}

	FString ChunkKindToTaskKind(const EIISChunkKind Kind)
	{
		switch (Kind)
		{
		case EIISChunkKind::Blueprint:
			return TEXT("Blueprint");
		case EIISChunkKind::Asset:
		case EIISChunkKind::Guardrail:
			return TEXT("Asset");
		case EIISChunkKind::Documentation:
			return TEXT("Documentation");
		case EIISChunkKind::CandidateReview:
			return TEXT("Review");
		case EIISChunkKind::PlanningIntake:
			return TEXT("Planning");
		case EIISChunkKind::RagPreparedChunk:
			return TEXT("Generic");
		default:
			return TEXT("Code");
		}
	}

	// -------------------------------------------------------------------------
	// Section C: JSON / JSONL helpers
	// -------------------------------------------------------------------------

	bool SaveJsonObjectToFile(const TSharedRef<FJsonObject>& Object, const FString& FilePath)
	{
		FString JsonOutput;
		const TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> Writer =
			TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&JsonOutput);
		if (!FJsonSerializer::Serialize(Object, Writer))
		{
			return false;
		}
		return FFileHelper::SaveStringToFile(
			JsonOutput, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}

	FString ComputeFileSha256(const FString& FilePath)
	{
		TArray<uint8> Bytes;
		if (!FFileHelper::LoadFileToArray(Bytes, *FilePath))
		{
			return FString();
		}

		return SHA256Bytes(Bytes.GetData(), Bytes.Num());
	}

	TArray<TSharedPtr<FJsonValue>> MakeStringJsonArray(const TArray<FString>& Values)
	{
		TArray<TSharedPtr<FJsonValue>> Out;
		for (const FString& V : Values)
		{
			Out.Add(MakeShared<FJsonValueString>(V));
		}
		return Out;
	}

	TSharedRef<FJsonObject> MakeJobJsonObject(const FIISEmbeddingJob& Job)
	{
		TSharedRef<FJsonObject> Obj = MakeShared<FJsonObject>();
		Obj->SetStringField(TEXT("job_id"),         Job.JobId);
		Obj->SetStringField(TEXT("chunk_id"),        Job.ChunkId);
		Obj->SetStringField(TEXT("route_id"),        Job.RouteId);
		Obj->SetStringField(TEXT("task_kind"),       Job.TaskKind);
		Obj->SetStringField(TEXT("status"),          EmbeddingJobStatusToString(Job.Status));
		Obj->SetStringField(TEXT("provider_id"),     Job.ProviderId);
		Obj->SetStringField(TEXT("model_id"),        Job.ModelId);
		Obj->SetNumberField(TEXT("dimensions"),      static_cast<double>(Job.Dimensions));
		Obj->SetStringField(TEXT("error_code"),      Job.ErrorCode);
		Obj->SetStringField(TEXT("error_message"),   Job.ErrorMessage);
		Obj->SetStringField(TEXT("created_at_utc"),  Job.CreatedAtUtc);
		Obj->SetStringField(TEXT("updated_at_utc"),  Job.UpdatedAtUtc);
		Obj->SetStringField(TEXT("text_sha256"),     Job.TextSha256);
		return Obj;
	}

	bool WriteEmbeddingJobsJsonl(const TArray<FIISEmbeddingJob>& Jobs)
	{
		const FString FilePath = FIISStoragePaths::GetEmbeddingsDir() / TEXT("embedding_jobs.jsonl");
		TArray<FString> Lines;
		for (const FIISEmbeddingJob& Job : Jobs)
		{
			FString Line;
			const TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
				TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Line);
			FJsonSerializer::Serialize(MakeJobJsonObject(Job), Writer);
			Lines.Add(Line);
		}
		return FFileHelper::SaveStringArrayToFile(
			Lines, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}

	bool LoadEmbeddingJobsJsonl(TArray<FIISEmbeddingJob>& OutJobs)
	{
		const FString FilePath = FIISStoragePaths::GetEmbeddingsDir() / TEXT("embedding_jobs.jsonl");
		if (!FPaths::FileExists(FilePath))
		{
			return false;
		}

		TArray<FString> Lines;
		if (!FFileHelper::LoadFileToStringArray(Lines, *FilePath))
		{
			return false;
		}

		for (const FString& Line : Lines)
		{
			FString Trimmed = Line.TrimStartAndEnd();
			if (Trimmed.IsEmpty())
			{
				continue;
			}
			TSharedPtr<FJsonObject> Obj;
			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Trimmed);
			if (!FJsonSerializer::Deserialize(Reader, Obj) || !Obj.IsValid())
			{
				continue;
			}
			FIISEmbeddingJob Job;
			Obj->TryGetStringField(TEXT("job_id"),         Job.JobId);
			Obj->TryGetStringField(TEXT("chunk_id"),        Job.ChunkId);
			Obj->TryGetStringField(TEXT("route_id"),        Job.RouteId);
			Obj->TryGetStringField(TEXT("task_kind"),       Job.TaskKind);
			FString StatusStr;
			Obj->TryGetStringField(TEXT("status"),          StatusStr);
			Job.Status = ParseEmbeddingJobStatus(StatusStr);
			Obj->TryGetStringField(TEXT("provider_id"),     Job.ProviderId);
			Obj->TryGetStringField(TEXT("model_id"),        Job.ModelId);
			{
				double DimsDouble = 0.0;
				Obj->TryGetNumberField(TEXT("dimensions"), DimsDouble);
				Job.Dimensions = static_cast<int32>(DimsDouble);
			}
			Obj->TryGetStringField(TEXT("error_code"),      Job.ErrorCode);
			Obj->TryGetStringField(TEXT("error_message"),   Job.ErrorMessage);
			Obj->TryGetStringField(TEXT("created_at_utc"),  Job.CreatedAtUtc);
			Obj->TryGetStringField(TEXT("updated_at_utc"),  Job.UpdatedAtUtc);
			Obj->TryGetStringField(TEXT("text_sha256"),     Job.TextSha256);
			OutJobs.Add(MoveTemp(Job));
		}
		return true;
	}

	struct FVectorRecord
	{
		FString ChunkId;
		FString RouteId;
		FString TextSha256;
		FString ProviderId;
		FString ModelId;
		int32   Dimensions = 0;
	};

	bool LoadExistingVectorRecords(TArray<FVectorRecord>& OutRecords)
	{
		const FString FilePath = FIISStoragePaths::GetVectorsDir() / TEXT("chunk_vectors.jsonl");
		if (!FPaths::FileExists(FilePath))
		{
			return true;
		}

		TArray<FString> Lines;
		if (!FFileHelper::LoadFileToStringArray(Lines, *FilePath))
		{
			return false;
		}

		for (const FString& Line : Lines)
		{
			FString Trimmed = Line.TrimStartAndEnd();
			if (Trimmed.IsEmpty())
			{
				continue;
			}
			TSharedPtr<FJsonObject> Obj;
			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Trimmed);
			if (!FJsonSerializer::Deserialize(Reader, Obj) || !Obj.IsValid())
			{
				continue;
			}
			FVectorRecord Rec;
			Obj->TryGetStringField(TEXT("chunk_id"),    Rec.ChunkId);
			Obj->TryGetStringField(TEXT("route_id"),    Rec.RouteId);
			Obj->TryGetStringField(TEXT("text_sha256"), Rec.TextSha256);
			Obj->TryGetStringField(TEXT("provider_id"), Rec.ProviderId);
			Obj->TryGetStringField(TEXT("model_id"),    Rec.ModelId);
			{
				double DimsDouble = 0.0;
				Obj->TryGetNumberField(TEXT("dimensions"), DimsDouble);
				Rec.Dimensions = static_cast<int32>(DimsDouble);
			}
			if (!Rec.ChunkId.IsEmpty())
			{
				OutRecords.Add(MoveTemp(Rec));
			}
		}
		return true;
	}

	bool RewriteVectorStoreForActiveChunks(
		const TSet<FString>& ActiveChunkIds,
		TArray<FString>& OutRemovedChunkIds)
	{
		OutRemovedChunkIds.Reset();

		const FString FilePath = FIISStoragePaths::GetVectorsDir() / TEXT("chunk_vectors.jsonl");
		if (!FPaths::FileExists(FilePath))
		{
			return true;
		}

		TArray<FString> Lines;
		if (!FFileHelper::LoadFileToStringArray(Lines, *FilePath))
		{
			return false;
		}

		FString Combined;
		for (const FString& Line : Lines)
		{
			FString Trimmed = Line.TrimStartAndEnd();
			if (Trimmed.IsEmpty())
			{
				continue;
			}

			TSharedPtr<FJsonObject> Obj;
			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Trimmed);
			if (!FJsonSerializer::Deserialize(Reader, Obj) || !Obj.IsValid())
			{
				continue;
			}

			FString ChunkId;
			Obj->TryGetStringField(TEXT("chunk_id"), ChunkId);
			if (ActiveChunkIds.Contains(ChunkId))
			{
				Combined += Trimmed + LINE_TERMINATOR;
			}
			else if (!ChunkId.IsEmpty())
			{
				OutRemovedChunkIds.AddUnique(ChunkId);
			}
		}

		if (Combined.IsEmpty())
		{
			return IFileManager::Get().Delete(*FilePath);
		}

		return FFileHelper::SaveStringToFile(
			Combined, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}

	void RemovePrunedChunksFromVectorBackend(const TArray<FString>& RemovedChunkIds)
	{
		if (RemovedChunkIds.Num() == 0)
		{
			return;
		}

		TUniquePtr<IIISVectorIndexBackend> Backend = CreateVectorIndexBackend();
		if (!Backend)
		{
			return;
		}

		for (const FString& ChunkId : RemovedChunkIds)
		{
			Backend->Remove(ChunkId);
		}
	}

	bool AppendVectorRecord(
		const FString& ChunkId,
		const FString& RouteId,
		const FString& TaskKind,
		const FString& ProviderId,
		const FString& ModelId,
		const int32 Dimensions,
		const TArray<float>& Vector,
		const FString& TextSha256,
		const FString& CreatedAtUtc)
	{
		const FString FilePath = FIISStoragePaths::GetVectorsDir() / TEXT("chunk_vectors.jsonl");

		TArray<TSharedPtr<FJsonValue>> VectorValues;
		for (const float Val : Vector)
		{
			VectorValues.Add(MakeShared<FJsonValueNumber>(static_cast<double>(Val)));
		}

		TSharedRef<FJsonObject> Obj = MakeShared<FJsonObject>();
		Obj->SetStringField(TEXT("chunk_id"),        ChunkId);
		Obj->SetStringField(TEXT("route_id"),        RouteId);
		Obj->SetStringField(TEXT("task_kind"),       TaskKind);
		Obj->SetStringField(TEXT("provider_id"),     ProviderId);
		Obj->SetStringField(TEXT("model_id"),        ModelId);
		Obj->SetNumberField(TEXT("dimensions"),      static_cast<double>(Dimensions));
		Obj->SetArrayField(TEXT("vector"),           VectorValues);
		Obj->SetStringField(TEXT("created_at_utc"),  CreatedAtUtc);
		Obj->SetStringField(TEXT("text_sha256"),     TextSha256);

		FString Line;
		const TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
			TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Line);
		if (!FJsonSerializer::Serialize(Obj, Writer))
		{
			return false;
		}
		Line += TEXT("\n");

		IFileManager& FM = IFileManager::Get();
		FArchive* Handle = FM.CreateFileWriter(*FilePath, FILEWRITE_Append | FILEWRITE_AllowRead);
		if (!Handle)
		{
			return false;
		}
		const FTCHARToUTF8 Utf8(*Line);
		Handle->Serialize(const_cast<ANSICHAR*>(Utf8.Get()), Utf8.Length());
		delete Handle;
		return true;
	}

	bool WriteVectorStoreManifest(
		const int32 VectorCount,
		const int32 Dimensions,
		const TArray<FString>& ProviderIds,
		const TArray<FString>& ModelIds,
		const FString& UpdatedAtUtc,
		const TArray<FString>& Warnings)
	{
		const FString FilePath = FIISStoragePaths::GetVectorsDir() / TEXT("vector_store_manifest.json");
		TSharedRef<FJsonObject> Obj = MakeShared<FJsonObject>();
		Obj->SetStringField(TEXT("schema_version"), TEXT("0.1.0"));
		Obj->SetStringField(TEXT("tool_name"),      TEXT("Internal Index Service"));
		Obj->SetStringField(TEXT("updated_at_utc"), UpdatedAtUtc);
		Obj->SetNumberField(TEXT("vector_count"),   static_cast<double>(VectorCount));
		Obj->SetNumberField(TEXT("dimensions"),     static_cast<double>(Dimensions));
		Obj->SetArrayField(TEXT("provider_ids"),    MakeStringJsonArray(ProviderIds));
		Obj->SetArrayField(TEXT("model_ids"),       MakeStringJsonArray(ModelIds));
		Obj->SetStringField(TEXT("sha256_vectors"), ComputeFileSha256(FIISStoragePaths::GetVectorsDir() / TEXT("chunk_vectors.jsonl")));
		Obj->SetArrayField(TEXT("warnings"),        MakeStringJsonArray(Warnings));
		return SaveJsonObjectToFile(Obj, FilePath);
	}

	bool ResolveEmbeddingRouteMetadata(
		const FString& RouteId,
		FString& OutModelId,
		int32& OutDimensions,
		TArray<FString>& OutWarnings)
	{
		if (RouteId.IsEmpty())
		{
			return false;
		}

		FIISEmbeddingRoute Route;
		if (!FIISEmbeddingRouteExecutorRegistry::ResolveEmbeddingRoute(RouteId, Route, OutWarnings))
		{
			return false;
		}

		OutModelId = Route.ModelId;
		OutDimensions = Route.Dimensions;
		return !OutModelId.IsEmpty() || OutDimensions > 0;
	}

	bool WriteEmbeddingRunReportJson(const FIISEmbeddingRunReport& Report)
	{
		TSharedRef<FJsonObject> Obj = MakeShared<FJsonObject>();
		Obj->SetStringField(TEXT("schema_version"),    Report.SchemaVersion);
		Obj->SetStringField(TEXT("tool_name"),         Report.ToolName);
		Obj->SetStringField(TEXT("generated_at_utc"),  Report.GeneratedAtUtc);
		Obj->SetStringField(TEXT("run_id"),            Report.RunId);
		Obj->SetStringField(TEXT("catalog_path"),      Report.CatalogPath);
		Obj->SetStringField(TEXT("vector_store_path"), Report.VectorStorePath);

		TSharedRef<FJsonObject> Summary = MakeShared<FJsonObject>();
		Summary->SetNumberField(TEXT("chunk_count"),                 static_cast<double>(Report.Summary.ChunkCount));
		Summary->SetNumberField(TEXT("job_count"),                   static_cast<double>(Report.Summary.JobCount));
		Summary->SetNumberField(TEXT("completed_count"),             static_cast<double>(Report.Summary.CompletedCount));
		Summary->SetNumberField(TEXT("skipped_count"),               static_cast<double>(Report.Summary.SkippedCount));
		Summary->SetNumberField(TEXT("failed_count"),                static_cast<double>(Report.Summary.FailedCount));
		Summary->SetNumberField(TEXT("blocked_by_governance_count"), static_cast<double>(Report.Summary.BlockedByGovernanceCount));
		Summary->SetNumberField(TEXT("vector_count"),                static_cast<double>(Report.Summary.VectorCount));
		Summary->SetNumberField(TEXT("warning_count"),               static_cast<double>(Report.Summary.WarningCount));
		Summary->SetNumberField(TEXT("error_count"),                 static_cast<double>(Report.Summary.ErrorCount));
		Obj->SetObjectField(TEXT("summary"), Summary);

		TArray<TSharedPtr<FJsonValue>> JobValues;
		for (const FIISEmbeddingJob& Job : Report.Jobs)
		{
			JobValues.Add(MakeShared<FJsonValueObject>(MakeJobJsonObject(Job)));
		}
		Obj->SetArrayField(TEXT("jobs"),     JobValues);
		Obj->SetArrayField(TEXT("warnings"), MakeStringJsonArray(Report.Warnings));
		Obj->SetArrayField(TEXT("errors"),   MakeStringJsonArray(Report.Errors));

		const FString FilePath = FIISStoragePaths::GetEmbeddingsDir() / TEXT("embedding_run_report.json");
		return SaveJsonObjectToFile(Obj, FilePath);
	}

	bool WriteEmbeddingRunReportMarkdown(const FIISEmbeddingRunReport& Report)
	{
		TArray<FString> Lines;
		Lines.Add(TEXT("# IIS Embedding Run Report"));
		Lines.Add(TEXT(""));
		Lines.Add(FString::Printf(TEXT("- GeneratedAtUtc: %s"), *Report.GeneratedAtUtc));
		Lines.Add(FString::Printf(TEXT("- RunId: %s"),           *Report.RunId));
		Lines.Add(FString::Printf(TEXT("- CatalogPath: %s"),     *Report.CatalogPath));
		Lines.Add(FString::Printf(TEXT("- VectorStorePath: %s"), *Report.VectorStorePath));
		Lines.Add(TEXT(""));
		Lines.Add(TEXT("## Summary"));
		Lines.Add(FString::Printf(TEXT("- ChunkCount: %d"),               Report.Summary.ChunkCount));
		Lines.Add(FString::Printf(TEXT("- JobCount: %d"),                  Report.Summary.JobCount));
		Lines.Add(FString::Printf(TEXT("- CompletedCount: %d"),            Report.Summary.CompletedCount));
		Lines.Add(FString::Printf(TEXT("- SkippedCount: %d"),              Report.Summary.SkippedCount));
		Lines.Add(FString::Printf(TEXT("- FailedCount: %d"),               Report.Summary.FailedCount));
		Lines.Add(FString::Printf(TEXT("- BlockedByGovernanceCount: %d"),  Report.Summary.BlockedByGovernanceCount));
		Lines.Add(FString::Printf(TEXT("- VectorCount: %d"),               Report.Summary.VectorCount));
		Lines.Add(FString::Printf(TEXT("- WarningCount: %d"),              Report.Summary.WarningCount));
		Lines.Add(FString::Printf(TEXT("- ErrorCount: %d"),                Report.Summary.ErrorCount));

		if (Report.Warnings.Num() > 0)
		{
			Lines.Add(TEXT(""));
			Lines.Add(TEXT("## Warnings"));
			for (const FString& W : Report.Warnings) { Lines.Add(TEXT("- ") + W); }
		}
		if (Report.Errors.Num() > 0)
		{
			Lines.Add(TEXT(""));
			Lines.Add(TEXT("## Errors"));
			for (const FString& E : Report.Errors) { Lines.Add(TEXT("- ") + E); }
		}
		if (Report.Jobs.Num() > 0)
		{
			Lines.Add(TEXT(""));
			Lines.Add(TEXT("## Jobs"));
			for (const FIISEmbeddingJob& Job : Report.Jobs)
			{
				Lines.Add(FString::Printf(TEXT("- %s | %s | %s | provider=%s | model=%s | dims=%d | %s"),
					*Job.ChunkId, *Job.RouteId, *EmbeddingJobStatusToString(Job.Status),
					*Job.ProviderId, *Job.ModelId, Job.Dimensions,
					Job.ErrorCode.IsEmpty() ? TEXT("ok") : *Job.ErrorCode));
			}
		}

		const FString FilePath = FIISStoragePaths::GetEmbeddingsDir() / TEXT("embedding_run_report.md");
		return FFileHelper::SaveStringArrayToFile(
			Lines, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}

} // anonymous namespace

// =============================================================================
// Public API
// =============================================================================

FString FIISEmbeddingJobQueue::ComputeSHA256(const FString& Text)
{
	const FTCHARToUTF8 Utf8(*Text);
	return SHA256Bytes(
		reinterpret_cast<const uint8*>(Utf8.Get()),
		static_cast<int64>(Utf8.Length()));
}

FString FIISEmbeddingJobQueue::MakeVectorSkipKey(
	const FString& ChunkId,
	const FString& RouteId,
	const FString& TextSha256,
	const FString& ModelId,
	const int32 Dimensions)
{
	return ChunkId + TEXT("|") + RouteId + TEXT("|") + TextSha256
		+ TEXT("|") + ModelId + TEXT("|") + FString::FromInt(Dimensions);
}

bool FIISEmbeddingJobQueue::ShouldIncludeChunkForEmbedding(const FIISIndexChunk& Chunk)
{
	return FIISChunkCatalog::IsActiveLifecycleState(Chunk.LifecycleState);
}

bool FIISEmbeddingJobQueue::BuildEmbeddingJobsFromCatalog(
	FString& OutReportPath,
	TArray<FString>& OutWarnings)
{
	OutReportPath.Reset();
	OutWarnings.Reset();

	FIISStoragePaths::EnsureDefaultFolders();

	const FString CatalogPath = FIISChunkCatalog::GetCatalogPath();
	if (!FPaths::FileExists(CatalogPath))
	{
		OutWarnings.Add(TEXT("IIS catalog does not exist; no embedding jobs created."));
		return false;
	}

	FSQLiteDatabase Database;
	if (!Database.Open(*CatalogPath, ESQLiteDatabaseOpenMode::ReadOnly))
	{
		OutWarnings.Add(FString::Printf(TEXT("Failed to open IIS catalog for embedding build: %s"), *Database.GetLastError()));
		return false;
	}

	TArray<FIISIndexChunk> AllChunks;
	Database.Execute(
		TEXT("SELECT chunk_id, source_chunk_id, chunk_kind, title, text, module_name, source_id, source_run_id, destination_run_id, ")
		TEXT("is_ai_generated, allows_migration_decision, allows_patch_generation, text_sha256, lifecycle_state, source_refs_hash ")
		TEXT("FROM chunks ORDER BY chunk_id;"),
		[&AllChunks](const FSQLitePreparedStatement& Row)
		{
			FIISIndexChunk Chunk;
			FString ChunkKindStr;
			int32 bAi = 0, bMig = 0, bPatch = 0;
			Row.GetColumnValueByName(TEXT("chunk_id"),                  Chunk.ChunkId);
			Row.GetColumnValueByName(TEXT("source_chunk_id"),           Chunk.SourceChunkId);
			Row.GetColumnValueByName(TEXT("chunk_kind"),                ChunkKindStr);
			Row.GetColumnValueByName(TEXT("title"),                     Chunk.Title);
			Row.GetColumnValueByName(TEXT("text"),                      Chunk.Text);
			Row.GetColumnValueByName(TEXT("module_name"),               Chunk.ModuleName);
			Row.GetColumnValueByName(TEXT("source_id"),                 Chunk.SourceId);
			Row.GetColumnValueByName(TEXT("source_run_id"),             Chunk.SourceRunId);
			Row.GetColumnValueByName(TEXT("destination_run_id"),        Chunk.DestinationRunId);
			Row.GetColumnValueByName(TEXT("is_ai_generated"),           bAi);
			Row.GetColumnValueByName(TEXT("allows_migration_decision"), bMig);
			Row.GetColumnValueByName(TEXT("allows_patch_generation"),   bPatch);
			Row.GetColumnValueByName(TEXT("text_sha256"),               Chunk.TextSha256);
			Row.GetColumnValueByName(TEXT("lifecycle_state"),           Chunk.LifecycleState);
			Row.GetColumnValueByName(TEXT("source_refs_hash"),          Chunk.SourceRefsHash);

			const FString Token = ChunkKindStr.ToLower()
				.Replace(TEXT("_"), TEXT(""))
				.Replace(TEXT("-"), TEXT(""));

			if (Token == TEXT("blueprint"))                                                    Chunk.ChunkKind = EIISChunkKind::Blueprint;
			else if (Token == TEXT("asset"))                                                   Chunk.ChunkKind = EIISChunkKind::Asset;
			else if (Token == TEXT("module"))                                                  Chunk.ChunkKind = EIISChunkKind::Module;
			else if (Token == TEXT("reflection"))                                              Chunk.ChunkKind = EIISChunkKind::Reflection;
			else if (Token == TEXT("network"))                                                 Chunk.ChunkKind = EIISChunkKind::Network;
			else if (Token == TEXT("architecturefinding"))                                     Chunk.ChunkKind = EIISChunkKind::ArchitectureFinding;
			else if (Token == TEXT("candidatereview"))                                         Chunk.ChunkKind = EIISChunkKind::CandidateReview;
			else if (Token == TEXT("planningintake") || Token == TEXT("planningintakeitem"))   Chunk.ChunkKind = EIISChunkKind::PlanningIntake;
			else if (Token == TEXT("ragpreparedchunk"))                                        Chunk.ChunkKind = EIISChunkKind::RagPreparedChunk;
			else if (Token == TEXT("documentation"))                                           Chunk.ChunkKind = EIISChunkKind::Documentation;
			else if (Token == TEXT("guardrail") || Token == TEXT("guardrailnotice"))           Chunk.ChunkKind = EIISChunkKind::Guardrail;
			else if (Token == TEXT("code"))                                                    Chunk.ChunkKind = EIISChunkKind::Code;
			else                                                                               Chunk.ChunkKind = EIISChunkKind::Unknown;

			Chunk.bIsAiGenerated           = bAi    != 0;
			Chunk.bAllowsMigrationDecision = bMig   != 0;
			Chunk.bAllowsPatchGeneration   = bPatch != 0;

			if (!Chunk.ChunkId.IsEmpty())
			{
				AllChunks.Add(MoveTemp(Chunk));
			}
			return ESQLitePreparedStatementExecuteRowResult::Continue;
		});
	Database.Close();

	TSet<FString> ActiveChunkIds;
	for (const FIISIndexChunk& Chunk : AllChunks)
	{
		if (ShouldIncludeChunkForEmbedding(Chunk))
		{
			ActiveChunkIds.Add(Chunk.ChunkId);
		}
	}
	TArray<FString> RemovedVectorChunkIds;
	RewriteVectorStoreForActiveChunks(ActiveChunkIds, RemovedVectorChunkIds);
	RemovePrunedChunksFromVectorBackend(RemovedVectorChunkIds);

	const bool bHasEmbeddingExecutor = FIISEmbeddingRouteExecutorRegistry::GetExecutor().IsValid();
	if (!bHasEmbeddingExecutor)
	{
		OutWarnings.Add(TEXT("No IIS embedding route executor is registered; jobs will be built without provider/model metadata."));
	}

	TArray<FVectorRecord> ExistingRecords;
	LoadExistingVectorRecords(ExistingRecords);
	TSet<FString> SkipKeys;
	for (const FVectorRecord& Rec : ExistingRecords)
	{
		if (Rec.Dimensions > 0 && !Rec.ChunkId.IsEmpty() && !Rec.RouteId.IsEmpty() && !Rec.TextSha256.IsEmpty())
		{
			SkipKeys.Add(FIISEmbeddingJobQueue::MakeVectorSkipKey(
				Rec.ChunkId, Rec.RouteId, Rec.TextSha256, Rec.ModelId, Rec.Dimensions));
		}
	}

	const FString NowUtc = FDateTime::UtcNow().ToIso8601();
	TArray<FIISEmbeddingJob> Jobs;

	for (const FIISIndexChunk& Chunk : AllChunks)
	{
		if (!ShouldIncludeChunkForEmbedding(Chunk))
		{
			continue;
		}

		FIISEmbeddingJob Job;
		Job.TextSha256 = Chunk.TextSha256.IsEmpty()
			? FIISEmbeddingJobQueue::ComputeSHA256(Chunk.Text)
			: Chunk.TextSha256;
		Job.RouteId      = SelectRouteForChunk(Chunk.ChunkKind, OutWarnings);
		Job.TaskKind     = ChunkKindToTaskKind(Chunk.ChunkKind);
		Job.ChunkId      = Chunk.ChunkId;
		Job.CreatedAtUtc = NowUtc;
		Job.UpdatedAtUtc = NowUtc;

		if (bHasEmbeddingExecutor)
		{
			ResolveEmbeddingRouteMetadata(Job.RouteId, Job.ModelId, Job.Dimensions, OutWarnings);
		}

		const FString JobIdInput = Chunk.ChunkId
			+ TEXT("|") + Job.RouteId
			+ TEXT("|") + Job.TextSha256
			+ TEXT("|embedding_job_v1");
		Job.JobId = FIISEmbeddingJobQueue::ComputeSHA256(JobIdInput);

		const FString SkipKey = FIISEmbeddingJobQueue::MakeVectorSkipKey(
			Chunk.ChunkId, Job.RouteId, Job.TextSha256, Job.ModelId, Job.Dimensions);
		Job.Status = SkipKeys.Contains(SkipKey) ? EIISEmbeddingJobStatus::Skipped : EIISEmbeddingJobStatus::Pending;
		Jobs.Add(MoveTemp(Job));
	}

	WriteEmbeddingJobsJsonl(Jobs);

	FIISEmbeddingRunReport Report;
	Report.GeneratedAtUtc  = NowUtc;
	Report.RunId           = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
	Report.CatalogPath     = CatalogPath;
	Report.VectorStorePath = FIISStoragePaths::GetVectorsDir() / TEXT("chunk_vectors.jsonl");
	Report.Summary.ChunkCount = AllChunks.Num();
	Report.Summary.JobCount   = Jobs.Num();
	for (const FIISEmbeddingJob& Job : Jobs)
	{
		if (Job.Status == EIISEmbeddingJobStatus::Skipped) { ++Report.Summary.SkippedCount; }
	}
	Report.Summary.WarningCount = OutWarnings.Num();
	Report.Jobs     = Jobs;
	Report.Warnings = OutWarnings;

	WriteEmbeddingRunReportJson(Report);
	WriteEmbeddingRunReportMarkdown(Report);

	OutReportPath = FIISStoragePaths::GetEmbeddingsDir() / TEXT("embedding_run_report.json");
	return true;
}

bool FIISEmbeddingJobQueue::ExecutePendingEmbeddingJobs(
	int32 MaxJobs,
	FString& OutReportPath,
	TArray<FString>& OutWarnings)
{
	OutReportPath.Reset();
	OutWarnings.Reset();

	FIISStoragePaths::EnsureDefaultFolders();

	TArray<FIISEmbeddingJob> Jobs;
	if (!LoadEmbeddingJobsJsonl(Jobs))
	{
		OutWarnings.Add(TEXT("embedding_jobs.jsonl not found; run BuildEmbeddingJobs first."));
		return false;
	}

	TSharedPtr<IIISEmbeddingRouteExecutor> EmbeddingExecutor = FIISEmbeddingRouteExecutorRegistry::GetExecutor();
	if (!EmbeddingExecutor)
	{
		const FString ErrMsg = TEXT("No IIS embedding route executor is registered; install or enable an embedding bridge plugin to execute jobs.");
		int32 MarkedFailed = 0;
		const FString NowFailed = FDateTime::UtcNow().ToIso8601();
		const int32 EffectiveMaxJobs = MaxJobs <= 0 ? Jobs.Num() : MaxJobs;
		for (FIISEmbeddingJob& Job : Jobs)
		{
			if (Job.Status == EIISEmbeddingJobStatus::Pending)
			{
				Job.Status       = EIISEmbeddingJobStatus::Failed;
				Job.ErrorCode    = TEXT("embedding_executor_not_registered");
				Job.ErrorMessage = ErrMsg;
				Job.UpdatedAtUtc = NowFailed;
				++MarkedFailed;
				if (MarkedFailed >= EffectiveMaxJobs) { break; }
			}
		}
		OutWarnings.Add(ErrMsg);
		WriteEmbeddingJobsJsonl(Jobs);

		FIISEmbeddingRunReport FailReport;
		FailReport.GeneratedAtUtc  = NowFailed;
		FailReport.RunId           = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
		FailReport.CatalogPath     = FIISChunkCatalog::GetCatalogPath();
		FailReport.VectorStorePath = FIISStoragePaths::GetVectorsDir() / TEXT("chunk_vectors.jsonl");
		FailReport.Summary.ChunkCount = Jobs.Num();
		FailReport.Summary.JobCount   = Jobs.Num();
		FailReport.Summary.FailedCount = MarkedFailed;
		FailReport.Summary.ErrorCount  = 1;
		FailReport.Jobs     = Jobs;
		FailReport.Errors   = { ErrMsg };
		FailReport.Warnings = OutWarnings;
		WriteEmbeddingRunReportJson(FailReport);
		WriteEmbeddingRunReportMarkdown(FailReport);
		OutReportPath = FIISStoragePaths::GetEmbeddingsDir() / TEXT("embedding_run_report.json");
		return false;
	}

	const FString NowUtc = FDateTime::UtcNow().ToIso8601();
	int32 Executed       = 0;
	int32 FailedThisRun  = 0;
	int32 BlockedThisRun = 0;
	const int32 EffectiveMaxJobs = MaxJobs <= 0 ? Jobs.Num() : MaxJobs;

	TArray<FVectorRecord> ExistingVectorsBeforeRun;
	LoadExistingVectorRecords(ExistingVectorsBeforeRun);
	TSet<FString> ExistingVectorKeys;
	for (const FVectorRecord& Rec : ExistingVectorsBeforeRun)
	{
		if (!Rec.ChunkId.IsEmpty() && !Rec.RouteId.IsEmpty() && !Rec.TextSha256.IsEmpty())
		{
			ExistingVectorKeys.Add(FIISEmbeddingJobQueue::MakeVectorSkipKey(
				Rec.ChunkId, Rec.RouteId, Rec.TextSha256, Rec.ModelId, Rec.Dimensions));
		}
	}

	for (FIISEmbeddingJob& Job : Jobs)
	{
		if (Job.Status != EIISEmbeddingJobStatus::Pending) { continue; }
		if (Job.ModelId.IsEmpty())
		{
			ResolveEmbeddingRouteMetadata(Job.RouteId, Job.ModelId, Job.Dimensions, OutWarnings);
		}
		const FString VectorKey = FIISEmbeddingJobQueue::MakeVectorSkipKey(
			Job.ChunkId, Job.RouteId, Job.TextSha256, Job.ModelId, Job.Dimensions);
		if (ExistingVectorKeys.Contains(VectorKey))
		{
			Job.Status       = EIISEmbeddingJobStatus::Skipped;
			Job.UpdatedAtUtc = NowUtc;
			continue;
		}
		if (Executed >= EffectiveMaxJobs) { break; }
		++Executed;

		FIISIndexChunk Chunk;
		TArray<FString> ChunkWarnings;
		if (!FIISChunkCatalog::LoadChunkById(Job.ChunkId, Chunk, ChunkWarnings))
		{
			Job.Status       = EIISEmbeddingJobStatus::Failed;
			Job.ErrorCode    = TEXT("embedding_chunk_not_found");
			Job.ErrorMessage = FString::Printf(TEXT("Chunk not found in catalog: %s"), *Job.ChunkId);
			Job.UpdatedAtUtc = NowUtc;
			++FailedThisRun;
			OutWarnings.Append(ChunkWarnings);
			continue;
		}

		if (Chunk.Text.IsEmpty())
		{
			Job.Status       = EIISEmbeddingJobStatus::Failed;
			Job.ErrorCode    = TEXT("embedding_input_empty");
			Job.ErrorMessage = FString::Printf(TEXT("Chunk text is empty: %s"), *Job.ChunkId);
			Job.UpdatedAtUtc = NowUtc;
			++FailedThisRun;
			continue;
		}

		FIISEmbeddingRequest Request;
		Request.RouteId   = Job.RouteId;
		Request.TaskKind  = Job.TaskKind;
		Request.InputText = Chunk.Text;
		Request.bLocalOnly = true;
		Request.Metadata.Add(TEXT("chunk_id"),           Job.ChunkId);
		Request.Metadata.Add(TEXT("chunk_kind"),         Job.TaskKind);
		Request.Metadata.Add(TEXT("source_id"),          Chunk.SourceId);
		Request.Metadata.Add(TEXT("source_run_id"),      Chunk.SourceRunId);
		Request.Metadata.Add(TEXT("destination_run_id"), Chunk.DestinationRunId);

		FIISEmbeddingResponse Response;
		EmbeddingExecutor->ExecuteEmbeddingRoute(Request, Response);

		Job.UpdatedAtUtc = FDateTime::UtcNow().ToIso8601();

		if (Response.bSuccess)
		{
			Job.Status     = EIISEmbeddingJobStatus::Completed;
			Job.ProviderId = Response.ProviderId;
			Job.ModelId    = Response.ModelId;
			Job.Dimensions = Response.Dimensions;
			AppendVectorRecord(
				Job.ChunkId, Job.RouteId, Job.TaskKind,
				Response.ProviderId, Response.ModelId, Response.Dimensions,
				Response.Vector, Job.TextSha256, Job.UpdatedAtUtc);
			ExistingVectorKeys.Add(FIISEmbeddingJobQueue::MakeVectorSkipKey(
				Job.ChunkId, Job.RouteId, Job.TextSha256, Job.ModelId, Job.Dimensions));
		}
		else if (IsGovernanceErrorCode(Response.ErrorCode))
		{
			Job.Status       = EIISEmbeddingJobStatus::BlockedByGovernance;
			Job.ErrorCode    = Response.ErrorCode;
			Job.ErrorMessage = Response.ErrorMessage;
			++BlockedThisRun;
		}
		else
		{
			Job.Status       = EIISEmbeddingJobStatus::Failed;
			Job.ErrorCode    = Response.ErrorCode.IsEmpty() ? TEXT("embedding_provider_failed") : Response.ErrorCode;
			Job.ErrorMessage = Response.ErrorMessage;
			++FailedThisRun;
		}
	}

	FIISEmbeddingRunReport Report;
	Report.GeneratedAtUtc  = FDateTime::UtcNow().ToIso8601();
	Report.RunId           = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
	Report.CatalogPath     = FIISChunkCatalog::GetCatalogPath();
	Report.VectorStorePath = FIISStoragePaths::GetVectorsDir() / TEXT("chunk_vectors.jsonl");
	Report.Summary.ChunkCount = Jobs.Num();
	for (const FIISEmbeddingJob& Job : Jobs)
	{
		++Report.Summary.JobCount;
		switch (Job.Status)
		{
		case EIISEmbeddingJobStatus::Completed:           ++Report.Summary.CompletedCount;          break;
		case EIISEmbeddingJobStatus::Skipped:             ++Report.Summary.SkippedCount;             break;
		case EIISEmbeddingJobStatus::Failed:              ++Report.Summary.FailedCount;              break;
		case EIISEmbeddingJobStatus::BlockedByGovernance: ++Report.Summary.BlockedByGovernanceCount; break;
		default: break;
		}
	}
	Report.Summary.WarningCount = OutWarnings.Num();
	Report.Jobs     = Jobs;
	Report.Warnings = OutWarnings;

	WriteEmbeddingJobsJsonl(Jobs);

	TArray<FVectorRecord> AllVecs;
	LoadExistingVectorRecords(AllVecs);
	Report.Summary.VectorCount = AllVecs.Num();
	TArray<FString> ManifestProviders, ManifestModels;
	int32 ManifestDimensions = 0;
	for (const FVectorRecord& V : AllVecs)
	{
		if (!V.ProviderId.IsEmpty()) ManifestProviders.AddUnique(V.ProviderId);
		if (!V.ModelId.IsEmpty())    ManifestModels.AddUnique(V.ModelId);
		if (V.Dimensions > 0)        ManifestDimensions = V.Dimensions;
	}
	WriteVectorStoreManifest(
		AllVecs.Num(), ManifestDimensions,
		ManifestProviders, ManifestModels,
		Report.GeneratedAtUtc, OutWarnings);

	WriteEmbeddingRunReportJson(Report);
	WriteEmbeddingRunReportMarkdown(Report);
	OutReportPath = FIISStoragePaths::GetEmbeddingsDir() / TEXT("embedding_run_report.json");
	return FailedThisRun == 0 && BlockedThisRun == 0;
}

bool FIISEmbeddingJobQueue::BuildAndExecuteEmbeddingJobs(
	int32 MaxJobs,
	FString& OutReportPath,
	TArray<FString>& OutWarnings)
{
	FString BuildReportPath;
	if (!BuildEmbeddingJobsFromCatalog(BuildReportPath, OutWarnings))
	{
		OutReportPath = BuildReportPath;
		return false;
	}
	return ExecutePendingEmbeddingJobs(MaxJobs, OutReportPath, OutWarnings);
}
