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

#include "IISPythonBridge.h"

#include "IISAgentAccessService.h"
#include "IISAgentAccessTypes.h"
#include "InternalIndexServiceModule.h"
#include "IISChunkCatalog.h"
#include "IISContextPackTypes.h"
#include "IISEmbeddingJobQueue.h"
#include "IISSearchTypes.h"
#include "IISServiceInterface.h"
#include "IISStoragePaths.h"
#include "Misc/FileHelper.h"
#include "Modules/ModuleManager.h"

namespace
{
	IInternalIndexService* GetIISService()
	{
		FInternalIndexServiceModule* Module =
			FModuleManager::Get().LoadModulePtr<FInternalIndexServiceModule>(TEXT("InternalIndexService"));
		if (!Module)
		{
			return nullptr;
		}

		return &Module->GetService();
	}

	FString SearchStatusToString(EIISSearchStatus Status)
	{
		switch (Status)
		{
		case EIISSearchStatus::Ready:
			return TEXT("Ready");
		case EIISSearchStatus::Empty:
			return TEXT("Empty");
		case EIISSearchStatus::Warning:
			return TEXT("Warning");
		case EIISSearchStatus::Error:
			return TEXT("Error");
		case EIISSearchStatus::Unknown:
		default:
			return TEXT("Unknown");
		}
	}

	FString ContextPackStatusToString(EIISContextPackStatus Status)
	{
		switch (Status)
		{
		case EIISContextPackStatus::Ready:
			return TEXT("Ready");
		case EIISContextPackStatus::Empty:
			return TEXT("Empty");
		case EIISContextPackStatus::Warning:
			return TEXT("Warning");
		case EIISContextPackStatus::Error:
			return TEXT("Error");
		case EIISContextPackStatus::Unknown:
		default:
			return TEXT("Unknown");
		}
	}

	EIISSearchMode ParseSearchMode(const FString& SearchMode)
	{
		const FString Normalized = SearchMode.ToLower();
		if (Normalized == TEXT("lexical"))
		{
			return EIISSearchMode::Lexical;
		}
		if (Normalized == TEXT("vector"))
		{
			return EIISSearchMode::Vector;
		}
		if (Normalized == TEXT("hybrid"))
		{
			return EIISSearchMode::Hybrid;
		}
		return EIISSearchMode::Unknown;
	}

	bool RunSearchReport(
		const FString& QueryText,
		const FString& SearchMode,
		const int32 MaxResults,
		const TArray<FString>& RequiredLabels,
		const TArray<FString>& PreferredGroups,
		const TArray<FString>& ExcludedSensitivities,
		FString& OutSearchReportPath)
	{
		OutSearchReportPath.Reset();

		IInternalIndexService* Service = GetIISService();
		if (!Service)
		{
			return false;
		}

		FIISSearchQuery Query;
		Query.QueryText = QueryText;
		Query.SearchMode = ParseSearchMode(SearchMode);
		Query.MaxResults = MaxResults <= 0 ? 10 : MaxResults;
		Query.RequiredLabels = RequiredLabels;
		Query.PreferredGroups = PreferredGroups;
		Query.ExcludedSensitivities = ExcludedSensitivities;

		FIISSearchResponse SearchResponse;
		const bool bSearchOk = Service->Search(Query, SearchResponse);

		FIISContextPack ContextPack;
		Service->BuildContextPack(Query, ContextPack);

		const bool bReportOk = FIISChunkCatalog::WriteSearchReport(
			Query,
			SearchResponse,
			&ContextPack,
			OutSearchReportPath);

		return bSearchOk && bReportOk && SearchResponse.Status != EIISSearchStatus::Error;
	}
}

FString UIISPythonBridge::GetIISVersion()
{
	IInternalIndexService* Service = GetIISService();
	return Service ? Service->GetServiceVersion() : TEXT("unavailable");
}

FString UIISPythonBridge::GetDefaultIndexRoot()
{
	IInternalIndexService* Service = GetIISService();
	return Service ? Service->GetDefaultIndexRoot() : FIISStoragePaths::GetDefaultIndexRoot();
}

bool UIISPythonBridge::IsIISAvailable()
{
	IInternalIndexService* Service = GetIISService();
	return Service && Service->IsAvailable();
}

