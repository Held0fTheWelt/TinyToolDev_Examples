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

#include "IISPreparedChunkImporter.h"

#include "IISStoragePaths.h"
#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
	FString BoolToJsonString(const bool bValue)
	{
		return bValue ? TEXT("true") : TEXT("false");
	}

	FString MakeTimestampForId()
	{
		const FDateTime Now = FDateTime::UtcNow();
		return FString::Printf(
			TEXT("%04d-%02d-%02d_%02d%02d%02d_%03d"),
			Now.GetYear(),
			Now.GetMonth(),
			Now.GetDay(),
			Now.GetHour(),
			Now.GetMinute(),
			Now.GetSecond(),
			Now.GetMillisecond());
	}

	FString NormalizePathForStorage(const FString& Path)
	{
		if (Path.IsEmpty())
		{
			return FString();
		}

		return FPaths::ConvertRelativePathToFull(Path);
	}

	uint32 Sha256RotateRight(const uint32 Value, const uint32 Count)
	{
		return (Value >> Count) | (Value << (32 - Count));
	}

	uint32 Sha256ReadBigEndian32(const TArray<uint8>& Bytes, const int32 Offset)
	{
		return (static_cast<uint32>(Bytes[Offset]) << 24)
			| (static_cast<uint32>(Bytes[Offset + 1]) << 16)
			| (static_cast<uint32>(Bytes[Offset + 2]) << 8)
			| static_cast<uint32>(Bytes[Offset + 3]);
	}

	FString ComputeSha256(const TArray<uint8>& Bytes)
	{
		static constexpr uint32 Constants[64] =
		{
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

		uint32 Hash[8] =
		{
			0x6a09e667,
			0xbb67ae85,
			0x3c6ef372,
			0xa54ff53a,
			0x510e527f,
			0x9b05688c,
			0x1f83d9ab,
			0x5be0cd19
		};

		TArray<uint8> PaddedBytes = Bytes;
		const uint64 BitLength = static_cast<uint64>(Bytes.Num()) * 8ull;
		PaddedBytes.Add(0x80);
		while ((PaddedBytes.Num() % 64) != 56)
		{
			PaddedBytes.Add(0);
		}

		for (int32 Shift = 56; Shift >= 0; Shift -= 8)
		{
			PaddedBytes.Add(static_cast<uint8>((BitLength >> Shift) & 0xff));
		}

		for (int32 ChunkOffset = 0; ChunkOffset < PaddedBytes.Num(); ChunkOffset += 64)
		{
			uint32 Words[64] = {};
			for (int32 Index = 0; Index < 16; ++Index)
			{
				Words[Index] = Sha256ReadBigEndian32(PaddedBytes, ChunkOffset + Index * 4);
			}
			for (int32 Index = 16; Index < 64; ++Index)
			{
				const uint32 S0 = Sha256RotateRight(Words[Index - 15], 7)
					^ Sha256RotateRight(Words[Index - 15], 18)
					^ (Words[Index - 15] >> 3);
				const uint32 S1 = Sha256RotateRight(Words[Index - 2], 17)
					^ Sha256RotateRight(Words[Index - 2], 19)
					^ (Words[Index - 2] >> 10);
				Words[Index] = Words[Index - 16] + S0 + Words[Index - 7] + S1;
			}

			uint32 A = Hash[0];
			uint32 B = Hash[1];
			uint32 C = Hash[2];
			uint32 D = Hash[3];
			uint32 E = Hash[4];
			uint32 F = Hash[5];
			uint32 G = Hash[6];
			uint32 H = Hash[7];

			for (int32 Index = 0; Index < 64; ++Index)
			{
				const uint32 S1 = Sha256RotateRight(E, 6)
					^ Sha256RotateRight(E, 11)
					^ Sha256RotateRight(E, 25);
				const uint32 Choice = (E & F) ^ ((~E) & G);
				const uint32 Temp1 = H + S1 + Choice + Constants[Index] + Words[Index];
				const uint32 S0 = Sha256RotateRight(A, 2)
					^ Sha256RotateRight(A, 13)
					^ Sha256RotateRight(A, 22);
				const uint32 Majority = (A & B) ^ (A & C) ^ (B & C);
				const uint32 Temp2 = S0 + Majority;

				H = G;
				G = F;
				F = E;
				E = D + Temp1;
				D = C;
				C = B;
				B = A;
				A = Temp1 + Temp2;
			}

			Hash[0] += A;
			Hash[1] += B;
			Hash[2] += C;
			Hash[3] += D;
			Hash[4] += E;
			Hash[5] += F;
			Hash[6] += G;
			Hash[7] += H;
		}

		FString HashString;
		for (const uint32 Word : Hash)
		{
			HashString += FString::Printf(TEXT("%08x"), Word);
		}

		return HashString;
	}

	FString ComputeFileSha256(const FString& FilePath)
	{
		TArray<uint8> Bytes;
		if (!FFileHelper::LoadFileToArray(Bytes, *FilePath))
		{
			return FString();
		}

		return ComputeSha256(Bytes);
	}

	FString MakeImportId(const FString& PreparedChunksJsonlPath)
	{
		const FString Hash = FPaths::FileExists(PreparedChunksJsonlPath)
			? ComputeFileSha256(PreparedChunksJsonlPath).Left(12)
			: TEXT("invalid");
		return FString::Printf(TEXT("iis_import_%s_%s"), *MakeTimestampForId(), Hash.IsEmpty() ? TEXT("nohash") : *Hash);
	}

	FString ImportStatusToString(const EIISImportStatus Status)
	{
		switch (Status)
		{
		case EIISImportStatus::Imported:
			return TEXT("Imported");
		case EIISImportStatus::Warning:
			return TEXT("Warning");
		case EIISImportStatus::Failed:
			return TEXT("Failed");
		case EIISImportStatus::Empty:
			return TEXT("Empty");
		case EIISImportStatus::Unknown:
		default:
			return TEXT("Unknown");
		}
	}

	FString ImportSourceFormatToString(const EIISImportSourceFormat SourceFormat)
	{
		switch (SourceFormat)
		{
		case EIISImportSourceFormat::PreparedRagChunksJsonl:
			return TEXT("PreparedRagChunksJsonl");
		case EIISImportSourceFormat::ManualJsonl:
			return TEXT("ManualJsonl");
		case EIISImportSourceFormat::Unknown:
		default:
			return TEXT("Unknown");
		}
	}

	FString ChunkKindToString(const EIISChunkKind ChunkKind)
	{
		switch (ChunkKind)
		{
		case EIISChunkKind::Code:
			return TEXT("Code");
		case EIISChunkKind::Blueprint:
			return TEXT("Blueprint");
		case EIISChunkKind::Asset:
			return TEXT("Asset");
		case EIISChunkKind::Module:
			return TEXT("Module");
		case EIISChunkKind::Reflection:
			return TEXT("Reflection");
		case EIISChunkKind::Network:
			return TEXT("Network");
		case EIISChunkKind::ArchitectureFinding:
			return TEXT("ArchitectureFinding");
		case EIISChunkKind::CandidateReview:
			return TEXT("CandidateReview");
		case EIISChunkKind::PlanningIntake:
			return TEXT("PlanningIntake");
		case EIISChunkKind::RagPreparedChunk:
			return TEXT("RagPreparedChunk");
		case EIISChunkKind::Documentation:
			return TEXT("Documentation");
		case EIISChunkKind::Guardrail:
			return TEXT("Guardrail");
		case EIISChunkKind::Unknown:
		default:
			return TEXT("Unknown");
		}
	}

	FString ChunkSensitivityToString(const EIISChunkSensitivity Sensitivity)
	{
		switch (Sensitivity)
		{
		case EIISChunkSensitivity::PublicProductSafe:
			return TEXT("PublicProductSafe");
		case EIISChunkSensitivity::ProjectLocal:
			return TEXT("ProjectLocal");
		case EIISChunkSensitivity::SourceEvidence:
			return TEXT("SourceEvidence");
		case EIISChunkSensitivity::PrivateReview:
			return TEXT("PrivateReview");
		case EIISChunkSensitivity::Restricted:
			return TEXT("Restricted");
		case EIISChunkSensitivity::Unknown:
		default:
			return TEXT("Unknown");
		}
	}

	FString NormalizeEnumToken(const FString& Value)
	{
		FString Normalized = Value;
		Normalized.ReplaceInline(TEXT("_"), TEXT(""));
		Normalized.ReplaceInline(TEXT("-"), TEXT(""));
		Normalized.ReplaceInline(TEXT(" "), TEXT(""));
		return Normalized.ToLower();
	}

	EIISChunkKind ParseChunkKind(const FString& Value)
	{
		const FString Token = NormalizeEnumToken(Value);
		if (Token == TEXT("code"))
		{
			return EIISChunkKind::Code;
		}
		if (Token == TEXT("blueprint"))
		{
			return EIISChunkKind::Blueprint;
		}
		if (Token == TEXT("asset"))
		{
			return EIISChunkKind::Asset;
		}
		if (Token == TEXT("module"))
		{
			return EIISChunkKind::Module;
		}
		if (Token == TEXT("reflection"))
		{
			return EIISChunkKind::Reflection;
		}
		if (Token == TEXT("network"))
		{
			return EIISChunkKind::Network;
		}
		if (Token == TEXT("architecturefinding"))
		{
			return EIISChunkKind::ArchitectureFinding;
		}
		if (Token == TEXT("candidatereview"))
		{
			return EIISChunkKind::CandidateReview;
		}
		if (Token == TEXT("planningintake") || Token == TEXT("planningintakeitem"))
		{
			return EIISChunkKind::PlanningIntake;
		}
		if (Token == TEXT("ragpreparedchunk"))
		{
			return EIISChunkKind::RagPreparedChunk;
		}
		if (Token == TEXT("documentation"))
		{
			return EIISChunkKind::Documentation;
		}
		if (Token == TEXT("guardrail") || Token == TEXT("guardrailnotice"))
		{
			return EIISChunkKind::Guardrail;
		}

		return EIISChunkKind::Unknown;
	}

	EIISChunkSensitivity ParseSensitivity(const FString& Value)
	{
		const FString Token = NormalizeEnumToken(Value);
		if (Token == TEXT("publicproductsafe"))
		{
			return EIISChunkSensitivity::PublicProductSafe;
		}
		if (Token == TEXT("projectlocal"))
		{
			return EIISChunkSensitivity::ProjectLocal;
		}
		if (Token == TEXT("sourceevidence"))
		{
			return EIISChunkSensitivity::SourceEvidence;
		}
		if (Token == TEXT("privatereview"))
		{
			return EIISChunkSensitivity::PrivateReview;
		}
		if (Token == TEXT("restricted"))
		{
			return EIISChunkSensitivity::Restricted;
		}

		return EIISChunkSensitivity::Unknown;
	}

	FString GetStringField(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName)
	{
		FString Value;
		if (Object.IsValid())
		{
			Object->TryGetStringField(FieldName, Value);
		}
		return Value;
	}

	bool GetBoolField(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName, const bool bDefaultValue = false)
	{
		bool bValue = bDefaultValue;
		if (Object.IsValid())
		{
			Object->TryGetBoolField(FieldName, bValue);
		}
		return bValue;
	}

	TSharedPtr<FJsonObject> LoadJsonObjectFromFile(const FString& FilePath, TArray<FString>* OutWarnings = nullptr)
	{
		if (FilePath.IsEmpty() || !FPaths::FileExists(FilePath))
		{
			return nullptr;
		}

		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *FilePath))
		{
			if (OutWarnings)
			{
				OutWarnings->Add(FString::Printf(TEXT("Failed to read JSON file: %s"), *FilePath));
			}
			return nullptr;
		}

		TSharedPtr<FJsonObject> Object;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		if (!FJsonSerializer::Deserialize(Reader, Object) || !Object.IsValid())
		{
			if (OutWarnings)
			{
				OutWarnings->Add(FString::Printf(TEXT("Failed to parse JSON file: %s"), *FilePath));
			}
			return nullptr;
		}

		return Object;
	}

	bool SaveJsonObjectToFile(const TSharedRef<FJsonObject>& Object, const FString& FilePath)
	{
		FString JsonOutput;
		const TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> Writer =
			TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&JsonOutput);
		if (!FJsonSerializer::Serialize(Object, Writer))
		{
			return false;
		}

		return FFileHelper::SaveStringToFile(JsonOutput, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}

	TArray<TSharedPtr<FJsonValue>> MakeStringArray(const TArray<FString>& Values)
	{
		TArray<TSharedPtr<FJsonValue>> JsonValues;
		for (const FString& Value : Values)
		{
			JsonValues.Add(MakeShared<FJsonValueString>(Value));
		}
		return JsonValues;
	}

	void AddStringArrayFromJsonValueArray(const TArray<TSharedPtr<FJsonValue>>& Values, TArray<FString>& OutValues)
	{
		for (const TSharedPtr<FJsonValue>& Value : Values)
		{
			if (!Value.IsValid())
			{
				continue;
			}

			if (Value->Type == EJson::String)
			{
				OutValues.Add(Value->AsString());
				continue;
			}

			if (Value->Type == EJson::Object)
			{
				const TSharedPtr<FJsonObject> Object = Value->AsObject();
				if (!Object.IsValid())
				{
					continue;
				}

				FString Label;
				if (Object->TryGetStringField(TEXT("label_id"), Label)
					|| Object->TryGetStringField(TEXT("group_id"), Label)
					|| Object->TryGetStringField(TEXT("value"), Label)
					|| Object->TryGetStringField(TEXT("label_kind"), Label))
				{
					OutValues.Add(Label);
				}
			}
		}
	}

	void LoadManifestFieldsIntoChunk(const TSharedPtr<FJsonObject>& ManifestObject, FIISIndexChunk& Chunk)
	{
		if (!ManifestObject.IsValid())
		{
			return;
		}

		Chunk.SourceId = GetStringField(ManifestObject, TEXT("source_id"));
		Chunk.SourceRunId = GetStringField(ManifestObject, TEXT("source_run_id"));
		Chunk.DestinationRunId = GetStringField(ManifestObject, TEXT("destination_run_id"));
	}

	void LoadGuardrails(const TSharedPtr<FJsonObject>& ManifestObject, TArray<FString>& OutGuardrails)
	{
		if (!ManifestObject.IsValid())
		{
			return;
		}

		const TArray<TSharedPtr<FJsonValue>>* GuardrailValues = nullptr;
		if (ManifestObject->TryGetArrayField(TEXT("guardrails"), GuardrailValues))
		{
			AddStringArrayFromJsonValueArray(*GuardrailValues, OutGuardrails);
		}
	}

	FIISSourceReference MakeSourceReferenceFromString(const FString& Value)
	{
		FIISSourceReference Reference;
		TArray<FString> Parts;
		Value.ParseIntoArray(Parts, TEXT("|"), false);
		for (FString& Part : Parts)
		{
			Part.TrimStartAndEndInline();
		}

		if (Parts.Num() >= 4)
		{
			Reference.JsonPointer = Parts[0];
			Reference.Fingerprint = Parts[1];
			Reference.ArtifactKind = Parts[2];
			Reference.RelativePath = Parts[3];
		}
		else
		{
			Reference.Explanation = Value;
		}

		return Reference;
	}

	FIISSourceReference MakeSourceReferenceFromObject(const TSharedPtr<FJsonObject>& Object)
	{
		FIISSourceReference Reference;
		Reference.ArtifactKind = GetStringField(Object, TEXT("artifact_kind"));
		Reference.RelativePath = GetStringField(Object, TEXT("relative_path"));
		Reference.JsonPointer = GetStringField(Object, TEXT("json_pointer"));
		Reference.Fingerprint = GetStringField(Object, TEXT("fingerprint"));
		Reference.Explanation = GetStringField(Object, TEXT("explanation"));
		return Reference;
	}

	void LoadSourceReferences(const TSharedPtr<FJsonObject>& Object, TArray<FIISSourceReference>& OutReferences)
	{
		const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
		if (!Object.IsValid() || !Object->TryGetArrayField(TEXT("source_references"), Values))
		{
			return;
		}

		for (const TSharedPtr<FJsonValue>& Value : *Values)
		{
			if (!Value.IsValid())
			{
				continue;
			}

			if (Value->Type == EJson::String)
			{
				OutReferences.Add(MakeSourceReferenceFromString(Value->AsString()));
			}
			else if (Value->Type == EJson::Object)
			{
				OutReferences.Add(MakeSourceReferenceFromObject(Value->AsObject()));
			}
		}
	}

	FString SerializeChunkToCompactJson(const FIISIndexChunk& Chunk)
	{
		FString JsonLine;
		const TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
			TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonLine);

		Writer->WriteObjectStart();
		Writer->WriteValue(TEXT("chunk_id"), Chunk.ChunkId);
		Writer->WriteValue(TEXT("source_chunk_id"), Chunk.SourceChunkId);
		Writer->WriteValue(TEXT("chunk_kind"), ChunkKindToString(Chunk.ChunkKind));
		Writer->WriteValue(TEXT("sensitivity"), ChunkSensitivityToString(Chunk.Sensitivity));
		Writer->WriteValue(TEXT("title"), Chunk.Title);
		Writer->WriteValue(TEXT("text"), Chunk.Text);
		Writer->WriteValue(TEXT("module_name"), Chunk.ModuleName);
		Writer->WriteValue(TEXT("source_id"), Chunk.SourceId);
		Writer->WriteValue(TEXT("source_run_id"), Chunk.SourceRunId);
		Writer->WriteValue(TEXT("destination_run_id"), Chunk.DestinationRunId);

		Writer->WriteArrayStart(TEXT("retrieval_labels"));
		for (const FString& Label : Chunk.RetrievalLabels)
		{
			Writer->WriteValue(Label);
		}
		Writer->WriteArrayEnd();

		Writer->WriteArrayStart(TEXT("retrieval_groups"));
		for (const FString& Group : Chunk.RetrievalGroups)
		{
			Writer->WriteValue(Group);
		}
		Writer->WriteArrayEnd();

		Writer->WriteArrayStart(TEXT("source_references"));
		for (const FIISSourceReference& Reference : Chunk.SourceReferences)
		{
			Writer->WriteObjectStart();
			Writer->WriteValue(TEXT("artifact_kind"), Reference.ArtifactKind);
			Writer->WriteValue(TEXT("relative_path"), Reference.RelativePath);
			Writer->WriteValue(TEXT("json_pointer"), Reference.JsonPointer);
			Writer->WriteValue(TEXT("fingerprint"), Reference.Fingerprint);
			Writer->WriteValue(TEXT("explanation"), Reference.Explanation);
			Writer->WriteObjectEnd();
		}
		Writer->WriteArrayEnd();

		Writer->WriteValue(TEXT("is_ai_generated"), Chunk.bIsAiGenerated);
		Writer->WriteValue(TEXT("allows_migration_decision"), Chunk.bAllowsMigrationDecision);
		Writer->WriteValue(TEXT("allows_patch_generation"), Chunk.bAllowsPatchGeneration);

		Writer->WriteArrayStart(TEXT("warnings"));
		for (const FString& Warning : Chunk.Warnings)
		{
			Writer->WriteValue(Warning);
		}
		Writer->WriteArrayEnd();

		Writer->WriteObjectEnd();
		Writer->Close();
		return JsonLine;
	}

	FIISChunkImportRecord* FindRecordByChunkId(FIISImportReport& Report, const FString& ChunkId)
	{
		for (FIISChunkImportRecord& Record : Report.Chunks)
		{
			if (Record.ChunkId == ChunkId)
			{
				return &Record;
			}
		}

		return nullptr;
	}

	TSharedRef<FJsonObject> MakeInputFilesObject(const FIISImportInputFiles& InputFiles)
	{
		TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetStringField(TEXT("prepared_chunks_jsonl_path"), InputFiles.PreparedChunksJsonlPath);
		Object->SetStringField(TEXT("prepared_chunks_manifest_path"), InputFiles.PreparedChunksManifestPath);
		Object->SetStringField(TEXT("retrieval_labels_path"), InputFiles.RetrievalLabelsPath);
		Object->SetStringField(TEXT("retrieval_groups_path"), InputFiles.RetrievalGroupsPath);
		return Object;
	}

	TSharedRef<FJsonObject> MakeSummaryObject(const FIISImportedChunkSummary& Summary)
	{
		TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetNumberField(TEXT("source_line_count"), Summary.SourceLineCount);
		Object->SetNumberField(TEXT("parsed_chunk_count"), Summary.ParsedChunkCount);
		Object->SetNumberField(TEXT("imported_chunk_count"), Summary.ImportedChunkCount);
		Object->SetNumberField(TEXT("skipped_chunk_count"), Summary.SkippedChunkCount);
		Object->SetNumberField(TEXT("duplicate_chunk_count"), Summary.DuplicateChunkCount);
		Object->SetNumberField(TEXT("warning_count"), Summary.WarningCount);
		Object->SetNumberField(TEXT("error_count"), Summary.ErrorCount);
		return Object;
	}

	TArray<TSharedPtr<FJsonValue>> MakeChunkRecordArray(const TArray<FIISChunkImportRecord>& Records)
	{
		TArray<TSharedPtr<FJsonValue>> Values;
		for (const FIISChunkImportRecord& Record : Records)
		{
			TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
			Object->SetStringField(TEXT("chunk_id"), Record.ChunkId);
			Object->SetStringField(TEXT("source_chunk_id"), Record.SourceChunkId);
			Object->SetStringField(TEXT("title"), Record.Title);
			Object->SetStringField(TEXT("chunk_kind"), Record.ChunkKind);
			Object->SetStringField(TEXT("sensitivity"), Record.Sensitivity);
			Object->SetBoolField(TEXT("imported"), Record.bImported);
			Object->SetBoolField(TEXT("duplicate"), Record.bDuplicate);
			Object->SetArrayField(TEXT("warnings"), MakeStringArray(Record.Warnings));
			Values.Add(MakeShared<FJsonValueObject>(Object));
		}
		return Values;
	}

	TSharedRef<FJsonObject> MakeImportReportObject(const FIISImportReport& Report)
	{
		TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetStringField(TEXT("schema_version"), Report.SchemaVersion);
		Object->SetStringField(TEXT("tool_name"), Report.ToolName);
		Object->SetStringField(TEXT("generated_at_utc"), Report.GeneratedAtUtc);
		Object->SetStringField(TEXT("import_id"), Report.ImportId);
		Object->SetStringField(TEXT("status"), ImportStatusToString(Report.Status));
		Object->SetStringField(TEXT("source_format"), ImportSourceFormatToString(Report.SourceFormat));
		Object->SetObjectField(TEXT("input_files"), MakeInputFilesObject(Report.InputFiles));
		Object->SetStringField(TEXT("local_import_directory"), Report.LocalImportDirectory);
		Object->SetStringField(TEXT("chunk_store_path"), Report.ChunkStorePath);
		Object->SetObjectField(TEXT("summary"), MakeSummaryObject(Report.Summary));
		Object->SetArrayField(TEXT("chunks"), MakeChunkRecordArray(Report.Chunks));
		Object->SetArrayField(TEXT("guardrails"), MakeStringArray(Report.Guardrails));
		Object->SetArrayField(TEXT("warnings"), MakeStringArray(Report.Warnings));
		Object->SetArrayField(TEXT("errors"), MakeStringArray(Report.Errors));
		return Object;
	}

	bool CopyOptionalInputFile(
		const FString& SourcePath,
		const FString& TargetPath,
		const FString& LogicalName,
		FIISImportReport& Report)
	{
		if (SourcePath.IsEmpty() || !FPaths::FileExists(SourcePath))
		{
			return true;
		}

		if (IFileManager::Get().Copy(*TargetPath, *SourcePath, true, true) != COPY_OK)
		{
			Report.Warnings.Add(FString::Printf(TEXT("Failed to copy %s into import directory: %s"), *LogicalName, *SourcePath));
			return false;
		}

		return true;
	}
}

