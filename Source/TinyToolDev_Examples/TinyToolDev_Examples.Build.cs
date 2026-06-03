// Copyright (c) 2026 Tiny Tool Development. All rights reserved.

using UnrealBuildTool;

public class TinyToolDev_Examples : ModuleRules
{
	public TinyToolDev_Examples(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"SmartContentDietInterface"
		});
	}
}