bool UIISPythonBridge::SmokeTestService(FString& OutReportPath)
{
	OutReportPath.Reset();

	TArray<FString> CreatedFolders;
	TArray<FString> FolderWarnings;
	const bool bFoldersReady = FIISStoragePaths::EnsureDefaultFolders(&CreatedFolders, &FolderWarnings);

	IInternalIndexService* Service = GetIISService();
	const bool bServiceAvailable = Service && Service->IsAvailable();

	FIISSearchQuery Query;
	Query.QueryText = TEXT("iis smoke test");
	Query.SearchMode = EIISSearchMode::Lexical;
	Query.MaxResults = 3;

	FIISSearchResponse SearchResponse;
	const bool bSearchOk = Service ? Service->Search(Query, SearchResponse) : false;

	FIISContextPack ContextPack;
	const bool bContextPackOk = Service ? Service->BuildContextPack(Query, ContextPack) : false;

	TArray<FString> Lines;
	Lines.Add(TEXT("# IIS Smoke Test Report"));
	Lines.Add(TEXT(""));
	Lines.Add(FString::Printf(TEXT("- Version: %s"), Service ? *Service->GetServiceVersion() : TEXT("unavailable")));
	Lines.Add(FString::Printf(TEXT("- Available: %s"), bServiceAvailable ? TEXT("true") : TEXT("false")));
	Lines.Add(FString::Printf(TEXT("- IndexRoot: %s"), *FIISStoragePaths::GetDefaultIndexRoot()));
	Lines.Add(FString::Printf(TEXT("- FoldersReady: %s"), bFoldersReady ? TEXT("true") : TEXT("false")));
	Lines.Add(FString::Printf(TEXT("- CreatedFolders: %d"), CreatedFolders.Num()));
	Lines.Add(FString::Printf(TEXT("- SearchOk: %s"), bSearchOk ? TEXT("true") : TEXT("false")));
	Lines.Add(FString::Printf(TEXT("- SearchStatus: %s"), *SearchStatusToString(SearchResponse.Status)));
	Lines.Add(FString::Printf(TEXT("- SearchResults: %d"), SearchResponse.Results.Num()));
	Lines.Add(FString::Printf(TEXT("- ContextPackOk: %s"), bContextPackOk ? TEXT("true") : TEXT("false")));
	Lines.Add(FString::Printf(TEXT("- ContextPackStatus: %s"), *ContextPackStatusToString(ContextPack.Status)));
	Lines.Add(FString::Printf(TEXT("- ContextPackItems: %d"), ContextPack.Items.Num()));

	for (const FString& Warning : FolderWarnings)
	{
		Lines.Add(FString::Printf(TEXT("- FolderWarning: %s"), *Warning));
	}
	for (const FString& Warning : SearchResponse.Warnings)
	{
		Lines.Add(FString::Printf(TEXT("- SearchWarning: %s"), *Warning));
	}
	for (const FString& Warning : ContextPack.Warnings)
	{
		Lines.Add(FString::Printf(TEXT("- ContextPackWarning: %s"), *Warning));
	}

	OutReportPath = FIISStoragePaths::GetLogsDir() / TEXT("iis_smoke_test_report.md");
	const bool bSaved = FFileHelper::SaveStringArrayToFile(Lines, *OutReportPath);
	return bSaved && bFoldersReady && bServiceAvailable && bSearchOk && bContextPackOk;
}

bool UIISPythonBridge::ImportPreparedChunksJsonl(
	const FString& PreparedChunksJsonlPath,
	FString& OutImportReportPath)
{
	TArray<FString> Warnings;
	return ImportPreparedChunksJsonlWithWarnings(PreparedChunksJsonlPath, OutImportReportPath, Warnings);
}

bool UIISPythonBridge::ImportPreparedChunksJsonlWithWarnings(
	const FString& PreparedChunksJsonlPath,
	FString& OutImportReportPath,
	TArray<FString>& OutWarnings)
{
	OutImportReportPath.Reset();
	OutWarnings.Reset();

	IInternalIndexService* Service = GetIISService();
	if (!Service)
	{
		OutWarnings.Add(TEXT("Internal Index Service module is unavailable."));
		return false;
	}

	return Service->ImportPreparedChunksJsonl(PreparedChunksJsonlPath, OutImportReportPath, OutWarnings);
}

bool UIISPythonBridge::BuildChunkCatalog(FString& OutReportPath)
{
	TArray<FString> Warnings;
	return BuildChunkCatalogWithWarnings(OutReportPath, Warnings);
}