bool FIISPreparedChunkImporter::ImportPreparedChunks(
	const FIISImportInputFiles& InputFiles,
	FString& OutImportReportPath,
	TArray<FString>& OutWarnings)
{
	OutImportReportPath.Reset();
	OutWarnings.Reset();

	FIISStoragePaths::EnsureDefaultFolders();

	FIISImportReport Report;
	Report.GeneratedAtUtc = FDateTime::UtcNow().ToIso8601();
	Report.SourceFormat = EIISImportSourceFormat::PreparedRagChunksJsonl;
	Report.InputFiles.PreparedChunksJsonlPath = NormalizePathForStorage(InputFiles.PreparedChunksJsonlPath);
	Report.InputFiles.PreparedChunksManifestPath = NormalizePathForStorage(InputFiles.PreparedChunksManifestPath);
	Report.InputFiles.RetrievalLabelsPath = NormalizePathForStorage(InputFiles.RetrievalLabelsPath);
	Report.InputFiles.RetrievalGroupsPath = NormalizePathForStorage(InputFiles.RetrievalGroupsPath);
	Report.ImportId = MakeImportId(Report.InputFiles.PreparedChunksJsonlPath);
	Report.LocalImportDirectory = FIISStoragePaths::GetImportsDir() / Report.ImportId;
	Report.ChunkStorePath = FIISStoragePaths::GetChunksDir() / TEXT("chunk_store.jsonl");

	IFileManager::Get().MakeDirectory(*Report.LocalImportDirectory, true);

	const bool bInputsValid = ValidateInputFiles(Report.InputFiles, Report);
	const TSharedPtr<FJsonObject> PreparedManifestObject = LoadJsonObjectFromFile(Report.InputFiles.PreparedChunksManifestPath, &Report.Warnings);
	LoadGuardrails(PreparedManifestObject, Report.Guardrails);

	CopyOptionalInputFile(
		Report.InputFiles.PreparedChunksManifestPath,
		Report.LocalImportDirectory / TEXT("source_prepared_manifest_copy.json"),
		TEXT("prepared chunks manifest"),
		Report);
	CopyOptionalInputFile(
		Report.InputFiles.RetrievalLabelsPath,
		Report.LocalImportDirectory / TEXT("source_retrieval_labels_copy.json"),
		TEXT("retrieval labels"),
		Report);
	CopyOptionalInputFile(
		Report.InputFiles.RetrievalGroupsPath,
		Report.LocalImportDirectory / TEXT("source_retrieval_groups_copy.json"),
		TEXT("retrieval groups"),
		Report);

	if (bInputsValid)
	{
		TArray<FIISIndexChunk> ImportedCandidates;
		TMap<FString, FString> ChunkJsonById;
		LoadPreparedChunksJsonl(Report.InputFiles, Report, ImportedCandidates, ChunkJsonById);

		const FString ImportedChunksDirectory = FIISStoragePaths::GetChunkImportsDir() / Report.ImportId;
		IFileManager::Get().MakeDirectory(*ImportedChunksDirectory, true);
		const FString ImportedChunksPath = ImportedChunksDirectory / TEXT("imported_chunks.jsonl");
		AppendImportedChunksToStore(ImportedCandidates, ChunkJsonById, Report.ChunkStorePath, ImportedChunksPath, Report);
	}

	Report.Summary.WarningCount = Report.Warnings.Num();
	Report.Summary.ErrorCount = Report.Errors.Num();

	if (!bInputsValid)
	{
		Report.Status = Report.InputFiles.PreparedChunksJsonlPath.IsEmpty() ? EIISImportStatus::Empty : EIISImportStatus::Failed;
	}
	else if (Report.Summary.ImportedChunkCount > 0)
	{
		Report.Status = (Report.Summary.WarningCount > 0 || Report.Summary.ErrorCount > 0)
			? EIISImportStatus::Warning
			: EIISImportStatus::Imported;
	}
	else if (Report.Summary.DuplicateChunkCount > 0 && Report.Summary.ErrorCount == 0)
	{
		Report.Status = EIISImportStatus::Warning;
	}
	else if (Report.Summary.ParsedChunkCount == 0 && Report.Summary.SourceLineCount == 0)
	{
		Report.Status = EIISImportStatus::Empty;
	}
	else
	{
		Report.Status = EIISImportStatus::Failed;
	}

	WriteImportManifest(Report);

	OutImportReportPath = Report.LocalImportDirectory / TEXT("import_report.json");
	WriteImportReportJson(Report, OutImportReportPath);
	WriteImportReportMarkdown(Report, Report.LocalImportDirectory / TEXT("import_report.md"));
	WriteChunkStoreManifest(Report, Report.ChunkStorePath);

	OutWarnings.Append(Report.Warnings);
	OutWarnings.Append(Report.Errors);

	return Report.Status == EIISImportStatus::Imported || Report.Status == EIISImportStatus::Warning;
}

