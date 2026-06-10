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

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "SCDHealthEvidenceJsonlTranslator.h"
#include "SCDHealthEvidenceToIISImporter.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
static bool WriteFixtureFile(const FString& Dir, const FString& FileName, const FString& Content)
{
	IFileManager::Get().MakeDirectory(*Dir, true);
	return FFileHelper::SaveStringToFile(Content, *(Dir / FileName));
}

static bool ReadFirstPreparedLine(const FString& PreparedPath, TSharedPtr<FJsonObject>& OutObject)
{
	FString Text;
	if (!FFileHelper::LoadFileToString(Text, *PreparedPath))
	{
		return false;
	}
	TArray<FString> Lines;
	Text.ParseIntoArrayLines(Lines, false);
	for (const FString& Line : Lines)
	{
		FString Trimmed = Line;
		Trimmed.TrimStartAndEndInline();
		if (Trimmed.IsEmpty())
		{
			continue;
		}
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Trimmed);
		return FJsonSerializer::Deserialize(Reader, OutObject) && OutObject.IsValid();
	}
	return false;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSCDHealthEvidenceTranslateJsonlTest,
	"SmartContentDietBridge.Health.TranslateJsonl",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSCDHealthEvidenceTranslateJsonlTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	const FString Dir = FPaths::Combine(
		FPaths::ProjectSavedDir(), TEXT("SmartContentDiet"), TEXT("HealthImportTest"), TEXT("Translate"));

	{
		const FString ScdPath = Dir / TEXT("happy.jsonl");
		const FString PreparedPath = Dir / TEXT("happy_prepared.jsonl");
		const FString Line =
			TEXT("{\"chunk_id\":\"/Game/A\",\"title\":\"Finding\",\"chunk_kind\":\"scd.finding\",")
			TEXT("\"sensitivity\":\"internal\",\"content\":\"dimension=Performance\"}\n");
		TestTrue(TEXT("Happy fixture writes."), WriteFixtureFile(Dir, TEXT("happy.jsonl"), Line));

		FString Error;
		TestTrue(
			TEXT("Happy path translates."),
			SCDHealthEvidenceJsonlTranslator::TranslateScdHealthJsonlToPreparedJsonl(ScdPath, PreparedPath, Error));

		TSharedPtr<FJsonObject> Prepared;
		TestTrue(TEXT("Prepared JSONL parses."), ReadFirstPreparedLine(PreparedPath, Prepared));
		FString PreparedId;
		FString PreparedText;
		TestTrue(TEXT("prepared_chunk_id present."), Prepared->TryGetStringField(TEXT("prepared_chunk_id"), PreparedId));
		TestTrue(TEXT("prepared_text present."), Prepared->TryGetStringField(TEXT("prepared_text"), PreparedText));
		TestEqual(TEXT("chunk_id maps to prepared_chunk_id."), PreparedId, FString(TEXT("/Game/A")));
		TestEqual(TEXT("content maps to prepared_text."), PreparedText, FString(TEXT("dimension=Performance")));
	}

	{
		const FString ScdPath = Dir / TEXT("missing_id.jsonl");
		const FString PreparedPath = Dir / TEXT("missing_id_prepared.jsonl");
		TestTrue(
			TEXT("Missing chunk_id fixture writes."),
			WriteFixtureFile(Dir, TEXT("missing_id.jsonl"), TEXT("{\"content\":\"x\"}\n")));

		FString Error;
		TestFalse(
			TEXT("Missing chunk_id fails."),
			SCDHealthEvidenceJsonlTranslator::TranslateScdHealthJsonlToPreparedJsonl(ScdPath, PreparedPath, Error));
		TestTrue(TEXT("Error mentions chunk_id."), Error.Contains(TEXT("chunk_id")));
	}

	{
		const FString ScdPath = Dir / TEXT("missing_content.jsonl");
		const FString PreparedPath = Dir / TEXT("missing_content_prepared.jsonl");
		TestTrue(
			TEXT("Missing content fixture writes."),
			WriteFixtureFile(Dir, TEXT("missing_content.jsonl"), TEXT("{\"chunk_id\":\"/Game/A\"}\n")));

		FString Error;
		TestFalse(
			TEXT("Missing content fails."),
			SCDHealthEvidenceJsonlTranslator::TranslateScdHealthJsonlToPreparedJsonl(ScdPath, PreparedPath, Error));
		TestTrue(TEXT("Error mentions content."), Error.Contains(TEXT("content")));
	}

	{
		const FString ScdPath = Dir / TEXT("malformed.jsonl");
		const FString PreparedPath = Dir / TEXT("malformed_prepared.jsonl");
		TestTrue(TEXT("Malformed fixture writes."), WriteFixtureFile(Dir, TEXT("malformed.jsonl"), TEXT("{broken\n")));

		FString Error;
		TestFalse(
			TEXT("Malformed JSON fails."),
			SCDHealthEvidenceJsonlTranslator::TranslateScdHealthJsonlToPreparedJsonl(ScdPath, PreparedPath, Error));
		TestTrue(TEXT("Error mentions parse."), Error.Contains(TEXT("parse")));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSCDHealthEvidenceKindSensitivityMapTest,
	"SmartContentDietBridge.Health.KindSensitivityMap",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSCDHealthEvidenceKindSensitivityMapTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	const FString Dir = FPaths::Combine(
		FPaths::ProjectSavedDir(), TEXT("SmartContentDiet"), TEXT("HealthImportTest"), TEXT("KindMap"));

	auto TranslateFirst =
		[&](const FString& FileStem, const FString& Line, TSharedPtr<FJsonObject>& OutObject) -> bool
	{
		const FString ScdPath = Dir / (FileStem + TEXT(".jsonl"));
		const FString PreparedPath = Dir / (FileStem + TEXT("_prepared.jsonl"));
		if (!WriteFixtureFile(Dir, FileStem + TEXT(".jsonl"), Line))
		{
			return false;
		}
		FString Error;
		if (!SCDHealthEvidenceJsonlTranslator::TranslateScdHealthJsonlToPreparedJsonl(ScdPath, PreparedPath, Error))
		{
			return false;
		}
		return ReadFirstPreparedLine(PreparedPath, OutObject);
	};

	{
		TSharedPtr<FJsonObject> Prepared;
		const FString Line =
			TEXT("{\"chunk_id\":\"/Game/A\",\"chunk_kind\":\"scd.finding\",")
			TEXT("\"sensitivity\":\"internal\",\"content\":\"x\"}\n");
		TestTrue(TEXT("Finding translates."), TranslateFirst(TEXT("finding"), Line, Prepared));
		FString Kind;
		FString Sensitivity;
		Prepared->TryGetStringField(TEXT("chunk_kind"), Kind);
		Prepared->TryGetStringField(TEXT("sensitivity"), Sensitivity);
		TestEqual(TEXT("scd.finding maps to IIS ArchitectureFinding."), Kind, FString(TEXT("ArchitectureFinding")));
		TestEqual(TEXT("internal maps to IIS ProjectLocal."), Sensitivity, FString(TEXT("ProjectLocal")));
	}

	{
		TSharedPtr<FJsonObject> Prepared;
		const FString Line =
			TEXT("{\"chunk_id\":\"scd.summary\",\"chunk_kind\":\"scd.summary\",")
			TEXT("\"sensitivity\":\"internal\",\"content\":\"y\"}\n");
		TestTrue(TEXT("Summary translates."), TranslateFirst(TEXT("summary"), Line, Prepared));
		FString Kind;
		Prepared->TryGetStringField(TEXT("chunk_kind"), Kind);
		TestEqual(TEXT("scd.summary maps to IIS Documentation."), Kind, FString(TEXT("Documentation")));
	}

	{
		TSharedPtr<FJsonObject> Prepared;
		const FString Line =
			TEXT("{\"chunk_id\":\"/Game/B\",\"chunk_kind\":\"scd.finding\",")
			TEXT("\"sensitivity\":\"ProjectLocal\",\"content\":\"z\"}\n");
		TestTrue(TEXT("Pass-through translates."), TranslateFirst(TEXT("passthrough"), Line, Prepared));
		FString Sensitivity;
		Prepared->TryGetStringField(TEXT("sensitivity"), Sensitivity);
		TestEqual(TEXT("Already-valid sensitivity is preserved."), Sensitivity, FString(TEXT("ProjectLocal")));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSCDHealthEvidenceImportJsonlTest,
	"SmartContentDietBridge.Health.ImportJsonl",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSCDHealthEvidenceImportJsonlTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	const FString Dir = FPaths::Combine(
		FPaths::ProjectSavedDir(), TEXT("SmartContentDiet"), TEXT("HealthImportTest"), TEXT("Import"));
	const FString FixturePath = Dir / TEXT("scd_health_chunks.jsonl");
	const FString Line =
		TEXT("{\"chunk_id\":\"/Game/SM_Tree.SM_Tree\",\"title\":\"Performance finding\",")
		TEXT("\"chunk_kind\":\"scd.finding\",\"sensitivity\":\"internal\",")
		TEXT("\"content\":\"dimension=Performance impact=high\"}\n");
	TestTrue(TEXT("Import fixture writes."), WriteFixtureFile(Dir, TEXT("scd_health_chunks.jsonl"), Line));

	FString Error;
	const bool bImported = FSCDHealthEvidenceToIISImporter::ImportHealthEvidenceJsonl(FixturePath, Error);
	if (!bImported && Error.Contains(TEXT("not available")))
	{
		AddWarning(TEXT("IIS not present — graceful no-op verified."));
		return true;
	}

	TestTrue(FString::Printf(TEXT("Import succeeded: %s"), *Error), bImported);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
