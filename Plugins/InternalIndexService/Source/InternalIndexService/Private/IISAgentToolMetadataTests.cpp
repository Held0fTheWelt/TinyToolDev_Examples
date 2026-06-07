/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "IISAgentAccessService.h"
#include "IISStoragePaths.h"
#include "Serialization/JsonSerializer.h"

namespace IISAgentToolMetadataTest
{
	static bool ReadToolNote(const FString& RegistryPath, const FString& ToolName, FString& OutNote)
	{
		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *RegistryPath))
		{
			return false;
		}

		TSharedPtr<FJsonObject> Root;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
		{
			return false;
		}

		const TArray<TSharedPtr<FJsonValue>>* Tools = nullptr;
		if (!Root->TryGetArrayField(TEXT("tools"), Tools) || !Tools)
		{
			return false;
		}

		for (const TSharedPtr<FJsonValue>& Value : *Tools)
		{
			const TSharedPtr<FJsonObject> ToolObj = Value->AsObject();
			if (!ToolObj.IsValid())
			{
				continue;
			}
			FString Name;
			if (!ToolObj->TryGetStringField(TEXT("tool_name"), Name) || Name != ToolName)
			{
				continue;
			}
			return ToolObj->TryGetStringField(TEXT("note"), OutNote);
		}
		return false;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISAgentToolMetadataReflectsGraphAndIR,
	"InternalIndexService.Agent.ToolMetadata",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISAgentToolMetadataReflectsGraphAndIR::RunTest(const FString& Parameters)
{
	FString ContractsPath;
	TArray<FString> Warnings;
	TestTrue(TEXT("write contracts"), FIISAgentAccessService::WriteAgentToolContracts(ContractsPath));

	const FString RegistryPath = FIISStoragePaths::GetAgentContractsDir() / TEXT("iis_agent_tool_registry.json");

	FString UsagesNote;
	TestTrue(TEXT("find usages note"), IISAgentToolMetadataTest::ReadToolNote(RegistryPath, TEXT("iis_find_usages"), UsagesNote));
	TestFalse(TEXT("find usages not foundation-only"), UsagesNote.Contains(TEXT("Foundation only")));
	TestTrue(TEXT("find usages mentions graph"), UsagesNote.Contains(TEXT("graph"), ESearchCase::IgnoreCase));

	FString BlueprintNote;
	TestTrue(TEXT("explain blueprint note"), IISAgentToolMetadataTest::ReadToolNote(RegistryPath, TEXT("iis_explain_blueprint"), BlueprintNote));
	TestFalse(TEXT("blueprint not foundation-only"), BlueprintNote.Contains(TEXT("Foundation only")));
	TestTrue(TEXT("blueprint mentions IR"), BlueprintNote.Contains(TEXT("IR"), ESearchCase::IgnoreCase));
	return true;
}

#endif
