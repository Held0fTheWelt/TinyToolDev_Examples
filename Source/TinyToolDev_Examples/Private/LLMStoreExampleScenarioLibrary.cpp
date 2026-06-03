// Copyright (c) 2026 Tiny Tool Development. All rights reserved.

#include "LLMStoreExampleScenarioLibrary.h"

#include "Engine/Engine.h"
#include "LLMStoreSubsystem.h"

namespace
{
	FLLMStoreExampleScenario MakeScenario(
		const TCHAR* Id,
		const TCHAR* DisplayName,
		const TCHAR* TaskKind,
		const TCHAR* SystemPrompt,
		const TCHAR* Prompt,
		const TCHAR* TutorialNote,
		int32 Budget,
		bool bGoodForStreaming)
	{
		FLLMStoreExampleScenario Scenario;
		Scenario.Id = Id;
		Scenario.DisplayName = DisplayName;
		Scenario.TaskKind = TaskKind;
		Scenario.SystemPrompt = SystemPrompt;
		Scenario.Prompt = Prompt;
		Scenario.TutorialNote = TutorialNote;
		Scenario.SuggestedBudgetTokens = Budget;
		Scenario.bGoodForStreaming = bGoodForStreaming;
		return Scenario;
	}

	FLLMResponse MakeScenarioFailure(const FString& Message)
	{
		FLLMResponse Response;
		Response.Result.bSuccess = false;
		Response.Result.ErrorCode = ELLMResultCode::MissingProvider;
		Response.Result.Message = Message;
		return Response;
	}
}

TArray<FLLMStoreExampleScenario> ULLMStoreExampleScenarioLibrary::GetExampleScenarios()
{
	TArray<FLLMStoreExampleScenario> Scenarios;

	Scenarios.Add(MakeScenario(
		TEXT("quick-default"),
		TEXT("Quick Default Check"),
		TEXT("default"),
		TEXT("You are a concise Unreal Engine assistant. Answer in one short paragraph."),
		TEXT("Explain in one sentence what the LLM Store does in this Unreal project."),
		TEXT("Use this first. It proves that the subsystem, mock provider, model, and default route are working."),
		512,
		false));

	Scenarios.Add(MakeScenario(
		TEXT("asset-explain-crate"),
		TEXT("Explain Content Asset"),
		TEXT("asset.explain"),
		TEXT("You help Unreal Engine artists understand production assets."),
		TEXT("StaticMesh'/Game/Props/SM_SupplyCrate.SM_SupplyCrate' has 24k triangles, three material slots, Nanite enabled, and no collision preset."),
		TEXT("Shows how a content tool can call a route without caring which model explains the asset."),
		768,
		false));

	Scenarios.Add(MakeScenario(
		TEXT("docs-summarize-plugin"),
		TEXT("Summarize Plugin Notes"),
		TEXT("docs.summarize"),
		TEXT("You summarize technical notes for Unreal Engine leads."),
		TEXT("The plugin adds an editor tab, stores provider config in Config/LLMStore.json, stores secrets separately, and exposes Blueprint route execution."),
		TEXT("Good for documentation assistants, release note helpers, and producer-facing summaries."),
		1024,
		false));

	Scenarios.Add(MakeScenario(
		TEXT("npc-dialogue-merchant"),
		TEXT("Draft NPC Dialogue"),
		TEXT("npc.dialogue"),
		TEXT("You write short placeholder dialogue for game prototypes."),
		TEXT("A tired market trader warns the player that the old bridge is unsafe after the storm."),
		TEXT("Shows a narrative route that can later use a different provider or stronger model."),
		768,
		true));

	Scenarios.Add(MakeScenario(
		TEXT("code-review-async"),
		TEXT("Review Code Note"),
		TEXT("code.review"),
		TEXT("You review Unreal Engine C++ notes with practical, testable advice."),
		TEXT("A Blueprint button calls an HTTP-backed LLM route several times in a loop and updates a UMG widget from each callback."),
		TEXT("Shows how engineering tools can route review tasks through governance and cost tracking."),
		1024,
		false));

	Scenarios.Add(MakeScenario(
		TEXT("qa-testplan-save"),
		TEXT("Create QA Test Plan"),
		TEXT("qa.testplan"),
		TEXT("You create compact QA checklists for Unreal Engine features."),
		TEXT("The settings panel exports LLM Store cost reports to CSV and lets users clear the local response cache."),
		TEXT("Shows a production-support route for QA, release validation, and automation planning."),
		1024,
		false));

	return Scenarios;
}

bool ULLMStoreExampleScenarioLibrary::FindExampleScenario(
	const FString& ScenarioId,
	FLLMStoreExampleScenario& OutScenario)
{
	const FString NormalizedId = ScenarioId.TrimStartAndEnd();
	for (const FLLMStoreExampleScenario& Scenario : GetExampleScenarios())
	{
		if (Scenario.Id.Equals(NormalizedId, ESearchCase::IgnoreCase))
		{
			OutScenario = Scenario;
			return true;
		}
	}

	return false;
}

FLLMRequest ULLMStoreExampleScenarioLibrary::MakeRequestFromScenario(
	const FLLMStoreExampleScenario& Scenario,
	bool bUseCache)
{
	FLLMRequest Request;
	Request.Operation = ELLMRequestOperation::Chat;
	Request.TaskKind = Scenario.TaskKind.TrimStartAndEnd().IsEmpty() ? TEXT("default") : Scenario.TaskKind;
	Request.SystemPrompt = Scenario.SystemPrompt;
	Request.Prompt = Scenario.Prompt;
	Request.bUseCache = bUseCache;
	Request.CacheTtlSeconds = 300;
	Request.MaxBudgetTokens = FMath::Max(0, Scenario.SuggestedBudgetTokens);
	return Request;
}

void ULLMStoreExampleScenarioLibrary::ExecuteExampleScenario(
	const FLLMStoreExampleScenario& Scenario,
	bool bUseCache,
	FLLMResponseDelegate OnComplete)
{
	ULLMStoreSubsystem* Store = GEngine ? GEngine->GetEngineSubsystem<ULLMStoreSubsystem>() : nullptr;
	if (!Store)
	{
		OnComplete.ExecuteIfBound(MakeScenarioFailure(TEXT("LLM Store subsystem is not available.")));
		return;
	}

	Store->ExecuteRoute(MakeRequestFromScenario(Scenario, bUseCache), MoveTemp(OnComplete));
}

FString ULLMStoreExampleScenarioLibrary::FormatScenario(const FLLMStoreExampleScenario& Scenario)
{
	return FString::Printf(
		TEXT("%s\nRoute: %s\nBudget: %d tokens\n\nPrompt:\n%s\n\nWhy it exists:\n%s"),
		*Scenario.DisplayName,
		*Scenario.TaskKind,
		Scenario.SuggestedBudgetTokens,
		*Scenario.Prompt,
		*Scenario.TutorialNote);
}
