// Copyright (c) 2026 Tiny Tool Development. All rights reserved.

#include "LLMStoreConversationComponent.h"

#include "Engine/Engine.h"
#include "LLMStoreExampleBlueprintLibrary.h"
#include "LLMStoreSubsystem.h"

ULLMStoreConversationComponent::ULLMStoreConversationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULLMStoreConversationComponent::SendUserMessage(const FString& Message)
{
	const FString CleanMessage = Message.TrimStartAndEnd();
	if (CleanMessage.IsEmpty())
	{
		BroadcastStatus(TEXT("Conversation message is empty."));
		return;
	}

	if (bEnsureSampleContentBeforeSend)
	{
		FString Summary;
		if (!ULLMStoreExampleBlueprintLibrary::EnsureExampleSampleContent(Summary))
		{
			BroadcastStatus(Summary);
			return;
		}
	}

	ULLMStoreSubsystem* Store = GEngine ? GEngine->GetEngineSubsystem<ULLMStoreSubsystem>() : nullptr;
	if (!Store)
	{
		BroadcastStatus(TEXT("LLM Store subsystem is not available."));
		return;
	}

	PendingUserMessage = CleanMessage;

	FLLMRequest Request;
	Request.Operation = ELLMRequestOperation::Chat;
	Request.TaskKind = TaskKind.TrimStartAndEnd().IsEmpty() ? TEXT("default") : TaskKind;
	Request.SystemPrompt = SystemPrompt;
	Request.Prompt = BuildPromptWithHistory(CleanMessage);
	Request.bUseCache = bUseCache;
	Request.CacheTtlSeconds = 300;
	Request.MaxBudgetTokens = FMath::Max(0, MaxBudgetTokens);

	FLLMResponseDelegate Complete;
	Complete.BindDynamic(this, &ULLMStoreConversationComponent::HandleConversationResponse);
	Store->ExecuteRoute(Request, Complete);

	BroadcastStatus(FString::Printf(TEXT("Sent conversation message through route '%s'."), *Request.TaskKind));
}

void ULLMStoreConversationComponent::ClearConversation()
{
	Messages.Reset();
	PendingUserMessage.Reset();
	BroadcastStatus(TEXT("Conversation transcript cleared."));
}

TArray<FLLMStoreConversationMessage> ULLMStoreConversationComponent::GetMessages() const
{
	return Messages;
}

FString ULLMStoreConversationComponent::GetTranscriptText() const
{
	TArray<FString> Lines;
	for (const FLLMStoreConversationMessage& Message : Messages)
	{
		Lines.Add(FString::Printf(TEXT("%s: %s"), *Message.Role, *Message.Content));
	}
	return FString::Join(Lines, TEXT("\n"));
}

FLLMStoreConversationMessage ULLMStoreConversationComponent::MakeMessage(
	const FString& Role,
	const FString& Content) const
{
	FLLMStoreConversationMessage Message;
	Message.Role = Role;
	Message.Content = Content;
	Message.TimestampUtc = FDateTime::UtcNow().ToIso8601();
	return Message;
}

FString ULLMStoreConversationComponent::BuildPromptWithHistory(const FString& NewUserMessage) const
{
	TArray<FString> Lines;
	Lines.Add(TEXT("Conversation so far:"));

	const int32 FirstMessageIndex = FMath::Max(0, Messages.Num() - FMath::Max(0, MaxHistoryMessages));
	for (int32 Index = FirstMessageIndex; Index < Messages.Num(); ++Index)
	{
		const FLLMStoreConversationMessage& Message = Messages[Index];
		Lines.Add(FString::Printf(TEXT("%s: %s"), *Message.Role, *Message.Content));
	}

	Lines.Add(FString::Printf(TEXT("user: %s"), *NewUserMessage));
	Lines.Add(TEXT("assistant:"));
	return FString::Join(Lines, TEXT("\n"));
}

void ULLMStoreConversationComponent::BroadcastStatus(const FString& Message)
{
	UE_LOG(LogTemp, Display, TEXT("LLM Store Conversation: %s"), *Message);
	OnConversationStatus.Broadcast(Message);
}

void ULLMStoreConversationComponent::HandleConversationResponse(const FLLMResponse& Response)
{
	if (!PendingUserMessage.IsEmpty())
	{
		Messages.Add(MakeMessage(TEXT("user"), PendingUserMessage));
		PendingUserMessage.Reset();
	}

	if (!Response.Result.bSuccess)
	{
		BroadcastStatus(FString::Printf(TEXT("Conversation response failed: %s"), *Response.Result.Message));
		OnAssistantResponse.Broadcast(FString(), Response);
		return;
	}

	Messages.Add(MakeMessage(TEXT("assistant"), Response.Content));
	BroadcastStatus(FString::Printf(
		TEXT("Assistant response received from model '%s' using %d token(s)."),
		*Response.ModelId,
		Response.Usage.TotalTokens));
	OnAssistantResponse.Broadcast(Response.Content, Response);
}
