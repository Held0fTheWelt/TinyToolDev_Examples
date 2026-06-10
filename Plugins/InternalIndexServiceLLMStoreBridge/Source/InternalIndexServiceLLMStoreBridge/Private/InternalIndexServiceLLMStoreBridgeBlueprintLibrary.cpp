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

#include "InternalIndexServiceLLMStoreBridgeBlueprintLibrary.h"

namespace
{
	const FString BlueprintBridgeExecutorId = TEXT("llmstore");
}

FString UInternalIndexServiceLLMStoreBridgeBlueprintLibrary::GetBridgeExecutorId()
{
	return BlueprintBridgeExecutorId;
}

bool UInternalIndexServiceLLMStoreBridgeBlueprintLibrary::IsBridgeExecutorRegistered()
{
	return FIISEmbeddingRouteExecutorRegistry::GetExecutor(BlueprintBridgeExecutorId).IsValid();
}

TArray<FString> UInternalIndexServiceLLMStoreBridgeBlueprintLibrary::GetRegisteredEmbeddingExecutorIds()
{
	return FIISEmbeddingRouteExecutorRegistry::GetExecutorIds();
}

bool UInternalIndexServiceLLMStoreBridgeBlueprintLibrary::ResolveEmbeddingRouteViaBridge(
	const FString& TaskKind,
	FIISEmbeddingRoute& OutRoute,
	TArray<FString>& OutWarnings)
{
	OutRoute = FIISEmbeddingRoute();
	OutWarnings.Reset();
	return FIISEmbeddingRouteExecutorRegistry::ResolveEmbeddingRoute(
		TaskKind,
		OutRoute,
		OutWarnings,
		BlueprintBridgeExecutorId);
}

FIISEmbeddingRequest UInternalIndexServiceLLMStoreBridgeBlueprintLibrary::MakeEmbeddingRequest(
	const FString& RouteId,
	const FString& TaskKind,
	const FString& InputText,
	const TMap<FString, FString>& Metadata,
	const int32 RequestedDimensions,
	const bool bNormalize,
	const bool bLocalOnly)
{
	FIISEmbeddingRequest Request;
	Request.RouteId = RouteId;
	Request.TaskKind = TaskKind;
	Request.InputText = InputText;
	Request.Metadata = Metadata;
	Request.RequestedDimensions = RequestedDimensions;
	Request.bNormalize = bNormalize;
	Request.bLocalOnly = bLocalOnly;
	return Request;
}

bool UInternalIndexServiceLLMStoreBridgeBlueprintLibrary::ExecuteEmbeddingRouteViaBridge(
	const FIISEmbeddingRequest& Request,
	FIISEmbeddingResponse& OutResponse)
{
	OutResponse = FIISEmbeddingResponse();
	return FIISEmbeddingRouteExecutorRegistry::ExecuteEmbeddingRoute(
		Request,
		OutResponse,
		BlueprintBridgeExecutorId);
}

bool UInternalIndexServiceLLMStoreBridgeBlueprintLibrary::ExecuteSimpleEmbeddingViaBridge(
	const FString& RouteId,
	const FString& TaskKind,
	const FString& InputText,
	FIISEmbeddingResponse& OutResponse)
{
	TMap<FString, FString> Metadata;
	Metadata.Add(TEXT("caller"), TEXT("blueprint"));

	const FIISEmbeddingRequest Request = MakeEmbeddingRequest(
		RouteId,
		TaskKind,
		InputText,
		Metadata,
		0,
		true,
		true);

	return ExecuteEmbeddingRouteViaBridge(Request, OutResponse);
}
