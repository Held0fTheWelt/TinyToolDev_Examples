// Copyright (c) 2026 Tiny Tool Development. All rights reserved.

#include "LLMStoreExampleBlueprintLibrary.h"

#include "Engine/Engine.h"
#include "LLMStarterSetup.h"
#include "LLMStoreConfig.h"
#include "LLMStoreSubsystem.h"

namespace
{
	// The sample route created by ELLMStarterKind::Mock. Keeping this constant
	// here makes the Blueprint nodes forgiving when a beginner leaves TaskKind empty.
	const FString DefaultTaskKind = TEXT("default");
	const FString DefaultSystemPrompt = TEXT("You are a concise Unreal Engine assistant. Answer in one short paragraph.");

	// Blueprint users see enum values as pins, but tutorial log output is easier
	// to read when the enum value is converted back to its short name.
	FString ResultCodeToText(ELLMResultCode Code)
	{
		if (const UEnum* Enum = StaticEnum<ELLMResultCode>())
		{
			return Enum->GetNameStringByValue(static_cast<int64>(Code));
		}
		return TEXT("Unknown");
	}

	FString NormalizeTaskKind(const FString& TaskKind)
	{
		// Empty strings are common while experimenting with Blueprint pins.
		// Falling back to "default" keeps the tutorial running.
		return TaskKind.TrimStartAndEnd().IsEmpty() ? DefaultTaskKind : TaskKind;
	}

	FString NormalizeSystemPrompt(const FString& SystemPrompt)
	{
		// A system prompt is optional, but a short default gives predictable
		// behavior when the sample is switched from mock to a real provider.
		return SystemPrompt.TrimStartAndEnd().IsEmpty() ? DefaultSystemPrompt : SystemPrompt;
	}

	FLLMResponse MakeUnavailableResponse(const FString& Message)
	{
		// This mirrors the shape of a provider failure so Blueprint completion
		// handlers can treat setup failures and provider failures the same way.
		FLLMResponse Response;
		Response.Result.bSuccess = false;
		Response.Result.ErrorCode = ELLMResultCode::MissingProvider;
		Response.Result.Message = Message;
		return Response;
	}

	FLLMRoutePolicy MakeLocalMockPolicy(const FString& TaskKind, int32 MaxContextTokens)
	{
		FLLMRoutePolicy Policy;
		Policy.TaskKind = TaskKind;
		Policy.bAllowCloud = false;
		Policy.bLocalOnly = true;
		Policy.MaxContextTokens = MaxContextTokens;
		Policy.MaxEstimatedCost = 0.0f;
		Policy.AllowedProviderTypes.Add(TEXT("mock"));
		return Policy;
	}

	bool AddPromptTemplateIfMissing(
		FLLMStoreConfig& Config,
		const FString& Id,
		const FString& Description,
		const FString& Template)
	{
		const bool bExists = Config.PromptTemplates.ContainsByPredicate(
			[&Id](const FLLMPromptTemplate& Existing)
			{
				return Existing.Id == Id;
			});
		if (bExists)
		{
			return false;
		}

		FLLMPromptTemplate PromptTemplate;
		PromptTemplate.Id = Id;
		PromptTemplate.Description = Description;
		PromptTemplate.Template = Template;
		PromptTemplate.Variables.Add(TEXT("prompt"));
		Config.PromptTemplates.Add(PromptTemplate);
		return true;
	}

	bool AddMockRouteIfMissing(
		FLLMStoreConfig& Config,
		const FString& TaskKind,
		const FString& WorkflowScope,
		const FString& PromptTemplateId,
		int32 MaxContextTokens)
	{
		const bool bExists = Config.Routes.ContainsByPredicate(
			[&TaskKind](const FLLMTaskRoute& Existing)
			{
				return Existing.TaskKind == TaskKind;
			});
		if (bExists)
		{
			return false;
		}

		FLLMTaskRoute Route;
		Route.TaskKind = TaskKind;
		Route.PreferredModelId = TEXT("quickstart_mock_model");
		Route.MockModelId = TEXT("quickstart_mock_model");
		Route.bUseMock = true;
		Route.WorkflowScope = WorkflowScope;
		Route.PromptTemplateId = PromptTemplateId;
		Route.Policy = MakeLocalMockPolicy(TaskKind, MaxContextTokens);
		Route.bEnabled = true;
		Config.Routes.Add(Route);
		return true;
	}
}

ULLMStoreSubsystem* ULLMStoreExampleBlueprintLibrary::GetLLMStoreSubsystem()
{
	// LLM Store is engine-wide. Tools, actors, widgets, and editor utilities all
	// ask the engine for the same subsystem rather than owning provider state.
	return GEngine ? GEngine->GetEngineSubsystem<ULLMStoreSubsystem>() : nullptr;
}

