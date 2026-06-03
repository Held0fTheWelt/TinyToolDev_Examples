// Copyright (c) 2026 Tiny Tool Development. All rights reserved.

#include "LLMStoreScenarioRunnerActor.h"

#include "LLMStoreExampleBlueprintLibrary.h"

ALLMStoreScenarioRunnerActor::ALLMStoreScenarioRunnerActor()
{
	// The runner is event-driven: it sends requests only when BeginPlay or a
	// Blueprint button calls one of the Run* functions.
	PrimaryActorTick.bCanEverTick = false;
}

void ALLMStoreScenarioRunnerActor::BeginPlay()
{
	Super::BeginPlay();

	if (bRunDefaultScenarioOnBeginPlay)
	{
		RunDefaultScenario();
	}
}

void ALLMStoreScenarioRunnerActor::RunDefaultScenario()
{
	RunScenarioById(DefaultScenarioId);
}

void ALLMStoreScenarioRunnerActor::RunScenarioById(const FString& ScenarioId)
{
	FLLMStoreExampleScenario Scenario;
	if (!ULLMStoreExampleScenarioLibrary::FindExampleScenario(ScenarioId, Scenario))
	{
		BroadcastStatus(FString::Printf(TEXT("Scenario '%s' was not found."), *ScenarioId));
		return;
	}

	RunScenario(Scenario);
}

void ALLMStoreScenarioRunnerActor::RunScenario(const FLLMStoreExampleScenario& Scenario)
{
	if (bEnsureSampleContentBeforeRun && !EnsureSampleContent())
	{
		return;
	}

	ActiveScenario = Scenario;
	BroadcastStatus(FString::Printf(
		TEXT("Running scenario '%s' through route '%s'."),
		*Scenario.DisplayName,
		*Scenario.TaskKind));
	OnScenarioStarted.Broadcast(Scenario);

	FLLMResponseDelegate Complete;
	Complete.BindDynamic(this, &ALLMStoreScenarioRunnerActor::HandleScenarioResponse);
	ULLMStoreExampleScenarioLibrary::ExecuteExampleScenario(Scenario, bUseCache, Complete);
}

void ALLMStoreScenarioRunnerActor::RunAllScenariosSequentially()
{
	ScenarioQueue = ULLMStoreExampleScenarioLibrary::GetExampleScenarios();
	ScenarioQueueIndex = 0;
	bRunningScenarioQueue = true;

	BroadcastStatus(FString::Printf(TEXT("Running %d LLM Store scenario(s)."), ScenarioQueue.Num()));
	RunNextQueuedScenario();
}

bool ALLMStoreScenarioRunnerActor::EnsureSampleContent()
{
	FString Summary;
	if (!ULLMStoreExampleBlueprintLibrary::EnsureExampleSampleContent(Summary))
	{
		BroadcastStatus(Summary);
		return false;
	}

	BroadcastStatus(Summary);
	return true;
}

void ALLMStoreScenarioRunnerActor::BroadcastStatus(const FString& Message)
{
	UE_LOG(LogTemp, Display, TEXT("LLM Store Scenario Runner: %s"), *Message);
	OnScenarioRunnerStatus.Broadcast(Message);
}

void ALLMStoreScenarioRunnerActor::RunNextQueuedScenario()
{
	if (!ScenarioQueue.IsValidIndex(ScenarioQueueIndex))
	{
		bRunningScenarioQueue = false;
		BroadcastStatus(TEXT("Scenario queue finished."));
		return;
	}

	RunScenario(ScenarioQueue[ScenarioQueueIndex]);
}

void ALLMStoreScenarioRunnerActor::HandleScenarioResponse(const FLLMResponse& Response)
{
	const FString Status = Response.Result.bSuccess
		? FString::Printf(TEXT("Scenario '%s' completed with model '%s'."), *ActiveScenario.DisplayName, *Response.ModelId)
		: FString::Printf(TEXT("Scenario '%s' failed: %s"), *ActiveScenario.DisplayName, *Response.Result.Message);

	BroadcastStatus(Status);
	OnScenarioFinished.Broadcast(ActiveScenario, Response);

	if (bRunningScenarioQueue)
	{
		++ScenarioQueueIndex;
		RunNextQueuedScenario();
	}
}