bool FIISPreparedChunkImporter::ValidateInputFiles(
	const FIISImportInputFiles& InputFiles,
	FIISImportReport& Report)
{
	bool bValid = true;
	if (InputFiles.PreparedChunksJsonlPath.IsEmpty())
	{
		Report.Errors.Add(TEXT("Prepared chunks JSONL path is empty."));
		return false;
	}

	if (!FPaths::FileExists(InputFiles.PreparedChunksJsonlPath))
	{
		Report.Errors.Add(FString::Printf(TEXT("Prepared chunks JSONL file does not exist: %s"), *InputFiles.PreparedChunksJsonlPath));
		bValid = false;
	}

	if (!InputFiles.PreparedChunksManifestPath.IsEmpty() && !FPaths::FileExists(InputFiles.PreparedChunksManifestPath))
	{
		Report.Warnings.Add(FString::Printf(TEXT("Prepared chunks manifest file does not exist: %s"), *InputFiles.PreparedChunksManifestPath));
	}

	if (!InputFiles.RetrievalLabelsPath.IsEmpty() && !FPaths::FileExists(InputFiles.RetrievalLabelsPath))
	{
		Report.Warnings.Add(FString::Printf(TEXT("Retrieval labels file does not exist: %s"), *InputFiles.RetrievalLabelsPath));
	}

	if (!InputFiles.RetrievalGroupsPath.IsEmpty() && !FPaths::FileExists(InputFiles.RetrievalGroupsPath))
	{
		Report.Warnings.Add(FString::Printf(TEXT("Retrieval groups file does not exist: %s"), *InputFiles.RetrievalGroupsPath));
	}

	return bValid;
}

