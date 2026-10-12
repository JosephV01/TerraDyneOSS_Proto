using UnrealBuildTool;

public class TerraDyneOSS_Proto : ModuleRules
{
	public TerraDyneOSS_Proto(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"UMG",
			"Slate",
			"SlateCore",
			"GeometryCore",
			"GeometryFramework",
			"RHI",
			"RenderCore",
			"TerraDyne"
		});
	}
}
