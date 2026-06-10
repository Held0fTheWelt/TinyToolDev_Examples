/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

using UnrealBuildTool;

public class InternalIndexServiceUMCPBridge : ModuleRules
{
	public InternalIndexServiceUMCPBridge(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		CppStandard = CppStandardVersion.Cpp20;

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Json",
			"InternalIndexServiceInterface",
			"InternalIndexService",
			"UnifiedMcpServerInterface"
		});
	}
}