bool ULLMStoreExampleBlueprintLibrary::EnsureExampleMockSetup(FString& OutSummary)
{
	ULLMStoreSubsystem* Store = GetLLMStoreSubsystem();
	if (!Store)
	{
		OutSummary = TEXT("LLM Store subsystem is not available. Check that the LLMStore plugin is enabled.");
		return false;
	}

	Store->ApplyStarterSetup(ELLMStarterKind::Mock, OutSummary);

	// ApplyStarterSetup is idempotent. The explicit resolve below gives the
	// tutorial a clear success/failure message after the config change.
	FLLMResolvedRoute Resolved;
	const FLLMResult Result = Store->ResolveRoute(DefaultTaskKind, Resolved);
	if (!Result.bSuccess)
	{
		OutSummary += FString::Printf(TEXT(" Route check failed: %s"), *Result.Message);
		return false;
	}

	OutSummary += FString::Printf(
		TEXT(" Route '%s' resolves to provider '%s' and model '%s'."),
		*DefaultTaskKind,
		*Resolved.Provider.Id,
		*Resolved.Model.Id);
	return true;
}

bool ULLMStoreExampleBlueprintLibrary::EnsureExampleSampleContent(FString& OutSummary)
{
	ULLMStoreSubsystem* Store = GetLLMStoreSubsystem();
	if (!Store)
	{
		OutSummary = TEXT("LLM Store subsystem is not available. Check that the LLMStore plugin is enabled.");
		return false;
	}

	FString StarterSummary;
	Store->ApplyStarterSetup(ELLMStarterKind::Mock, StarterSummary);

	FLLMStoreConfig Config = Store->GetConfig();
	int32 AddedRoutes = 0;
	int32 AddedTemplates = 0;

	AddedTemplates += AddPromptTemplateIfMissing(
		Config,
		TEXT("example.asset.explain"),
		TEXT("Explains an Unreal asset or content item for artists and designers."),
		TEXT("You are helping an Unreal Engine content team understand an asset.\n\nAsset notes:\n{prompt}\n\nReturn: purpose, likely owner, risk, and one useful next action."))
		? 1 : 0;
	AddedTemplates += AddPromptTemplateIfMissing(
		Config,
		TEXT("example.docs.summarize"),
		TEXT("Summarizes documentation into decisions and next steps."),
		TEXT("Summarize the following Unreal project documentation for a busy technical lead.\n\nDocumentation:\n{prompt}\n\nReturn: summary, important decisions, missing information, and follow-up tasks."))
		? 1 : 0;
	AddedTemplates += AddPromptTemplateIfMissing(
		Config,
		TEXT("example.npc.dialogue"),
		TEXT("Drafts a short in-character NPC line from narrative context."),
		TEXT("You are drafting placeholder NPC dialogue for an Unreal game prototype.\n\nNarrative context:\n{prompt}\n\nReturn three short dialogue options with different emotional tones."))
		? 1 : 0;
	AddedTemplates += AddPromptTemplateIfMissing(
		Config,
		TEXT("example.code.review"),
		TEXT("Reviews a small C++ or Blueprint-adjacent code snippet."),
		TEXT("Review this Unreal Engine implementation note or snippet.\n\nContent:\n{prompt}\n\nReturn: likely issue, severity, safer pattern, and test idea."))
		? 1 : 0;
	AddedTemplates += AddPromptTemplateIfMissing(
		Config,
		TEXT("example.qa.testplan"),
		TEXT("Turns a feature note into a compact QA checklist."),
		TEXT("Create a practical QA checklist for this Unreal feature.\n\nFeature note:\n{prompt}\n\nReturn: smoke tests, edge cases, regression risks, and automation ideas."))
		? 1 : 0;

	AddedRoutes += AddMockRouteIfMissing(Config, TEXT("asset.explain"), TEXT("Content"), TEXT("example.asset.explain"), 2048) ? 1 : 0;
	AddedRoutes += AddMockRouteIfMissing(Config, TEXT("docs.summarize"), TEXT("Documentation"), TEXT("example.docs.summarize"), 4096) ? 1 : 0;
	AddedRoutes += AddMockRouteIfMissing(Config, TEXT("npc.dialogue"), TEXT("Narrative"), TEXT("example.npc.dialogue"), 2048) ? 1 : 0;
	AddedRoutes += AddMockRouteIfMissing(Config, TEXT("code.review"), TEXT("Engineering"), TEXT("example.code.review"), 4096) ? 1 : 0;
	AddedRoutes += AddMockRouteIfMissing(Config, TEXT("qa.testplan"), TEXT("Quality"), TEXT("example.qa.testplan"), 3072) ? 1 : 0;

	if (AddedRoutes > 0 || AddedTemplates > 0)
	{
		Store->SetConfig(Config);
	}

	OutSummary = FString::Printf(
		TEXT("%s Sample content ready: %d route(s) and %d prompt template(s) added."),
		*StarterSummary,
		AddedRoutes,
		AddedTemplates);
	return true;
}

