using UnrealBuildTool;
using System.Collections.Generic;

public class TerraDyneOSS_ProtoEditorTarget : TargetRules
{
	public TerraDyneOSS_ProtoEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("TerraDyneOSS_Proto");
	}
}
