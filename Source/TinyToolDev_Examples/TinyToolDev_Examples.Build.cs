// Copyright (c) 2026 Tiny Tool Development. All rights reserved.

using UnrealBuildTool;

public class TinyToolDev_Examples : ModuleRules
{
	public TinyToolDev_Examples(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		// Keep the base module deliberately small. Example branches add their
		// plugin dependencies and UI/runtime modules only where they are needed.
	}
}