bool FIISPreparedChunkImporter::LoadPreparedChunksJsonl(
	const FIISImportInputFiles& InputFiles,
	FIISImportReport& Report,
	TArray<FIISIndexChunk>& OutChunks,
	TMap<FString, FString>& OutChunkJsonById)
{
	TArray<FString> Lines;
	if (!FFileHelper::LoadFileToStringArray(Lines, *InputFiles.PreparedChunksJsonlPath))
	{
		Report.Errors.Add(FString::Printf(TEXT("Failed to read prepared chunks JSONL: %s"), *InputFiles.PreparedChunksJsonlPath));
		return false;
	}

	const TSharedPtr<FJsonObject> PreparedManifestObject = LoadJsonObjectFromFile(InputFiles.PreparedChunksManifestPath, &Report.Warnings);
	TSet<FString> SeenChunkIds;

	for (int32 LineIndex = 0; LineIndex < Lines.Num(); ++LineIndex)
	{
		FString Line = Lines[LineIndex];
		Line.TrimStartAndEndInline();
		if (Line.IsEmpty())
		{
			continue;
		}

		++Report.Summary.SourceLineCount;

		TSharedPtr<FJsonObject> ChunkObject;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Line);
		if (!FJsonSerializer::Deserialize(Reader, ChunkObject) || !ChunkObject.IsValid())
		{
			++Report.Summary.SkippedChunkCount;
			Report.Errors.Add(FString::Printf(TEXT("Line %d is not a valid JSON object."), LineIndex + 1));
			continue;
		}

		++Report.Summary.ParsedChunkCount;

		FIISIndexChunk Chunk;
		FIISChunkImportRecord Record;
		TArray<FString> ConversionErrors;
		const bool bConverted = ConvertPreparedChunkJsonToIndexChunk(
			ChunkObject,
			PreparedManifestObject,
			LineIndex + 1,
			Chunk,
			Record,
			ConversionErrors);

		if (SeenChunkIds.Contains(Record.ChunkId))
		{
			Record.bDuplicate = true;
			Record.Warnings.Add(FString::Printf(TEXT("Duplicate chunk id inside current import: %s"), *Record.ChunkId));
			Report.Warnings.Append(Record.Warnings);
			Report.Chunks.Add(Record);
			++Report.Summary.DuplicateChunkCount;
			++Report.Summary.SkippedChunkCount;
			continue;
		}

		Report.Chunks.Add(Record);

		if (!bConverted)
		{
			Report.Errors.Append(ConversionErrors);
			++Report.Summary.SkippedChunkCount;
			continue;
		}

		SeenChunkIds.Add(Chunk.ChunkId);
		Report.Warnings.Append(Chunk.Warnings);
		OutChunkJsonById.Add(Chunk.ChunkId, SerializeChunkToCompactJson(Chunk));
		OutChunks.Add(MoveTemp(Chunk));
	}

	return Report.Errors.Num() == 0 || OutChunks.Num() > 0;
}

