// Copyright (c) 2026 Tiny Tool Development. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/LLMStoreInterface.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/LLMStoreTypes.h"
#include "LLMStoreExampleBlueprintLibrary.generated.h"

class ULLMStoreSubsystem;

/**
 * Small Blueprint helper layer for the sample project.
 *
 * The real integration point is still ULLMStoreSubsystem. These functions keep
 * the demo graph compact and make the route/request/response flow visible.
 *
 * Production Blueprints can call the subsystem directly. These helpers are here
 * so first-time users can build a graph without manually filling every struct
 * field during the first tutorial pass.
 */
UCLASS()
class TINYTOOLDEV_EXAMPLES_API ULLMStoreExampleBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns the engine-wide LLM Store subsystem, or null when the plugin is not active. */
	UFUNCTION(BlueprintPure, Category="LLMStore|Example", meta=(DisplayName="Get LLM Store"))
	static ULLMStoreSubsystem* GetLLMStoreSubsystem();

	/** Creates the built-in mock provider/model/default route and immediately verifies route resolution. */
	UFUNCTION(BlueprintCallable, Category="LLMStore|Example", meta=(DisplayName="Ensure Example Mock Setup"))
	static bool EnsureExampleMockSetup(FString& OutSummary);

	/** Adds all tutorial routes and prompt templates from this example project without overwriting existing entries. */
	UFUNCTION(BlueprintCallable, Category="LLMStore|Example", meta=(DisplayName="Ensure Example Sample Content"))
	static bool EnsureExampleSampleContent(FString& OutSummary);

	/** Builds a provider-neutral chat request with beginner-friendly defaults for TaskKind and SystemPrompt. */
	UFUNCTION(BlueprintPure, Category="LLMStore|Example", meta=(DisplayName="Make Example LLM Request"))
	static FLLMRequest MakeExampleRequest(
		const FString& TaskKind,
		const FString& Prompt,
		const FString& SystemPrompt,
		bool bUseCache,
		int32 MaxBudgetTokens);

	/** Resolves a route without sending a prompt. Useful for setup UI and troubleshooting graphs. */
	UFUNCTION(BlueprintCallable, Category="LLMStore|Example", meta=(DisplayName="Resolve Example Route"))
	static FLLMResult ResolveExampleRoute(const FString& TaskKind, FLLMResolvedRoute& OutResolved);

	/** Convenience node that builds a request and executes it through the store in one Blueprint call. */
	UFUNCTION(BlueprintCallable, Category="LLMStore|Example", meta=(DisplayName="Execute Example Prompt"))
	static void ExecuteExamplePrompt(
		const FString& TaskKind,
		const FString& Prompt,
		const FString& SystemPrompt,
		FLLMResponseDelegate OnComplete);

	/** Converts FLLMResponse into one readable string for Print String, logs, or simple UI labels. */
	UFUNCTION(BlueprintPure, Category="LLMStore|Example", meta=(DisplayName="Format LLM Response"))
	static FString FormatResponse(const FLLMResponse& Response);

	/** Converts the readiness snapshot into a readable summary for setup screens or tutorial output. */
	UFUNCTION(BlueprintPure, Category="LLMStore|Example", meta=(DisplayName="Format LLM Readiness"))
	static FString FormatReadiness(const FLLMReadiness& Readiness);
};
