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

#pragma once

#include "CoreMinimal.h"
#include "IISEmbeddingTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InternalIndexServiceLLMStoreBridgeBlueprintLibrary.generated.h"

/**
 * Blueprint access to the optional IIS -> LLM Store embedding bridge.
 *
 * These nodes operate through the IIS embedding executor registry. They do not
 * select providers/models and do not call provider APIs directly; the registered
 * bridge executor forwards all embedding work to LLM Store governance.
 */
UCLASS()
class INTERNALINDEXSERVICELLMSTOREBRIDGE_API UInternalIndexServiceLLMStoreBridgeBlueprintLibrary
	: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Internal Index Service|LLM Store Bridge",
		meta = (DisplayName = "Get IIS LLM Store Bridge Executor Id",
			ToolTip = "Returns the IIS embedding executor id registered by this bridge.",
			Keywords = "IIS LLMStore LLM Store Bridge Executor"))
	static FString GetBridgeExecutorId();

	UFUNCTION(BlueprintPure, Category = "Internal Index Service|LLM Store Bridge",
		meta = (DisplayName = "Is IIS LLM Store Bridge Registered",
			ToolTip = "Returns true when the bridge executor is registered in the IIS embedding executor registry.",
			Keywords = "IIS LLMStore LLM Store Bridge Ready Registered"))
	static bool IsBridgeExecutorRegistered();

	UFUNCTION(BlueprintPure, Category = "Internal Index Service|LLM Store Bridge",
		meta = (DisplayName = "Get IIS Embedding Executor Ids",
			ToolTip = "Lists all IIS embedding executors currently registered with IIS.",
			Keywords = "IIS Embedding Executor Registry"))
	static TArray<FString> GetRegisteredEmbeddingExecutorIds();

	UFUNCTION(BlueprintCallable, Category = "Internal Index Service|LLM Store Bridge",
		meta = (DisplayName = "Resolve IIS Embedding Route via LLM Store",
			ToolTip = "Resolves an IIS embedding task kind through the LLM Store bridge executor. LLM Store owns provider/model selection.",
			Keywords = "IIS LLMStore LLM Store Embedding Route Resolve"))
	static bool ResolveEmbeddingRouteViaBridge(
		const FString& TaskKind,
		FIISEmbeddingRoute& OutRoute,
		TArray<FString>& OutWarnings);

	UFUNCTION(BlueprintPure, Category = "Internal Index Service|LLM Store Bridge",
		meta = (DisplayName = "Make IIS Embedding Request",
			ToolTip = "Builds an IIS embedding request struct for Execute IIS Embedding Route via LLM Store.",
			Keywords = "IIS LLMStore LLM Store Embedding Request Make"))
	static FIISEmbeddingRequest MakeEmbeddingRequest(
		const FString& RouteId,
		const FString& TaskKind,
		const FString& InputText,
		const TMap<FString, FString>& Metadata,
		int32 RequestedDimensions,
		bool bNormalize,
		bool bLocalOnly);

	UFUNCTION(BlueprintCallable, Category = "Internal Index Service|LLM Store Bridge",
		meta = (DisplayName = "Execute IIS Embedding Route via LLM Store",
			ToolTip = "Executes one IIS embedding request through the bridge executor. LLM Store applies provider routing, runtime mode, governance, and fallbacks.",
			Keywords = "IIS LLMStore LLM Store Embedding Execute Vector"))
	static bool ExecuteEmbeddingRouteViaBridge(
		const FIISEmbeddingRequest& Request,
		FIISEmbeddingResponse& OutResponse);

	UFUNCTION(BlueprintCallable, Category = "Internal Index Service|LLM Store Bridge",
		meta = (DisplayName = "Execute Simple IIS Embedding via LLM Store",
			ToolTip = "Convenience node for embedding one text input through an IIS/LLM Store route.",
			Keywords = "IIS LLMStore LLM Store Embedding Simple Vector"))
	static bool ExecuteSimpleEmbeddingViaBridge(
		const FString& RouteId,
		const FString& TaskKind,
		const FString& InputText,
		FIISEmbeddingResponse& OutResponse);
};