bool FIISPreparedChunkImporter::ConvertPreparedChunkJsonToIndexChunk(
	const TSharedPtr<FJsonObject>& ChunkObject,
	const TSharedPtr<FJsonObject>& PreparedManifestObject,
	const int32 SourceLineNumber,
	FIISIndexChunk& OutChunk,
	FIISChunkImportRecord& OutRecord,
	TArray<FString>& OutErrors)
{
	OutChunk = FIISIndexChunk();
	OutRecord = FIISChunkImportRecord();

	OutChunk.ChunkId = GetStringField(ChunkObject, TEXT("prepared_chunk_id"));
	OutChunk.SourceChunkId = GetStringField(ChunkObject, TEXT("source_chunk_id"));
	OutChunk.Title = GetStringField(ChunkObject, TEXT("title"));
	OutChunk.Text = GetStringField(ChunkObject, TEXT("prepared_text"));
	LoadManifestFieldsIntoChunk(PreparedManifestObject, OutChunk);

	const FString ChunkKind = GetStringField(ChunkObject, TEXT("chunk_kind"));
	const FString Sensitivity = GetStringField(ChunkObject, TEXT("sensitivity"));
	OutChunk.ChunkKind = ParseChunkKind(ChunkKind);
	OutChunk.Sensitivity = ParseSensitivity(Sensitivity);

	OutRecord.ChunkId = OutChunk.ChunkId;
	OutRecord.SourceChunkId = OutChunk.SourceChunkId;
	OutRecord.Title = OutChunk.Title;
	OutRecord.ChunkKind = ChunkKind;
	OutRecord.Sensitivity = Sensitivity;

	if (OutChunk.ChunkId.IsEmpty())
	{
		OutErrors.Add(FString::Printf(TEXT("Line %d has no prepared_chunk_id."), SourceLineNumber));
		return false;
	}

	if (OutChunk.Text.IsEmpty())
	{
		OutRecord.Warnings.Add(FString::Printf(TEXT("Chunk %s has empty prepared_text."), *OutChunk.ChunkId));
	}

	if (!ChunkKind.IsEmpty() && OutChunk.ChunkKind == EIISChunkKind::Unknown)
	{
		OutRecord.Warnings.Add(FString::Printf(TEXT("Unknown chunk_kind '%s' for chunk %s."), *ChunkKind, *OutChunk.ChunkId));
	}

	if (!Sensitivity.IsEmpty() && OutChunk.Sensitivity == EIISChunkSensitivity::Unknown)
	{
		OutRecord.Warnings.Add(FString::Printf(TEXT("Unknown sensitivity '%s' for chunk %s."), *Sensitivity, *OutChunk.ChunkId));
	}

	const TArray<TSharedPtr<FJsonValue>>* Labels = nullptr;
	if (ChunkObject->TryGetArrayField(TEXT("retrieval_labels"), Labels))
	{
		AddStringArrayFromJsonValueArray(*Labels, OutChunk.RetrievalLabels);
	}

	const TArray<TSharedPtr<FJsonValue>>* Groups = nullptr;
	if (ChunkObject->TryGetArrayField(TEXT("retrieval_groups"), Groups))
	{
		AddStringArrayFromJsonValueArray(*Groups, OutChunk.RetrievalGroups);
	}

	LoadSourceReferences(ChunkObject, OutChunk.SourceReferences);

	OutChunk.bIsAiGenerated = GetBoolField(ChunkObject, TEXT("is_ai_generated"));
	OutChunk.bAllowsMigrationDecision = GetBoolField(ChunkObject, TEXT("allows_migration_decision"));
	OutChunk.bAllowsPatchGeneration = GetBoolField(ChunkObject, TEXT("allows_patch_generation"));

	const TArray<TSharedPtr<FJsonValue>>* Warnings = nullptr;
	if (ChunkObject->TryGetArrayField(TEXT("warnings"), Warnings))
	{
		AddStringArrayFromJsonValueArray(*Warnings, OutChunk.Warnings);
		OutRecord.Warnings.Append(OutChunk.Warnings);
	}

	if (OutChunk.bIsAiGenerated || OutChunk.bAllowsMigrationDecision || OutChunk.bAllowsPatchGeneration)
	{
		OutRecord.Warnings.Add(FString::Printf(
			TEXT("Chunk %s failed IIS guardrails: is_ai_generated=%s, allows_migration_decision=%s, allows_patch_generation=%s."),
			*OutChunk.ChunkId,
			*BoolToJsonString(OutChunk.bIsAiGenerated),
			*BoolToJsonString(OutChunk.bAllowsMigrationDecision),
			*BoolToJsonString(OutChunk.bAllowsPatchGeneration)));
		OutErrors.Append(OutRecord.Warnings);
		return false;
	}

	OutChunk.Warnings.Append(OutRecord.Warnings);
	return true;
}

