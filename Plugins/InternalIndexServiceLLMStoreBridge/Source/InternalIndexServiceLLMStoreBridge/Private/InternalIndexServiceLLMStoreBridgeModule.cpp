/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA
 *
 * This file is part of the "IIS LLM Store Bridge" Unreal Engine plugin.
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

#include "InternalIndexServiceLLMStoreBridgeModule.h"

#include "Engine/Engine.h"
#include "Interfaces/LLMStoreInterface.h"
#include "Modules/ModuleInterface.h"
#include "Subsystems/EngineSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogIISLLMStoreBridge, Log, All);

namespace
{
	// Stable IIS executor id. IIS embedding jobs select this executor when the
	// optional bridge is present, without IIS taking a compile dependency on
	// LLM Store.
	const FString BridgeExecutorId = TEXT("llmstore");

	// Resolve the LLM Store engine subsystem through its public interface at
	// call time. This keeps startup tolerant of module load order while still
	// failing with explicit warnings when LLM Store is not available.
	ILLMStore* ResolveLLMStore(TArray<FString>& OutWarnings)
	{
		FModuleManager::Get().LoadModulePtr<IModuleInterface>(TEXT("LLMStore"));

		if (!GEngine)
		{
			OutWarnings.Add(TEXT("GEngine is not available; LLM Store cannot be resolved by the IIS bridge."));
			return nullptr;
		}

		UClass* StoreClass = FindObject<UClass>(nullptr, TEXT("/Script/LLMStore.LLMStoreSubsystem"));
		if (!StoreClass)
		{
			StoreClass = LoadObject<UClass>(nullptr, TEXT("/Script/LLMStore.LLMStoreSubsystem"));
		}

		if (!StoreClass || !StoreClass->IsChildOf(UEngineSubsystem::StaticClass()))
		{
			OutWarnings.Add(TEXT("LLM Store subsystem class was not found or is not an engine subsystem."));
			return nullptr;
		}

		UEngineSubsystem* Subsystem = GEngine->GetEngineSubsystemBase(StoreClass);
		ILLMStore* Store = Cast<ILLMStore>(Subsystem);
		if (!Store)
		{
			OutWarnings.Add(TEXT("LLM Store subsystem does not expose the public ILLMStore interface."));
			return nullptr;
		}

		return Store;
	}

	// The bridge deliberately copies between IIS and LLM Store DTOs instead of
	// sharing internal types. That keeps each plugin's public API independent
	// and makes boundary changes visible at compile time.
	void CopyRouteToIIS(const FLLMStoreEmbeddingRoute& InRoute, FIISEmbeddingRoute& OutRoute)
	{
		OutRoute.RouteId = InRoute.RouteId;
		OutRoute.TaskKind = InRoute.TaskKind;
		OutRoute.ProviderId = InRoute.ProviderId;
		OutRoute.ModelId = InRoute.ModelId;
		OutRoute.Dimensions = InRoute.Dimensions;
		OutRoute.bEnabled = InRoute.bEnabled;
		OutRoute.bLocalOnly = InRoute.bLocalOnly;
		OutRoute.bAllowFallback = InRoute.bAllowFallback;
		OutRoute.FallbackRouteIds = InRoute.FallbackRouteIds;
		OutRoute.AllowedRuntimeModes = InRoute.AllowedRuntimeModes;
	}

	void CopyRequestToLLMStore(const FIISEmbeddingRequest& InRequest, FLLMStoreEmbeddingRequest& OutRequest)
	{
		OutRequest.RouteId = InRequest.RouteId;
		OutRequest.TaskKind = InRequest.TaskKind;
		OutRequest.InputText = InRequest.InputText;
		OutRequest.Metadata = InRequest.Metadata;
		OutRequest.RequestedDimensions = InRequest.RequestedDimensions;
		OutRequest.bNormalize = InRequest.bNormalize;
		OutRequest.bLocalOnly = InRequest.bLocalOnly;
	}

	void CopyResponseToIIS(const FLLMStoreEmbeddingResponse& InResponse, FIISEmbeddingResponse& OutResponse)
	{
		OutResponse.bSuccess = InResponse.bSuccess;
		OutResponse.RouteId = InResponse.RouteId;
		OutResponse.ProviderId = InResponse.ProviderId;
		OutResponse.ModelId = InResponse.ModelId;
		OutResponse.Vector = InResponse.Vector;
		OutResponse.Dimensions = InResponse.Dimensions;
		OutResponse.ErrorCode = InResponse.ErrorCode;
		OutResponse.ErrorMessage = InResponse.ErrorMessage;
		OutResponse.Metadata = InResponse.Metadata;
	}

