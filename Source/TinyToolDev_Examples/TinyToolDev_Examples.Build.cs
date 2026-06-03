// Copyright (c) 2026 Tiny Tool Development. All rights reserved.

using UnrealBuildTool;

public class TinyToolDev_Examples : ModuleRules
{
	public TinyToolDev_Examples(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// The LLM Store sample uses the plugin in two ways:
		// - LLMStoreInterface provides the public request/response structs and delegates.
		// - LLMStore provides ULLMStoreSubsystem and helper setup functions used by the tutorial actors.
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"LLMStore",
			"LLMStoreInterface"
		});
	}
}
