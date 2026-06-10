/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA
 *
 * This file is part of the "SmartContentDiet IIS Similarity Bridge" Unreal Engine plugin.
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

#include "SCDHealthEvidenceJsonlTranslator.h"

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
FString NormalizeMappingToken(const FString& Value)
{
	FString Token = Value;
	Token.ReplaceInline(TEXT("_"), TEXT(""));
	Token.ReplaceInline(TEXT("-"), TEXT(""));
	Token.ReplaceInline(TEXT("."), TEXT(""));
	Token.ReplaceInline(TEXT(" "), TEXT(""));
	return Token.ToLower();
}

// Map SmartContentDiet's own health vocabulary onto IIS-recognized tokens so
// imported health evidence lands with a real chunk_kind/sensitivity instead of
// IIS 'Unknown' (which warns on every chunk and hides it from labelled retrieval).
FString MapScdChunkKindToIisToken(const FString& ScdKind)
{
	const FString Token = NormalizeMappingToken(ScdKind);
	if (Token == TEXT("scdsummary"))
	{
		return TEXT("Documentation");
	}
	// scd.finding and any other content-health evidence are finding-class.
	return TEXT("ArchitectureFinding");
}

FString MapScdSensitivityToIisToken(const FString& ScdSensitivity)
{
	const FString Token = NormalizeMappingToken(ScdSensitivity);
	if (Token == TEXT("publicproductsafe")) { return TEXT("PublicProductSafe"); }
	if (Token == TEXT("projectlocal")) { return TEXT("ProjectLocal"); }
	if (Token == TEXT("sourceevidence")) { return TEXT("SourceEvidence"); }
	if (Token == TEXT("privatereview")) { return TEXT("PrivateReview"); }
	if (Token == TEXT("restricted")) { return TEXT("Restricted"); }
	// 'internal' and any unknown/empty value default to project-local.
	return TEXT("ProjectLocal");
}

static bool TranslateScdLineToPreparedLine(
	const FString& Line,
	const int32 LineNumber,
	FString& OutPreparedLine,
	FString& OutError)
{
	FString Trimmed = Line;
	Trimmed.TrimStartAndEndInline();
	if (Trimmed.IsEmpty())
	{
		return true;
	}

	TSharedPtr<FJsonObject> ScdObject;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Trimmed);
	if (!FJsonSerializer::Deserialize(Reader, ScdObject) || !ScdObject.IsValid())
	{
		OutError = FString::Printf(TEXT("JSON parse failed at line %d."), LineNumber);
		return false;
	}

	FString ChunkId;
	if (!ScdObject->TryGetStringField(TEXT("chunk_id"), ChunkId) || ChunkId.IsEmpty())
	{
		OutError = FString::Printf(TEXT("Line %d is missing chunk_id."), LineNumber);
		return false;
	}

	FString Content;
	if (!ScdObject->TryGetStringField(TEXT("content"), Content))
	{
		OutError = FString::Printf(TEXT("Line %d is missing content."), LineNumber);
		return false;
	}

	const TSharedRef<FJsonObject> PreparedObject = MakeShared<FJsonObject>();
	PreparedObject->SetStringField(TEXT("prepared_chunk_id"), ChunkId);
	PreparedObject->SetStringField(TEXT("prepared_text"), Content);
	PreparedObject->SetStringField(TEXT("source_chunk_id"), ChunkId);

	FString Title;
	if (ScdObject->TryGetStringField(TEXT("title"), Title))
	{
		PreparedObject->SetStringField(TEXT("title"), Title);
	}

	FString ChunkKind;
	ScdObject->TryGetStringField(TEXT("chunk_kind"), ChunkKind);
	PreparedObject->SetStringField(TEXT("chunk_kind"), MapScdChunkKindToIisToken(ChunkKind));

	FString Sensitivity;
	ScdObject->TryGetStringField(TEXT("sensitivity"), Sensitivity);
	PreparedObject->SetStringField(TEXT("sensitivity"), MapScdSensitivityToIisToken(Sensitivity));

	const TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
		TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&OutPreparedLine);
	if (!FJsonSerializer::Serialize(PreparedObject, Writer))
	{
		OutError = FString::Printf(TEXT("JSON serialize failed at line %d."), LineNumber);
		return false;
	}

	return true;
}
}

bool SCDHealthEvidenceJsonlTranslator::TranslateScdHealthJsonlToPreparedJsonl(
	const FString& ScdJsonlPath,
	const FString& OutPreparedJsonlPath,
	FString& OutError)
{
	FString ScdText;
	if (!FFileHelper::LoadFileToString(ScdText, *ScdJsonlPath))
	{
		OutError = FString::Printf(TEXT("JSONL not found or unreadable: %s"), *ScdJsonlPath);
		return false;
	}

	TArray<FString> Lines;
	ScdText.ParseIntoArrayLines(Lines, false);

	FString PreparedText;
	int32 LineNumber = 0;
	for (const FString& Line : Lines)
	{
		++LineNumber;
		FString Trimmed = Line;
		Trimmed.TrimStartAndEndInline();
		if (Trimmed.IsEmpty())
		{
			continue;
		}

		FString PreparedLine;
		if (!TranslateScdLineToPreparedLine(Trimmed, LineNumber, PreparedLine, OutError))
		{
			return false;
		}

		if (!PreparedLine.IsEmpty())
		{
			PreparedText += PreparedLine;
			PreparedText += TEXT("\n");
		}
	}

	if (PreparedText.IsEmpty())
	{
		OutError = TEXT("SCD JSONL contained no translatable chunks.");
		return false;
	}

	const FString OutDir = FPaths::GetPath(OutPreparedJsonlPath);
	IFileManager::Get().MakeDirectory(*OutDir, true);
	if (!FFileHelper::SaveStringToFile(PreparedText, *OutPreparedJsonlPath))
	{
		OutError = FString::Printf(TEXT("Failed to write prepared JSONL: %s"), *OutPreparedJsonlPath);
		return false;
	}

	return true;
}
