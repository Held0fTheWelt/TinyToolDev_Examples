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

#include "IISLocalIndexService.h"

#include "IISAgentAccessService.h"
#include "IISChunkCatalog.h"
#include "IISEmbeddingJobQueue.h"
#include "IISPreparedChunkImporter.h"
#include "IISStoragePaths.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"

namespace
{
	FString IISSearchModeToString(const EIISSearchMode SearchMode)
	{
		switch (SearchMode)
		{
		case EIISSearchMode::Lexical:
			return TEXT("Lexical");
		case EIISSearchMode::Vector:
			return TEXT("Vector");
		case EIISSearchMode::Hybrid:
			return TEXT("Hybrid");
		case EIISSearchMode::Unknown:
		default:
			return TEXT("Unknown");
		}
	}
}

bool FIISLocalIndexService::IsAvailable() const
{
	return true;
}

FString FIISLocalIndexService::GetServiceVersion() const
{
	return TEXT("0.1.0");
}

FString FIISLocalIndexService::GetDefaultIndexRoot() const
{
	return FIISStoragePaths::GetDefaultIndexRoot();
}

bool FIISLocalIndexService::ImportPreparedChunksJsonl(
	const FString& PreparedChunksJsonlPath,
	FString& OutImportReportPath,
	TArray<FString>& OutWarnings)
{
	const FString InputPath = PreparedChunksJsonlPath.IsEmpty()
		? FString()
		: FPaths::ConvertRelativePathToFull(PreparedChunksJsonlPath);
	const FString InputDirectory = InputPath.IsEmpty()
		? FString()
		: FPaths::GetPath(InputPath);

	FIISImportInputFiles InputFiles;
	InputFiles.PreparedChunksJsonlPath = InputPath;
	if (!InputDirectory.IsEmpty())
	{
		InputFiles.PreparedChunksManifestPath = InputDirectory / TEXT("prepared_rag_chunks_manifest.json");
		InputFiles.RetrievalLabelsPath = InputDirectory / TEXT("retrieval_labels.json");
		InputFiles.RetrievalGroupsPath = InputDirectory / TEXT("retrieval_groups.json");
	}

	return FIISPreparedChunkImporter::ImportPreparedChunks(InputFiles, OutImportReportPath, OutWarnings);
}

bool FIISLocalIndexService::Search(const FIISSearchQuery& Query, FIISSearchResponse& OutResponse)
{
	return FIISChunkCatalog::SearchCatalog(Query, OutResponse);
}

bool FIISLocalIndexService::BuildContextPack(const FIISSearchQuery& Query, FIISContextPack& OutContextPack)
{
	OutContextPack = FIISContextPack();
	OutContextPack.ContextPackId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
	OutContextPack.CreatedAtUtc = FDateTime::UtcNow().ToIso8601();
	OutContextPack.QueryText = Query.QueryText;
	OutContextPack.SearchMode = IISSearchModeToString(Query.SearchMode);
	OutContextPack.bAllowsMigrationDecision = false;
	OutContextPack.bAllowsPatchGeneration = false;
	OutContextPack.Guardrails.Add(TEXT("This context pack is retrieval evidence only."));
	OutContextPack.Guardrails.Add(TEXT("It does not authorize migration, placement, patching, copying, or project mutation."));
	OutContextPack.Guardrails.Add(TEXT("It may contain source evidence and private review notes depending on sensitivity filters."));
	OutContextPack.Guardrails.Add(TEXT("No migration decision is authorized by this context pack."));
	OutContextPack.Guardrails.Add(TEXT("No patch generation is authorized by this context pack."));
	OutContextPack.Guardrails.Add(TEXT("No AI summarization or reranking was executed."));

	FIISSearchQuery SearchQuery = Query;
	SearchQuery.SearchMode = Query.SearchMode == EIISSearchMode::Unknown ? EIISSearchMode::Lexical : Query.SearchMode;

	FIISSearchResponse SearchResponse;
	const bool bSearchOk = Search(SearchQuery, SearchResponse);
	OutContextPack.Warnings.Append(SearchResponse.Warnings);
	OutContextPack.Warnings.Append(SearchResponse.Errors);

	if (!bSearchOk || SearchResponse.Status == EIISSearchStatus::Error)
	{
		OutContextPack.Status = EIISContextPackStatus::Error;
		return false;
	}

	for (const FIISSearchResult& Result : SearchResponse.Results)
	{
		FIISContextPackItem Item;
		Item.ChunkId = Result.ChunkId;
		Item.Title = Result.Title;
		Item.Text = Result.Chunk.Text;
		Item.SourceReferences = Result.Chunk.SourceReferences;
		Item.RetrievalLabels = Result.Chunk.RetrievalLabels;
		OutContextPack.Items.Add(MoveTemp(Item));
	}

	if (OutContextPack.Items.Num() > 0)
	{
		OutContextPack.Status = OutContextPack.Warnings.Num() > 0
			? EIISContextPackStatus::Warning
			: EIISContextPackStatus::Ready;
	}
	else
	{
		OutContextPack.Status = OutContextPack.Warnings.Num() > 0
			? EIISContextPackStatus::Warning
			: EIISContextPackStatus::Empty;
	}

	return true;
}

bool FIISLocalIndexService::BuildEmbeddingJobs(
	FString& OutReportPath,
	TArray<FString>& OutWarnings)
{
	return FIISEmbeddingJobQueue::BuildEmbeddingJobsFromCatalog(OutReportPath, OutWarnings);
}

bool FIISLocalIndexService::ExecuteEmbeddingJobs(
	int32 MaxJobs,
	FString& OutReportPath,
	TArray<FString>& OutWarnings)
{
	return FIISEmbeddingJobQueue::ExecutePendingEmbeddingJobs(MaxJobs, OutReportPath, OutWarnings);
}

bool FIISLocalIndexService::ExecuteAgentTool(
	const FIISAgentToolRequest& Request,
	FIISAgentToolResponse& OutResponse)
{
	return FIISAgentAccessService::ExecuteAgentTool(Request, OutResponse);
}

bool FIISLocalIndexService::GetChunk(
	const FString& ChunkId,
	FIISAgentToolResponse& OutResponse)
{
	return FIISAgentAccessService::GetChunk(ChunkId, OutResponse);
}

bool FIISLocalIndexService::GetSourceReferences(
	const FString& ChunkId,
	FIISAgentToolResponse& OutResponse)
{
	return FIISAgentAccessService::GetSourceReferences(ChunkId, OutResponse);
}

bool FIISLocalIndexService::FindUsages(
	const FIISAgentToolRequest& Request,
	FIISAgentToolResponse& OutResponse)
{
	return FIISAgentAccessService::FindUsages(Request, OutResponse);
}

bool FIISLocalIndexService::ExplainBlueprint(
	const FIISAgentToolRequest& Request,
	FIISAgentToolResponse& OutResponse)
{
	return FIISAgentAccessService::ExplainBlueprint(Request, OutResponse);
}