bool UIISPythonBridge::BuildChunkCatalogWithWarnings(
	FString& OutReportPath,
	TArray<FString>& OutWarnings)
{
	OutReportPath.Reset();
	OutWarnings.Reset();

	return FIISChunkCatalog::BuildOrUpdateCatalogFromChunkStore(OutReportPath, OutWarnings);
}

bool UIISPythonBridge::SearchChunksLexical(
	const FString& QueryText,
	int32 MaxResults,
	FString& OutSearchReportPath)
{
	return SearchChunksWithMode(QueryText, TEXT("Lexical"), MaxResults, OutSearchReportPath);
}

bool UIISPythonBridge::SearchChunksVector(
	const FString& QueryText,
	int32 MaxResults,
	FString& OutSearchReportPath)
{
	return SearchChunksWithMode(QueryText, TEXT("Vector"), MaxResults, OutSearchReportPath);
}

bool UIISPythonBridge::SearchChunksHybrid(
	const FString& QueryText,
	int32 MaxResults,
	FString& OutSearchReportPath)
{
	return SearchChunksWithMode(QueryText, TEXT("Hybrid"), MaxResults, OutSearchReportPath);
}

bool UIISPythonBridge::SearchChunksWithMode(
	const FString& QueryText,
	const FString& SearchMode,
	int32 MaxResults,
	FString& OutSearchReportPath)
{
	const TArray<FString> Empty;
	return RunSearchReport(QueryText, SearchMode, MaxResults, Empty, Empty, Empty, OutSearchReportPath);
}

bool UIISPythonBridge::SearchChunksWithFilters(
	const FString& QueryText,
	const FString& SearchMode,
	int32 MaxResults,
	const TArray<FString>& RequiredLabels,
	const TArray<FString>& PreferredGroups,
	const TArray<FString>& ExcludedSensitivities,
	FString& OutSearchReportPath)
{
	return RunSearchReport(
		QueryText,
		SearchMode,
		MaxResults,
		RequiredLabels,
		PreferredGroups,
		ExcludedSensitivities,
		OutSearchReportPath);
}

bool UIISPythonBridge::BuildContextPackForQuery(
	const FString& QueryText,
	const FString& SearchMode,
	int32 MaxResults,
	FString& OutContextPackReportPath)
{
	OutContextPackReportPath.Reset();

	IInternalIndexService* Service = GetIISService();
	if (!Service)
	{
		return false;
	}

	FIISSearchQuery Query;
	Query.QueryText = QueryText;
	Query.SearchMode = ParseSearchMode(SearchMode);
	Query.MaxResults = MaxResults <= 0 ? 10 : MaxResults;

	FIISContextPack ContextPack;
	const bool bContextPackOk = Service->BuildContextPack(Query, ContextPack);
	const bool bReportOk = FIISChunkCatalog::WriteContextPackReport(ContextPack, OutContextPackReportPath);
	return bContextPackOk && bReportOk && ContextPack.Status != EIISContextPackStatus::Error;
}

bool UIISPythonBridge::BuildEmbeddingJobs(FString& OutReportPath)
{
	OutReportPath.Reset();
	TArray<FString> Warnings;
	return FIISEmbeddingJobQueue::BuildEmbeddingJobsFromCatalog(OutReportPath, Warnings);
}

bool UIISPythonBridge::ExecuteEmbeddingJobs(int32 MaxJobs, FString& OutReportPath)
{
	OutReportPath.Reset();
	TArray<FString> Warnings;
	return FIISEmbeddingJobQueue::ExecutePendingEmbeddingJobs(MaxJobs, OutReportPath, Warnings);
}

bool UIISPythonBridge::BuildAndExecuteEmbeddingJobs(int32 MaxJobs, FString& OutReportPath)
{
	OutReportPath.Reset();
	TArray<FString> Warnings;
	return FIISEmbeddingJobQueue::BuildAndExecuteEmbeddingJobs(MaxJobs, OutReportPath, Warnings);
}

bool UIISPythonBridge::ExecuteEmbeddingJobsWithWarnings(
	int32 MaxJobs,
	FString& OutReportPath,
	TArray<FString>& OutWarnings)
{
	OutReportPath.Reset();
	OutWarnings.Reset();
	return FIISEmbeddingJobQueue::ExecutePendingEmbeddingJobs(MaxJobs, OutReportPath, OutWarnings);
}