bool FIISPreparedChunkImporter::LoadExistingChunkIds(
	const FString& ChunkStorePath,
	TSet<FString>& OutExistingChunkIds,
	TMap<FString, FString>& OutExistingChunkJsonById,
	TArray<FString>& OutWarnings)
{
	if (!FPaths::FileExists(ChunkStorePath))
	{
		return true;
	}

	TArray<FString> Lines;
	if (!FFileHelper::LoadFileToStringArray(Lines, *ChunkStorePath))
	{
		OutWarnings.Add(FString::Printf(TEXT("Failed to read existing chunk store: %s"), *ChunkStorePath));
		return false;
	}

	for (int32 LineIndex = 0; LineIndex < Lines.Num(); ++LineIndex)
	{
		FString Line = Lines[LineIndex];
		Line.TrimStartAndEndInline();
		if (Line.IsEmpty())
		{
			continue;
		}

		TSharedPtr<FJsonObject> Object;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Line);
		if (!FJsonSerializer::Deserialize(Reader, Object) || !Object.IsValid())
		{
			OutWarnings.Add(FString::Printf(TEXT("Existing chunk store line %d is invalid JSON and was ignored."), LineIndex + 1));
			continue;
		}

		const FString ChunkId = GetStringField(Object, TEXT("chunk_id"));
		if (ChunkId.IsEmpty())
		{
			OutWarnings.Add(FString::Printf(TEXT("Existing chunk store line %d has no chunk_id and was ignored."), LineIndex + 1));
			continue;
		}

		OutExistingChunkIds.Add(ChunkId);
		OutExistingChunkJsonById.Add(ChunkId, Line);
	}

	return true;
}

