// Copyright (c) 2026 Tiny Tool Development. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/LLMStoreInterface.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/LLMStoreTypes.h"
#include "LLMStoreExampleScenarioLibrary.generated.h"

/**
 * One ready-to-run sample prompt.
 *
 * The struct is BlueprintType so buyers can build a simple list UI without
 * creating Data Assets first. Each scenario maps to one route in Config/LLMStore.json.
 */
USTRUCT(BlueprintType)
struct FLLMStoreExampleScenario
{
	GENERATED_BODY()

	/** Stable id used by FindExampleScenario. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Example|Scenario")
	FString Id;

	/** Human-readable title for menus, buttons, or tutorial output. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Example|Scenario")
	FString DisplayName;

	/** Route called through LLM Store. This is intentionally provider-neutral. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Example|Scenario")
	FString TaskKind;

	/** System/developer instruction for this sample task. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Example|Scenario", meta=(MultiLine=true))
	FString SystemPrompt;

	/** User prompt for this sample task. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Example|Scenario", meta=(MultiLine=true))
	FString Prompt;

	/** Beginner explanation of what this scenario is meant to teach. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Example|Scenario", meta=(MultiLine=true))
	FString TutorialNote;

	/** Suggested request budget. The mock provider has no cost, but real providers can use this as a guardrail. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Example|Scenario")
	int32 SuggestedBudgetTokens = 512;

	/** True when this is a reasonable scenario to test with Execute Route Streaming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LLMStore|Example|Scenario")
	bool bGoodForStreaming = false;
};

/**
 * Blueprint-accessible catalog of sample LLM Store prompts.
 *
 * This intentionally returns code-defined data instead of requiring .uasset
 * files. It stays source-control friendly and works immediately after compile.
 */
UCLASS()
class TINYTOOLDEV_EXAMPLES_API ULLMStoreExampleScenarioLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns all built-in sample scenarios that match the mock routes in Config/LLMStore.json. */
	UFUNCTION(BlueprintPure, Category="LLMStore|Example|Scenario", meta=(DisplayName="Get LLM Store Example Scenarios"))
	static TArray<FLLMStoreExampleScenario> GetExampleScenarios();

	/** Finds one scenario by Id. Use this when a UI button stores only the scenario id. */
	UFUNCTION(BlueprintPure, Category="LLMStore|Example|Scenario", meta=(DisplayName="Find LLM Store Example Scenario"))
	static bool FindExampleScenario(const FString& ScenarioId, FLLMStoreExampleScenario& OutScenario);

	/** Converts a scenario into FLLMRequest so it can be passed to Execute Route. */
	UFUNCTION(BlueprintPure, Category="LLMStore|Example|Scenario", meta=(DisplayName="Make LLM Request From Scenario"))
	static FLLMRequest MakeRequestFromScenario(const FLLMStoreExampleScenario& Scenario, bool bUseCache);

	/** Executes a scenario through ULLMStoreSubsystem in one Blueprint-friendly call. */
	UFUNCTION(BlueprintCallable, Category="LLMStore|Example|Scenario", meta=(DisplayName="Execute LLM Store Example Scenario"))
	static void ExecuteExampleScenario(const FLLMStoreExampleScenario& Scenario, bool bUseCache, FLLMResponseDelegate OnComplete);

	/** Formats a scenario for Print String, debug logs, or simple list widgets. */
	UFUNCTION(BlueprintPure, Category="LLMStore|Example|Scenario", meta=(DisplayName="Format LLM Store Example Scenario"))
	static FString FormatScenario(const FLLMStoreExampleScenario& Scenario);
};