FLLMRequest ULLMStoreExampleBlueprintLibrary::MakeExampleRequest(
	const FString& TaskKind,
	const FString& Prompt,
	const FString& SystemPrompt,
	bool bUseCache,
	int32 MaxBudgetTokens)
{
	FLLMRequest Request;
	// Chat is the common first example. The same route pattern also supports
	// embeddings and rerank operations through the plugin API.
	Request.Operation = ELLMRequestOperation::Chat;
	Request.TaskKind = NormalizeTaskKind(TaskKind);
	Request.Prompt = Prompt.TrimStartAndEnd().IsEmpty()
		? TEXT("Explain in one sentence what the LLM Store does in this Unreal project.")
		: Prompt;
	Request.SystemPrompt = NormalizeSystemPrompt(SystemPrompt);
	Request.bUseCache = bUseCache;
	Request.CacheTtlSeconds = 300;
	// Negative budgets are not meaningful. Clamp them so beginners can connect
	// any integer pin without accidentally creating invalid request data.
	Request.MaxBudgetTokens = FMath::Max(0, MaxBudgetTokens);
	return Request;
}

FLLMResult ULLMStoreExampleBlueprintLibrary::ResolveExampleRoute(
	const FString& TaskKind,
	FLLMResolvedRoute& OutResolved)
{
	if (ULLMStoreSubsystem* Store = GetLLMStoreSubsystem())
	{
		// ResolveRoute is a cheap setup check: it verifies route/model/provider
		// references and returns the selected provider/model without sending HTTP.
		return Store->ResolveRoute(NormalizeTaskKind(TaskKind), OutResolved);
	}

	FLLMResult Result;
	Result.bSuccess = false;
	Result.ErrorCode = ELLMResultCode::MissingProvider;
	Result.Message = TEXT("LLM Store subsystem is not available.");
	return Result;
}

void ULLMStoreExampleBlueprintLibrary::ExecuteExamplePrompt(
	const FString& TaskKind,
	const FString& Prompt,
	const FString& SystemPrompt,
	FLLMResponseDelegate OnComplete)
{
	ULLMStoreSubsystem* Store = GetLLMStoreSubsystem();
	if (!Store)
	{
		OnComplete.ExecuteIfBound(MakeUnavailableResponse(TEXT("LLM Store subsystem is not available.")));
		return;
	}

	const FLLMRequest Request = MakeExampleRequest(TaskKind, Prompt, SystemPrompt, true, 512);
	// This helper intentionally uses the same ExecuteRoute function shown in the
	// C++ tutorial. The only difference is that Blueprint receives the delegate.
	Store->ExecuteRoute(Request, MoveTemp(OnComplete));
}

FString ULLMStoreExampleBlueprintLibrary::FormatResponse(const FLLMResponse& Response)
{
	// Keep failures compact: the enum-like code is useful for logic, while the
	// message usually tells the user what setup step failed.
	if (!Response.Result.bSuccess)
	{
		return FString::Printf(
			TEXT("Failed (%s): %s"),
			*ResultCodeToText(Response.Result.ErrorCode),
			*Response.Result.Message);
	}

	// Success output includes the fields buyers usually want to verify first:
	// visible content, selected model, token usage, estimated cost, and cache hit.
	return FString::Printf(
		TEXT("%s\n\nModel: %s | Tokens: %d | Cost: %.6f %s%s"),
		*Response.Content,
		*Response.ModelId,
		Response.Usage.TotalTokens,
		Response.Usage.EstimatedCost,
		*Response.Usage.Currency,
		Response.bFromCache ? TEXT(" | cache") : TEXT(""));
}

FString ULLMStoreExampleBlueprintLibrary::FormatReadiness(const FLLMReadiness& Readiness)
{
	// Readiness is designed for setup screens. Severity 0 is OK, 1 is warning,
	// and 2 is blocked. Blockers are the fastest path to troubleshooting.
	TArray<FString> Lines;
	Lines.Add(FString::Printf(TEXT("%s (severity %d)"), *Readiness.Headline, Readiness.Severity));

	if (Readiness.Blockers.Num() > 0)
	{
		Lines.Add(FString::Printf(TEXT("Blockers: %s"), *FString::Join(Readiness.Blockers, TEXT("; "))));
	}

	if (Readiness.Inventory.Num() > 0)
	{
		Lines.Add(FString::Printf(TEXT("Inventory: %s"), *FString::Join(Readiness.Inventory, TEXT("; "))));
	}

	return FString::Join(Lines, TEXT("\n"));
}