bool FIISPreparedChunkImporter::AppendImportedChunksToStore(
	const TArray<FIISIndexChunk>& Chunks,
	const TMap<FString, FString>& ChunkJsonById,
	const FString& ChunkStorePath,
	const FString& ImportedChunksPath,
	FIISImportReport& Report)
{
	TSet<FString> ExistingChunkIds;
	TMap<FString, FString> ExistingChunkJsonById;
	LoadExistingChunkIds(ChunkStorePath, ExistingChunkIds, ExistingChunkJsonById, Report.Warnings);

	TArray<FString> LinesToAppend;
	for (const FIISIndexChunk& Chunk : Chunks)
	{
		const FString* ChunkJson = ChunkJsonById.Find(Chunk.ChunkId);
		FIISChunkImportRecord* Record = FindRecordByChunkId(Report, Chunk.ChunkId);
		if (!ChunkJson)
		{
			Report.Errors.Add(FString::Printf(TEXT("Internal IIS import error: no serialized chunk JSON for %s."), *Chunk.ChunkId));
			++Report.Summary.SkippedChunkCount;
			continue;
		}

		if (ExistingChunkIds.Contains(Chunk.ChunkId))
		{
			++Report.Summary.DuplicateChunkCount;
			++Report.Summary.SkippedChunkCount;
			const FString* ExistingJson = ExistingChunkJsonById.Find(Chunk.ChunkId);
			const bool bConflict = ExistingJson && !ExistingJson->Equals(*ChunkJson, ESearchCase::CaseSensitive);
			const FString Warning = bConflict
				? FString::Printf(TEXT("Conflicting duplicate chunk id retained without overwrite: %s"), *Chunk.ChunkId)
				: FString::Printf(TEXT("Duplicate chunk id retained without re-import: %s"), *Chunk.ChunkId);
			Report.Warnings.Add(Warning);
			if (Record)
			{
				Record->bDuplicate = true;
				Record->Warnings.Add(Warning);
			}
			continue;
		}

		LinesToAppend.Add(*ChunkJson);
		ExistingChunkIds.Add(Chunk.ChunkId);
		ExistingChunkJsonById.Add(Chunk.ChunkId, *ChunkJson);
		++Report.Summary.ImportedChunkCount;
		if (Record)
		{
			Record->bImported = true;
		}
	}

	FString ImportedChunksPayload;
	if (LinesToAppend.Num() > 0)
	{
		ImportedChunksPayload = FString::Join(LinesToAppend, LINE_TERMINATOR);
		ImportedChunksPayload += LINE_TERMINATOR;
	}

	FFileHelper::SaveStringToFile(
		ImportedChunksPayload,
		*ImportedChunksPath,
		FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);

	if (LinesToAppend.Num() == 0)
	{
		return true;
	}

	return FFileHelper::SaveStringToFile(
		ImportedChunksPayload,
		*ChunkStorePath,
		FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM,
		&IFileManager::Get(),
		FILEWRITE_Append);
}

