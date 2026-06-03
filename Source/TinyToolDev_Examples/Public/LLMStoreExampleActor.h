// Copyright (c) 2026 Tiny Tool Development. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/LLMStoreInterface.h"
#include "Types/LLMStoreTypes.h"
#include "LLMStoreExampleActor.generated.h"

class ULLMStoreSubsystem;

/** Human-readable tutorial/status messages. Bind this in Blueprint to print each step of the example flow. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLLMStoreExampleStatusEvent, const FString&, Message);
/** Streaming delta event. A provider can emit many chunks, or just one final chunk when native streaming is unavailable. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLLMStoreExampleStreamEvent, const FString&, Delta, bool, bDone);
/** Final response event split into simple Blueprint pins so beginners do not have to break the full response struct first. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SevenParams(
	FLLMStoreExampleResponseEvent,
	bool, bSuccess,
	const FString&, Content,
	const FString&, Message,
	const FString&, ModelId,
	int32, TotalTokens,
	float, EstimatedCost,
	const FString&, Currency);

/**
 * Drop this actor into a level to demonstrate a governed LLM Store request.
 *
 * Tutorial sequence:
 * 1. Optionally create/validate the offline mock setup.
 * 2. Ask LLM Store for readiness information.
 * 3. Resolve a stable route name such as "default".
 * 4. Build an FLLMRequest without provider-specific code.
 * 5. Execute the route through ULLMStoreSubsystem.
 * 6. Broadcast response data in beginner-friendly Blueprint events.
 */
UCLASS()
class TINYTOOLDEV_EXAMPLES_API ALLMStoreExampleActor : public AActor
{
	GENERATED_BODY()

public:
	ALLMStoreExampleActor();

	/** When true, BeginPlay immediately sends Prompt through TaskKind after readiness and route checks. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Example")
	bool bRunOnBeginPlay = true;

	/** Keeps the sample self-contained by creating the built-in mock provider/model/default route when needed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Example")
	bool bEnsureMockSetupOnBeginPlay = true;

	/** Uses ExecuteRouteStreaming instead of ExecuteRoute. Mock providers usually emit one final chunk. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Example")
	bool bUseStreaming = false;

	/** Allows LLM Store to use its response cache for identical route/model/prompt combinations. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Example")
	bool bUseCache = true;

	/** Stable route key. The route decides provider/model/policy; this actor never hard-codes those choices. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Example")
	FString TaskKind = TEXT("default");

	/** User prompt passed to FLLMRequest::Prompt. Replace this with your tool or gameplay question. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Example", meta=(MultiLine=true))
	FString Prompt = TEXT("Explain in one sentence what the LLM Store does in this Unreal project.");

	/** System/developer instruction passed with the request. Keep project policy and tone guidance here. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Example", meta=(MultiLine=true))
	FString SystemPrompt = TEXT("You are a concise Unreal Engine assistant. Answer in one short paragraph.");

	/** Optional token budget for governance and estimates. Zero means no explicit budget. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Example")
	int32 MaxBudgetTokens = 512;

	/** Fires for setup, readiness, route resolution, stream diagnostics, and final formatted response text. */
	UPROPERTY(BlueprintAssignable, Category="LLMStore|Example")
	FLLMStoreExampleStatusEvent OnExampleStatus;

	/** Fires when ExecuteRouteStreaming reports a chunk. Use this for progressive UI text. */
	UPROPERTY(BlueprintAssignable, Category="LLMStore|Example")
	FLLMStoreExampleStreamEvent OnStreamChunk;

	/** Fires once for the final response. This is the easiest event for Blueprint UI binding. */
	UPROPERTY(BlueprintAssignable, Category="LLMStore|Example")
	FLLMStoreExampleResponseEvent OnResponse;

	/** Creates the offline mock quick-start entries and the extra sample routes/templates idempotently. */
	UFUNCTION(BlueprintCallable, Category="LLMStore|Example")
	bool EnsureMockSetup(FString& OutSummary);

	/** Reads the store readiness snapshot and broadcasts a beginner-readable status message. */
	UFUNCTION(BlueprintCallable, Category="LLMStore|Example")
	void CheckReadiness();

	/** Resolves TaskKind and broadcasts the selected provider/model without executing a prompt. */
	UFUNCTION(BlueprintCallable, Category="LLMStore|Example")
	void ResolveRoute();

	/** Sends the actor's Prompt property through TaskKind. */
	UFUNCTION(BlueprintCallable, Category="LLMStore|Example")
	void SendPrompt();

	/** Sends a one-off prompt while keeping the actor's TaskKind, SystemPrompt, cache, and budget settings. */
	UFUNCTION(BlueprintCallable, Category="LLMStore|Example")
	void SendPromptText(const FString& PromptOverride);

protected:
	virtual void BeginPlay() override;

private:
	/** Engine subsystem lookup. Returns null when the plugin is disabled or the engine is not initialized. */
	ULLMStoreSubsystem* GetStore() const;
	/** Converts actor properties into the provider-neutral request struct consumed by LLM Store. */
	FLLMRequest BuildRequest(const FString& PromptText, bool bStreamingRequest) const;
	/** Logs to Output Log and forwards the same text to Blueprint. */
	void BroadcastStatus(const FString& Message);

	/** Bound to FLLMResponseDelegate. Splits the final response into simple event pins. */
	UFUNCTION()
	void HandleResponse(const FLLMResponse& Response);

	/** Bound to FLLMStreamChunkDelegate. Forwards streaming deltas to Blueprint. */
	UFUNCTION()
	void HandleStreamChunk(const FLLMStreamChunk& Chunk);
};
