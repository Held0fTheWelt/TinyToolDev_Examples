// Copyright (c) 2026 Tiny Tool Development. All rights reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class TinyToolDev_ExamplesTarget : TargetRules
{
	public TinyToolDev_ExamplesTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.Add("TinyToolDev_Examples");
	}
}
