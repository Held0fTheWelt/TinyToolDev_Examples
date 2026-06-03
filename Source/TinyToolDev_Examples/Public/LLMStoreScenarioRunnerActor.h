// Copyright (c) 2026 Tiny Tool Development. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/LLMStoreInterface.h"
#include "LLMStoreExampleScenarioLibrary.h"
#include "LLMStoreScenarioRunnerActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLLMStoreScenarioRunnerStatusEvent, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLLMStoreScenarioStartedEvent, const FLLMStoreExampleScenario&, Scenario);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FLLMStoreScenarioFinishedEvent,
	const FLLMStoreExampleScenario&, Scenario,
	const FLLMResponse&, Response);

/**
 * Runs one or many built-in sample scenarios through LLM Store.
 *
 * This actor is meant for Blueprint demo menus:
 * - place it in a level
 * - bind the events
 * - call Run Scenario By Id from a button
 * - or call Run All Scenarios Sequentially for a guided demo
 */
UCLASS(Blueprintable)
class TINYTOOLDEV_EXAMPLES_API ALLMStoreScenarioRunnerActor : public AActor
{
	GENERATED_BODY()

public:
	ALLMStoreScenarioRunnerActor();

	/** Scenario id used by Run Default Scenario and optional BeginPlay execution. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Scenario Runner")
	FString DefaultScenarioId = TEXT("quick-default");

	/** Ensures mock provider, sample routes, and prompt templates exist before running. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Scenario Runner")
	bool bEnsureSampleContentBeforeRun = true;

	/** Run DefaultScenarioId automatically on BeginPlay. Useful for a one-actor smoke test level. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Scenario Runner")
	bool bRunDefaultScenarioOnBeginPlay = false;

	/** Enables response cache for scenario requests. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Scenario Runner")
	bool bUseCache = true;

	/** Status text suitable for Print String or a debug Text widget. */
	UPROPERTY(BlueprintAssignable, Category="LLMStore|Scenario Runner")
	FLLMStoreScenarioRunnerStatusEvent OnScenarioRunnerStatus;

	/** Fires immediately before a scenario request is sent. */
	UPROPERTY(BlueprintAssignable, Category="LLMStore|Scenario Runner")
	FLLMStoreScenarioStartedEvent OnScenarioStarted;

	/** Fires when LLM Store returns the scenario response. */
	UPROPERTY(BlueprintAssignable, Category="LLMStore|Scenario Runner")
	FLLMStoreScenarioFinishedEvent OnScenarioFinished;

	/** Runs DefaultScenarioId. */
	UFUNCTION(BlueprintCallable, Category="LLMStore|Scenario Runner")
	void RunDefaultScenario();

	/** Finds a scenario by id and runs it. */
	UFUNCTION(BlueprintCallable, Category="LLMStore|Scenario Runner")
	void RunScenarioById(const FString& ScenarioId);

	/** Runs a scenario struct, usually from Get LLM Store Example Scenarios. */
	UFUNCTION(BlueprintCallable, Category="LLMStore|Scenario Runner")
	void RunScenario(const FLLMStoreExampleScenario& Scenario);

	/** Runs every built-in scenario one after another. */
	UFUNCTION(BlueprintCallable, Category="LLMStore|Scenario Runner")
	void RunAllScenariosSequentially();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	FLLMStoreExampleScenario ActiveScenario;

	UPROPERTY()
	TArray<FLLMStoreExampleScenario> ScenarioQueue;

	int32 ScenarioQueueIndex = 0;
	bool bRunningScenarioQueue = false;

	bool EnsureSampleContent();
	void BroadcastStatus(const FString& Message);
	void RunNextQueuedScenario();

	UFUNCTION()
	void HandleScenarioResponse(const FLLMResponse& Response);
};
