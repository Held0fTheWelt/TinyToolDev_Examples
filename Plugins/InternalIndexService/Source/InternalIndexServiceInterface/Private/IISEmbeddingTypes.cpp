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

#include "IISEmbeddingTypes.h"

#include "HAL/CriticalSection.h"
#include "Misc/ScopeLock.h"

namespace
{
	FCriticalSection GIISEmbeddingExecutorRegistryMutex;
	TMap<FString, TSharedPtr<IIISEmbeddingRouteExecutor>> GIISEmbeddingExecutors;

	TSharedPtr<IIISEmbeddingRouteExecutor> GetFirstRegisteredExecutor()
	{
		TArray<FString> ExecutorIds;
		GIISEmbeddingExecutors.GetKeys(ExecutorIds);
		ExecutorIds.Sort();

		for (const FString& ExecutorId : ExecutorIds)
		{
			if (const TSharedPtr<IIISEmbeddingRouteExecutor>* Executor = GIISEmbeddingExecutors.Find(ExecutorId))
			{
				return *Executor;
			}
		}

		return nullptr;
	}
}

bool FIISEmbeddingRouteExecutorRegistry::RegisterExecutor(TSharedRef<IIISEmbeddingRouteExecutor> Executor)
{
	const FString ExecutorId = Executor->GetExecutorId().TrimStartAndEnd();
	if (ExecutorId.IsEmpty())
	{
		return false;
	}

	FScopeLock Lock(&GIISEmbeddingExecutorRegistryMutex);
	GIISEmbeddingExecutors.Add(ExecutorId, Executor);
	return true;
}

bool FIISEmbeddingRouteExecutorRegistry::UnregisterExecutor(const FString& ExecutorId)
{
	const FString NormalizedExecutorId = ExecutorId.TrimStartAndEnd();
	if (NormalizedExecutorId.IsEmpty())
	{
		return false;
	}

	FScopeLock Lock(&GIISEmbeddingExecutorRegistryMutex);
	return GIISEmbeddingExecutors.Remove(NormalizedExecutorId) > 0;
}

TSharedPtr<IIISEmbeddingRouteExecutor> FIISEmbeddingRouteExecutorRegistry::GetExecutor(
	const FString& PreferredExecutorId)
{
	FScopeLock Lock(&GIISEmbeddingExecutorRegistryMutex);

	const FString NormalizedExecutorId = PreferredExecutorId.TrimStartAndEnd();
	if (!NormalizedExecutorId.IsEmpty())
	{
		if (const TSharedPtr<IIISEmbeddingRouteExecutor>* Executor = GIISEmbeddingExecutors.Find(NormalizedExecutorId))
		{
			return *Executor;
		}
		return nullptr;
	}

	return GetFirstRegisteredExecutor();
}

TArray<FString> FIISEmbeddingRouteExecutorRegistry::GetExecutorIds()
{
	FScopeLock Lock(&GIISEmbeddingExecutorRegistryMutex);

	TArray<FString> ExecutorIds;
	GIISEmbeddingExecutors.GetKeys(ExecutorIds);
	ExecutorIds.Sort();
	return ExecutorIds;
}

bool FIISEmbeddingRouteExecutorRegistry::ResolveEmbeddingRoute(
	const FString& TaskKind,
	FIISEmbeddingRoute& OutRoute,
	TArray<FString>& OutWarnings,
	const FString& PreferredExecutorId)
{
	const TSharedPtr<IIISEmbeddingRouteExecutor> Executor = GetExecutor(PreferredExecutorId);
	if (!Executor)
	{
		OutWarnings.Add(TEXT("No IIS embedding route executor is registered; embedding route metadata is unavailable."));
		return false;
	}

	return Executor->ResolveEmbeddingRoute(TaskKind, OutRoute, OutWarnings);
}

bool FIISEmbeddingRouteExecutorRegistry::ExecuteEmbeddingRoute(
	const FIISEmbeddingRequest& Request,
	FIISEmbeddingResponse& OutResponse,
	const FString& PreferredExecutorId)
{
	const TSharedPtr<IIISEmbeddingRouteExecutor> Executor = GetExecutor(PreferredExecutorId);
	if (!Executor)
	{
		OutResponse.bSuccess = false;
		OutResponse.RouteId = Request.RouteId;
		OutResponse.ErrorCode = TEXT("embedding_executor_not_registered");
		OutResponse.ErrorMessage = TEXT("No IIS embedding route executor is registered.");
		return false;
	}

	return Executor->ExecuteEmbeddingRoute(Request, OutResponse);
}