bool FIISPreparedChunkImporter::WriteImportManifest(const FIISImportReport& Report)
{
	TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
	Object->SetStringField(TEXT("schema_version"), Report.SchemaVersion);
	Object->SetStringField(TEXT("tool_name"), Report.ToolName);
	Object->SetStringField(TEXT("generated_at_utc"), Report.GeneratedAtUtc);
	Object->SetStringField(TEXT("import_id"), Report.ImportId);
	Object->SetStringField(TEXT("source_format"), ImportSourceFormatToString(Report.SourceFormat));
	Object->SetObjectField(TEXT("input_files"), MakeInputFilesObject(Report.InputFiles));
	Object->SetStringField(TEXT("local_import_directory"), Report.LocalImportDirectory);
	Object->SetStringField(TEXT("chunk_store_path"), Report.ChunkStorePath);
	Object->SetArrayField(TEXT("guardrails"), MakeStringArray(Report.Guardrails));
	return SaveJsonObjectToFile(Object, Report.LocalImportDirectory / TEXT("import_manifest.json"));
}

bool FIISPreparedChunkImporter::WriteImportReportJson(
	const FIISImportReport& Report,
	const FString& ReportPath)
{
	return SaveJsonObjectToFile(MakeImportReportObject(Report), ReportPath);
}

bool FIISPreparedChunkImporter::WriteImportReportMarkdown(
	const FIISImportReport& Report,
	const FString& ReportPath)
{
	TArray<FString> Lines;
	Lines.Add(TEXT("# IIS Prepared Chunks Import Report"));
	Lines.Add(TEXT(""));
	Lines.Add(FString::Printf(TEXT("- ImportId: %s"), *Report.ImportId));
	Lines.Add(FString::Printf(TEXT("- GeneratedAtUtc: %s"), *Report.GeneratedAtUtc));
	Lines.Add(FString::Printf(TEXT("- Status: %s"), *ImportStatusToString(Report.Status)));
	Lines.Add(FString::Printf(TEXT("- SourceFormat: %s"), *ImportSourceFormatToString(Report.SourceFormat)));
	Lines.Add(FString::Printf(TEXT("- PreparedChunksJsonl: %s"), *Report.InputFiles.PreparedChunksJsonlPath));
	Lines.Add(FString::Printf(TEXT("- LocalImportDirectory: %s"), *Report.LocalImportDirectory));
	Lines.Add(FString::Printf(TEXT("- ChunkStore: %s"), *Report.ChunkStorePath));
	Lines.Add(TEXT(""));
	Lines.Add(TEXT("## Summary"));
	Lines.Add(FString::Printf(TEXT("- SourceLineCount: %d"), Report.Summary.SourceLineCount));
	Lines.Add(FString::Printf(TEXT("- ParsedChunkCount: %d"), Report.Summary.ParsedChunkCount));
	Lines.Add(FString::Printf(TEXT("- ImportedChunkCount: %d"), Report.Summary.ImportedChunkCount));
	Lines.Add(FString::Printf(TEXT("- SkippedChunkCount: %d"), Report.Summary.SkippedChunkCount));
	Lines.Add(FString::Printf(TEXT("- DuplicateChunkCount: %d"), Report.Summary.DuplicateChunkCount));
	Lines.Add(FString::Printf(TEXT("- WarningCount: %d"), Report.Summary.WarningCount));
	Lines.Add(FString::Printf(TEXT("- ErrorCount: %d"), Report.Summary.ErrorCount));

	if (Report.Guardrails.Num() > 0)
	{
		Lines.Add(TEXT(""));
		Lines.Add(TEXT("## Guardrails"));
		for (const FString& Guardrail : Report.Guardrails)
		{
			Lines.Add(FString::Printf(TEXT("- %s"), *Guardrail));
		}
	}

	if (Report.Warnings.Num() > 0)
	{
		Lines.Add(TEXT(""));
		Lines.Add(TEXT("## Warnings"));
		for (const FString& Warning : Report.Warnings)
		{
			Lines.Add(FString::Printf(TEXT("- %s"), *Warning));
		}
	}

	if (Report.Errors.Num() > 0)
	{
		Lines.Add(TEXT(""));
		Lines.Add(TEXT("## Errors"));
		for (const FString& Error : Report.Errors)
		{
			Lines.Add(FString::Printf(TEXT("- %s"), *Error));
		}
	}

	return FFileHelper::SaveStringArrayToFile(Lines, *ReportPath);
}

bool FIISPreparedChunkImporter::WriteChunkStoreManifest(
	const FIISImportReport& Report,
	const FString& ChunkStorePath)
{
	const FString ManifestPath = FIISStoragePaths::GetChunksDir() / TEXT("chunk_store_manifest.json");
	TArray<FString> Warnings;
	TArray<FString> ImportIds;

	const TSharedPtr<FJsonObject> ExistingManifest = LoadJsonObjectFromFile(ManifestPath, &Warnings);
	if (ExistingManifest.IsValid())
	{
		const TArray<TSharedPtr<FJsonValue>>* ExistingImportIds = nullptr;
		if (ExistingManifest->TryGetArrayField(TEXT("import_ids"), ExistingImportIds))
		{
			AddStringArrayFromJsonValueArray(*ExistingImportIds, ImportIds);
		}
	}

	if (!ImportIds.Contains(Report.ImportId))
	{
		ImportIds.Add(Report.ImportId);
	}

	TSet<FString> ExistingChunkIds;
	TMap<FString, FString> ExistingChunkJsonById;
	LoadExistingChunkIds(ChunkStorePath, ExistingChunkIds, ExistingChunkJsonById, Warnings);

	TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
	Object->SetStringField(TEXT("schema_version"), TEXT("0.1.0"));
	Object->SetStringField(TEXT("tool_name"), TEXT("Internal Index Service"));
	Object->SetStringField(TEXT("updated_at_utc"), FDateTime::UtcNow().ToIso8601());
	Object->SetStringField(TEXT("chunk_store_path"), ChunkStorePath);
	Object->SetNumberField(TEXT("chunk_count"), ExistingChunkIds.Num());
	Object->SetNumberField(TEXT("import_count"), ImportIds.Num());
	Object->SetStringField(TEXT("sha256_chunk_store"), FPaths::FileExists(ChunkStorePath) ? ComputeFileSha256(ChunkStorePath) : FString());
	Object->SetArrayField(TEXT("import_ids"), MakeStringArray(ImportIds));
	Object->SetArrayField(TEXT("warnings"), MakeStringArray(Warnings));

	return SaveJsonObjectToFile(Object, ManifestPath);
}
