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

#include "IISSubsystem.h"
#include "InternalIndexServiceModule.h"

IInternalIndexService* UIISSubsystem::ResolveService()
{
	if (!FInternalIndexServiceModule::IsAvailable())
	{
		return nullptr;
	}
	return &FInternalIndexServiceModule::Get().GetService();
}

bool UIISSubsystem::IsAvailable() const
{
	IInternalIndexService* Service = ResolveService();
	return Service ? Service->IsAvailable() : false;
}

FString UIISSubsystem::GetServiceVersion() const
{
	IInternalIndexService* Service = ResolveService();
	return Service ? Service->GetServiceVersion() : FString();
}

FString UIISSubsystem::GetDefaultIndexRoot() const
{
	IInternalIndexService* Service = ResolveService();
	return Service ? Service->GetDefaultIndexRoot() : FString();
}

bool UIISSubsystem::ImportPreparedChunksJsonl(
	const FString& PreparedChunksJsonlPath,
	FString& OutImportReportPath,
	TArray<FString>& OutWarnings)
{
	IInternalIndexService* Service = ResolveService();
	if (!Service)
	{
		OutWarnings.Add(TEXT("IIS service unavailable."));
		return false;
	}
	return Service->ImportPreparedChunksJsonl(PreparedChunksJsonlPath, OutImportReportPath, OutWarnings);
}

bool UIISSubsystem::Search(const FIISSearchQuery& Query, FIISSearchResponse& OutResponse)
{
	IInternalIndexService* Service = ResolveService();
	if (!Service)
	{
		OutResponse.Status = EIISSearchStatus::Error;
		OutResponse.Errors.Add(TEXT("IIS service unavailable."));
		return false;
	}
	return Service->Search(Query, OutResponse);
}

bool UIISSubsystem::BuildContextPack(const FIISSearchQuery& Query, FIISContextPack& OutContextPack)
{
	IInternalIndexService* Service = ResolveService();
	if (!Service)
	{
		OutContextPack.Status = EIISContextPackStatus::Error;
		return false;
	}
	return Service->BuildContextPack(Query, OutContextPack);
}

bool UIISSubsystem::BuildEmbeddingJobs(FString& OutReportPath, TArray<FString>& OutWarnings)
{
	IInternalIndexService* Service = ResolveService();
	if (!Service)
	{
		OutWarnings.Add(TEXT("IIS service unavailable."));
		return false;
	}
	return Service->BuildEmbeddingJobs(OutReportPath, OutWarnings);
}

bool UIISSubsystem::ExecuteEmbeddingJobs(int32 MaxJobs, FString& OutReportPath, TArray<FString>& OutWarnings)
{
	IInternalIndexService* Service = ResolveService();
	if (!Service)
	{
		OutWarnings.Add(TEXT("IIS service unavailable."));
		return false;
	}
	return Service->ExecuteEmbeddingJobs(MaxJobs, OutReportPath, OutWarnings);
}

bool UIISSubsystem::ExecuteAgentTool(const FIISAgentToolRequest& Request, FIISAgentToolResponse& OutResponse)
{
	IInternalIndexService* Service = ResolveService();
	if (!Service)
	{
		OutResponse.Status = EIISAgentToolStatus::Error;
		OutResponse.Errors.Add(TEXT("IIS service unavailable."));
		return false;
	}
	return Service->ExecuteAgentTool(Request, OutResponse);
}

bool UIISSubsystem::GetChunk(const FString& ChunkId, FIISAgentToolResponse& OutResponse)
{
	IInternalIndexService* Service = ResolveService();
	if (!Service)
	{
		OutResponse.Status = EIISAgentToolStatus::Error;
		OutResponse.Errors.Add(TEXT("IIS service unavailable."));
		return false;
	}
	return Service->GetChunk(ChunkId, OutResponse);
}

bool UIISSubsystem::GetSourceReferences(const FString& ChunkId, FIISAgentToolResponse& OutResponse)
{
	IInternalIndexService* Service = ResolveService();
	if (!Service)
	{
		OutResponse.Status = EIISAgentToolStatus::Error;
		OutResponse.Errors.Add(TEXT("IIS service unavailable."));
		return false;
	}
	return Service->GetSourceReferences(ChunkId, OutResponse);
}

bool UIISSubsystem::FindUsages(const FIISAgentToolRequest& Request, FIISAgentToolResponse& OutResponse)
{
	IInternalIndexService* Service = ResolveService();
	if (!Service)
	{
		OutResponse.Status = EIISAgentToolStatus::Error;
		OutResponse.Errors.Add(TEXT("IIS service unavailable."));
		return false;
	}
	return Service->FindUsages(Request, OutResponse);
}

bool UIISSubsystem::ExplainBlueprint(const FIISAgentToolRequest& Request, FIISAgentToolResponse& OutResponse)
{
	IInternalIndexService* Service = ResolveService();
	if (!Service)
	{
		OutResponse.Status = EIISAgentToolStatus::Error;
		OutResponse.Errors.Add(TEXT("IIS service unavailable."));
		return false;
	}
	return Service->ExplainBlueprint(Request, OutResponse);
}