bool UIISPythonBridge::AgentSearch(
	const FString& QueryText,
	const FString& SearchMode,
	int32 MaxResults,
	FString& OutResponsePath)
{
	OutResponsePath.Reset();

	FIISAgentToolRequest Request;
	Request.ToolKind = EIISAgentToolKind::Search;
	Request.QueryText = QueryText;
	Request.SearchMode = ParseSearchMode(SearchMode);
	Request.MaxResults = MaxResults <= 0 ? 10 : MaxResults;

	FIISAgentToolResponse Response;
	const bool bSuccess = FIISAgentAccessService::Search(Request, Response);
	OutResponsePath = FIISAgentAccessService::GetLatestResponsePath(EIISAgentToolKind::Search);
	return bSuccess;
}

bool UIISPythonBridge::AgentGetContextPack(
	const FString& QueryText,
	const FString& SearchMode,
	int32 MaxResults,
	FString& OutResponsePath)
{
	OutResponsePath.Reset();

	FIISAgentToolRequest Request;
	Request.ToolKind = EIISAgentToolKind::GetContextPack;
	Request.QueryText = QueryText;
	Request.SearchMode = ParseSearchMode(SearchMode);
	Request.MaxResults = MaxResults <= 0 ? 10 : MaxResults;

	FIISAgentToolResponse Response;
	const bool bSuccess = FIISAgentAccessService::GetContextPack(Request, Response);
	OutResponsePath = FIISAgentAccessService::GetLatestResponsePath(EIISAgentToolKind::GetContextPack);
	return bSuccess;
}

bool UIISPythonBridge::AgentGetChunk(
	const FString& ChunkId,
	FString& OutResponsePath)
{
	OutResponsePath.Reset();

	FIISAgentToolResponse Response;
	const bool bSuccess = FIISAgentAccessService::GetChunk(ChunkId, Response);
	OutResponsePath = FIISAgentAccessService::GetLatestResponsePath(EIISAgentToolKind::GetChunk);
	return bSuccess;
}

bool UIISPythonBridge::AgentGetSourceReferences(
	const FString& ChunkId,
	FString& OutResponsePath)
{
	OutResponsePath.Reset();

	FIISAgentToolResponse Response;
	const bool bSuccess = FIISAgentAccessService::GetSourceReferences(ChunkId, Response);
	OutResponsePath = FIISAgentAccessService::GetLatestResponsePath(EIISAgentToolKind::GetSourceReferences);
	return bSuccess;
}

bool UIISPythonBridge::AgentFindUsages(
	const FString& SymbolName,
	FString& OutResponsePath)
{
	OutResponsePath.Reset();

	FIISAgentToolRequest Request;
	Request.ToolKind = EIISAgentToolKind::FindUsages;
	Request.SymbolName = SymbolName;
	Request.MaxResults = 10;

	FIISAgentToolResponse Response;
	const bool bSuccess = FIISAgentAccessService::FindUsages(Request, Response);
	OutResponsePath = FIISAgentAccessService::GetLatestResponsePath(EIISAgentToolKind::FindUsages);
	return bSuccess;
}

bool UIISPythonBridge::AgentExplainBlueprint(
	const FString& AssetPathOrQuery,
	FString& OutResponsePath)
{
	OutResponsePath.Reset();

	FIISAgentToolRequest Request;
	Request.ToolKind = EIISAgentToolKind::ExplainBlueprint;
	Request.AssetPath = AssetPathOrQuery;
	Request.QueryText = AssetPathOrQuery;
	Request.SearchMode = EIISSearchMode::Hybrid;
	Request.MaxResults = 10;

	FIISAgentToolResponse Response;
	const bool bSuccess = FIISAgentAccessService::ExplainBlueprint(Request, Response);
	OutResponsePath = FIISAgentAccessService::GetLatestResponsePath(EIISAgentToolKind::ExplainBlueprint);
	return bSuccess;
}

bool UIISPythonBridge::WriteAgentToolContracts(FString& OutContractsPath)
{
	OutContractsPath.Reset();
	return FIISAgentAccessService::WriteAgentToolContracts(OutContractsPath);
}

bool UIISPythonBridge::ExecuteAgentToolFromJson(
	const FString& InputJsonPath,
	FString& OutResponseJsonPath)
{
	OutResponseJsonPath.Reset();
	return FIISAgentAccessService::ExecuteAgentToolFromJson(InputJsonPath, OutResponseJsonPath);
}
