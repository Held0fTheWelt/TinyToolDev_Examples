/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#include "IISPanelStatus.h"

#include "Engine/Engine.h"
#include "IISChunkCatalog.h"
#include "IISEmbeddingTypes.h"
#include "IISStoragePaths.h"
#include "IISSubsystem.h"
#include "InternalIndexServiceEditorModule.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
	FString GetCatalogBuildReportPath()
	{
		const FString IndexReportPath = FIISStoragePaths::GetIndexesDir() / TEXT("catalog_build_report.json");
		if (FPaths::FileExists(IndexReportPath))
		{
			return IndexReportPath;
		}

		return FIISStoragePaths::GetReportsDir() / TEXT("catalog_build_report.json");
	}

	bool TryReadNumberField(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName, int32& OutValue)
	{
		if (!Object.IsValid())
		{
			return false;
		}

		double Number = 0.0;
		if (!Object->TryGetNumberField(FieldName, Number))
		{
			return false;
		}

		OutValue = static_cast<int32>(Number);
		return true;
	}

	int32 ReadChunkCountFromCatalogReport()
	{
		const FString ReportPath = GetCatalogBuildReportPath();
		FString JsonText;
		if (!FPaths::FileExists(ReportPath) || !FFileHelper::LoadFileToString(JsonText, *ReportPath))
		{
			return -1;
		}

		TSharedPtr<FJsonObject> Root;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
		if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
		{
			return -1;
		}

		const TSharedPtr<FJsonObject>* Summary = nullptr;
		if (Root->TryGetObjectField(TEXT("summary"), Summary) && Summary)
		{
			int32 Count = -1;
			if (TryReadNumberField(*Summary, TEXT("catalog_chunk_count"), Count)
				|| TryReadNumberField(*Summary, TEXT("chunk_count"), Count)
				|| TryReadNumberField(*Summary, TEXT("source_chunk_count"), Count))
			{
				return Count;
			}
		}

		int32 TopCount = -1;
		if (TryReadNumberField(Root, TEXT("catalog_chunk_count"), TopCount)
			|| TryReadNumberField(Root, TEXT("chunk_count"), TopCount)
			|| TryReadNumberField(Root, TEXT("source_chunk_count"), TopCount))
		{
			return TopCount;
		}
		return -1;
	}
}

FIISPanelStatusSnapshot IISCapturePanelStatus()
{
	FIISStoragePaths::EnsureDefaultFolders();

	FIISPanelStatusSnapshot Snapshot;

	UIISSubsystem* Subsystem = GEngine ? GEngine->GetEngineSubsystem<UIISSubsystem>() : nullptr;
	Snapshot.bServiceAvailable = Subsystem && Subsystem->IsAvailable();
	Snapshot.ServiceVersion = Snapshot.bServiceAvailable ? Subsystem->GetServiceVersion() : TEXT("n/a");
	Snapshot.IndexRoot = Snapshot.bServiceAvailable ? Subsystem->GetDefaultIndexRoot() : FIISStoragePaths::GetDefaultIndexRoot();

	Snapshot.bCatalogPresent = FPaths::FileExists(FIISChunkCatalog::GetCatalogPath());
	Snapshot.bVectorsPresent = FPaths::FileExists(FIISStoragePaths::GetVectorsDir() / TEXT("chunk_vectors.jsonl"));
	Snapshot.ChunkCount = ReadChunkCountFromCatalogReport();

	Snapshot.Mcp = FModuleManager::LoadModuleChecked<FInternalIndexServiceEditorModule>(
		TEXT("InternalIndexServiceEditor")).GetMcpStatus();

	for (const FString& ExecutorId : FIISEmbeddingRouteExecutorRegistry::GetExecutorIds())
	{
		FIISIntegrationStatus Integration;
		Integration.ExecutorId = ExecutorId;
		Integration.bRegistered = FIISEmbeddingRouteExecutorRegistry::GetExecutor(ExecutorId).IsValid();
		Snapshot.Integrations.Add(Integration);
	}

	return Snapshot;
}
