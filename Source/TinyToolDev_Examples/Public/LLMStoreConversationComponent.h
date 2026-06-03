// Copyright (c) 2026 Tiny Tool Development. All rights reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Interfaces/LLMStoreInterface.h"
#include "LLMStoreConversationComponent.generated.h"

/** One message in the tutorial conversation transcript. */
USTRUCT(BlueprintType)
struct FLLMStoreConversationMessage
{
	GENERATED_BODY()

	/** Usually "user", "assistant", or "system". */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Conversation")
	FString Role;

	/** Message text. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Conversation", meta=(MultiLine=true))
	FString Content;

	/** UTC timestamp for debug output and UI display. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Conversation")
	FString TimestampUtc;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLLMStoreConversationStatusEvent, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FLLMStoreConversationResponseEvent,
	const FString&, AssistantMessage,
	const FLLMResponse&, Response);

/**
 * Minimal chat-style component for gameplay actors, UMG widgets, or prototypes.
 *
 * It demonstrates how to keep short local history, build a provider-neutral
 * FLLMRequest, and send it through a route such as npc.dialogue or default.
 */
UCLASS(ClassGroup=(LLMStore), Blueprintable, meta=(BlueprintSpawnableComponent))
class TINYTOOLDEV_EXAMPLES_API ULLMStoreConversationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULLMStoreConversationComponent();

	/** Route used for every user message. Try "npc.dialogue" for the narrative sample. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Conversation")
	FString TaskKind = TEXT("npc.dialogue");

	/** System instruction prepended to each request. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Conversation", meta=(MultiLine=true))
	FString SystemPrompt = TEXT("You are a concise in-game assistant. Keep replies short and useful.");

	/** How many previous messages to include when building the next prompt. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Conversation", meta=(ClampMin="0", UIMin="0"))
	int32 MaxHistoryMessages = 6;

	/** Optional token budget for each request. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Conversation", meta=(ClampMin="0", UIMin="0"))
	int32 MaxBudgetTokens = 768;

	/** Enables LLM Store response cache. Disable for true conversation experiments with real providers. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Conversation")
	bool bUseCache = false;

	/** Ensures mock/sample routes are present before the first request. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Conversation")
	bool bEnsureSampleContentBeforeSend = true;

	/** Fires for setup, send, and error messages. */
	UPROPERTY(BlueprintAssignable, Category="LLMStore|Conversation")
	FLLMStoreConversationStatusEvent OnConversationStatus;

	/** Fires when the assistant response arrives. */
	UPROPERTY(BlueprintAssignable, Category="LLMStore|Conversation")
	FLLMStoreConversationResponseEvent OnAssistantResponse;

	/** Sends a user message through TaskKind and appends both user/assistant messages to the transcript. */
	UFUNCTION(BlueprintCallable, Category="LLMStore|Conversation")
	void SendUserMessage(const FString& Message);

	/** Clears local tutorial transcript only. It does not clear LLM Store cache or cost ledger. */
	UFUNCTION(BlueprintCallable, Category="LLMStore|Conversation")
	void ClearConversation();

	/** Returns a copy of all local transcript messages. */
	UFUNCTION(BlueprintPure, Category="LLMStore|Conversation")
	TArray<FLLMStoreConversationMessage> GetMessages() const;

	/** Formats the transcript for Print String, debug panels, or simple UMG text blocks. */
	UFUNCTION(BlueprintPure, Category="LLMStore|Conversation")
	FString GetTranscriptText() const;

private:
	UPROPERTY()
	TArray<FLLMStoreConversationMessage> Messages;

	FString PendingUserMessage;

	FLLMStoreConversationMessage MakeMessage(const FString& Role, const FString& Content) const;
	FString BuildPromptWithHistory(const FString& NewUserMessage) const;
	void BroadcastStatus(const FString& Message);

	UFUNCTION()
	void HandleConversationResponse(const FLLMResponse& Response);
};
