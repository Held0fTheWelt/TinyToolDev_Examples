// Copyright (c) 2026 Tiny Tool Development. All rights reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class TinyToolDev_ExamplesEditorTarget : TargetRules
{
	public TinyToolDev_ExamplesEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.Add("TinyToolDev_Examples");
	}
}