	class FLLMStoreIISEmbeddingRouteExecutor final : public IIISEmbeddingRouteExecutor
	{
	public:
		// IIS uses the executor id for diagnostics and for selecting which
		// registered embedding executor should handle a job.
		virtual FString GetExecutorId() const override
		{
			return BridgeExecutorId;
		}

		// Route resolution is delegated entirely to LLM Store governance. IIS
		// receives the resolved route shape, but never chooses a provider/model.
		virtual bool ResolveEmbeddingRoute(
			const FString& TaskKind,
			FIISEmbeddingRoute& OutRoute,
			TArray<FString>& OutWarnings) override
		{
			ILLMStore* Store = ResolveLLMStore(OutWarnings);
			if (!Store)
			{
				return false;
			}

			FLLMStoreEmbeddingRoute LLMStoreRoute;
			if (!Store->ResolveEmbeddingRoute(TaskKind, LLMStoreRoute))
			{
				OutWarnings.Add(FString::Printf(
					TEXT("LLM Store could not resolve embedding route '%s'."),
					*TaskKind));
				return false;
			}

			CopyRouteToIIS(LLMStoreRoute, OutRoute);
			return true;
		}

		// Execute a single IIS embedding request through LLM Store. No provider
		// specific calls, secrets, network behavior, or model fallback logic live
		// in this bridge.
		virtual bool ExecuteEmbeddingRoute(
			const FIISEmbeddingRequest& Request,
			FIISEmbeddingResponse& OutResponse) override
		{
			TArray<FString> Warnings;
			ILLMStore* Store = ResolveLLMStore(Warnings);
			if (!Store)
			{
				OutResponse.bSuccess = false;
				OutResponse.RouteId = Request.RouteId;
				OutResponse.ErrorCode = TEXT("embedding_executor_backend_unavailable");
				OutResponse.ErrorMessage = Warnings.Num() > 0
					? FString::Join(Warnings, TEXT(" "))
					: TEXT("LLM Store is not available to the IIS bridge.");
				return false;
			}

			FLLMStoreEmbeddingRequest LLMStoreRequest;
			CopyRequestToLLMStore(Request, LLMStoreRequest);

			FLLMStoreEmbeddingResponse LLMStoreResponse;
			const bool bExecuted = Store->ExecuteEmbeddingRoute(LLMStoreRequest, LLMStoreResponse);
			CopyResponseToIIS(LLMStoreResponse, OutResponse);
			return bExecuted && OutResponse.bSuccess;
		}
	};
}

void FInternalIndexServiceLLMStoreBridgeModule::StartupModule()
{
	// Registration is the only runtime side effect of the bridge. Once
	// registered, IIS can discover the executor through its own public registry.
	EmbeddingExecutor = MakeShared<FLLMStoreIISEmbeddingRouteExecutor>();
	if (FIISEmbeddingRouteExecutorRegistry::RegisterExecutor(EmbeddingExecutor.ToSharedRef()))
	{
		UE_LOG(LogIISLLMStoreBridge, Log, TEXT("IIS LLM Store Bridge registered embedding executor '%s'."), *BridgeExecutorId);
	}
	else
	{
		UE_LOG(LogIISLLMStoreBridge, Warning, TEXT("IIS LLM Store Bridge failed to register embedding executor."));
	}
}

void FInternalIndexServiceLLMStoreBridgeModule::ShutdownModule()
{
	// Always unregister by id before dropping the shared executor so IIS cannot
	// retain a stale callback after module shutdown.
	FIISEmbeddingRouteExecutorRegistry::UnregisterExecutor(BridgeExecutorId);
	EmbeddingExecutor.Reset();
	UE_LOG(LogIISLLMStoreBridge, Log, TEXT("IIS LLM Store Bridge unregistered embedding executor '%s'."), *BridgeExecutorId);
}

IMPLEMENT_MODULE(FInternalIndexServiceLLMStoreBridgeModule, InternalIndexServiceLLMStoreBridge)
