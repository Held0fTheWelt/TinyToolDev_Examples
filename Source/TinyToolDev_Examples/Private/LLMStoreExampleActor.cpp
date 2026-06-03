// Copyright (c) 2026 Tiny Tool Development. All rights reserved.

#include "LLMStoreExampleActor.h"

#include "Engine/Engine.h"
#include "LLMStoreExampleBlueprintLibrary.h"
#include "LLMStoreSubsystem.h"

ALLMStoreExampleActor::ALLMStoreExampleActor()
{
	// This is a request demo, not a per-frame actor. Keeping ticking disabled
	// makes the example easier to reason about and avoids accidental repeated calls.
	PrimaryActorTick.bCanEverTick = false;
}

void ALLMStoreExampleActor::BeginPlay()
{
	Super::BeginPlay();

	// Beginner-friendly first run:
	// the mock setup keeps the sample useful even before a buyer configures
	// Ollama, OpenAI, Anthropic, or another provider.
	if (bEnsureMockSetupOnBeginPlay)
	{
		FString Summary;
		EnsureMockSetup(Summary);
		BroadcastStatus(Summary);
	}

	// These two calls are intentionally separate from SendPrompt. They teach the
	// normal LLM Store workflow: first inspect readiness, then resolve the route,
	// then execute only when the route is understandable.
	CheckReadiness();
	ResolveRoute();

	if (bRunOnBeginPlay)
	{
		SendPrompt();
	}
}

ULLMStoreSubsystem* ALLMStoreExampleActor::GetStore() const
{
	// LLM Store lives as an engine subsystem. Consumers should request it from
	// GEngine instead of constructing provider or routing objects themselves.
	return GEngine ? GEngine->GetEngineSubsystem<ULLMStoreSubsystem>() : nullptr;
}

bool ALLMStoreExampleActor::EnsureMockSetup(FString& OutSummary)
{
	ULLMStoreSubsystem* Store = GetStore();
	if (!Store)
	{
		OutSummary = TEXT("LLM Store subsystem is not available. Check that the LLMStore plugin is enabled.");
		return false;
	}

	return ULLMStoreExampleBlueprintLibrary::EnsureExampleSampleContent(OutSummary);
}

void ALLMStoreExampleActor::CheckReadiness()
{
	ULLMStoreSubsystem* Store = GetStore();
	if (!Store)
	{
		BroadcastStatus(TEXT("Readiness check failed: LLM Store subsystem is not available."));
		return;
	}

	BroadcastStatus(ULLMStoreExampleBlueprintLibrary::FormatReadiness(Store->GetReadiness()));
}

void ALLMStoreExampleActor::ResolveRoute()
{
	ULLMStoreSubsystem* Store = GetStore();
	if (!Store)
	{
		BroadcastStatus(TEXT("Route resolve failed: LLM Store subsystem is not available."));
		return;
	}

	FLLMResolvedRoute Resolved;
	const FLLMResult Result = Store->ResolveRoute(TaskKind, Resolved);
	if (!Result.bSuccess)
	{
		BroadcastStatus(FString::Printf(TEXT("Route '%s' failed: %s"), *TaskKind, *Result.Message));
		return;
	}

	BroadcastStatus(FString::Printf(
		TEXT("Route '%s' -> provider '%s' (%s), model '%s'."),
		*TaskKind,
		*Resolved.Provider.Id,
		*Resolved.Provider.Type,
		*Resolved.Model.Id));
}

void ALLMStoreExampleActor::SendPrompt()
{
	SendPromptText(Prompt);
}

void ALLMStoreExampleActor::SendPromptText(const FString& PromptOverride)
{
	ULLMStoreSubsystem* Store = GetStore();
	if (!Store)
	{
		BroadcastStatus(TEXT("Prompt execution failed: LLM Store subsystem is not available."));
		return;
	}

	FLLMResponseDelegate Complete;
	Complete.BindDynamic(this, &ALLMStoreExampleActor::HandleResponse);

	// Normal and streaming execution share the same request data. The only
	// difference is the delegate shape: streaming can update UI progressively,
	// while the final response delegate always reports completion.
	if (bUseStreaming)
	{
		FLLMStreamChunkDelegate Chunk;
		Chunk.BindDynamic(this, &ALLMStoreExampleActor::HandleStreamChunk);
		Store->ExecuteRouteStreaming(BuildRequest(PromptOverride, true), Chunk, Complete);
	}
	else
	{
		Store->ExecuteRoute(BuildRequest(PromptOverride, false), Complete);
	}
}

FLLMRequest ALLMStoreExampleActor::BuildRequest(const FString& PromptText, bool bStreamingRequest) const
{
	// The helper library fills in safe defaults and keeps the tutorial graph
	// compact. Production code can build FLLMRequest directly when desired.
	FLLMRequest Request = ULLMStoreExampleBlueprintLibrary::MakeExampleRequest(
		TaskKind,
		PromptText,
		SystemPrompt,
		bUseCache,
		MaxBudgetTokens);
	Request.bStream = bStreamingRequest;
	return Request;
}

void ALLMStoreExampleActor::BroadcastStatus(const FString& Message)
{
	UE_LOG(LogTemp, Display, TEXT("LLM Store Example: %s"), *Message);
	OnExampleStatus.Broadcast(Message);
}

void ALLMStoreExampleActor::HandleResponse(const FLLMResponse& Response)
{
	// Response.Result contains the provider-neutral success state. The content,
	// model id, token usage, estimated cost, and cache flag are also provider
	// neutral, so Blueprint UI does not need provider-specific parsing.
	const FString Message = Response.Result.bSuccess
		? TEXT("LLM request completed.")
		: Response.Result.Message;

	BroadcastStatus(ULLMStoreExampleBlueprintLibrary::FormatResponse(Response));
	OnResponse.Broadcast(
		Response.Result.bSuccess,
		Response.Content,
		Message,
		Response.ModelId,
		Response.Usage.TotalTokens,
		Response.Usage.EstimatedCost,
		Response.Usage.Currency);
}

void ALLMStoreExampleActor::HandleStreamChunk(const FLLMStreamChunk& Chunk)
{
	// Native streaming providers can call this many times. Providers without
	// streaming support may emit a single final chunk; that still keeps Blueprint
	// code compatible with both provider types.
	OnStreamChunk.Broadcast(Chunk.Delta, Chunk.bDone);
	if (!Chunk.Delta.IsEmpty())
	{
		BroadcastStatus(FString::Printf(TEXT("Stream chunk: %s"), *Chunk.Delta));
	}
}
