// Copyright (c) 2026 GregOrigin. All Rights Reserved.
using UnrealBuildTool;

public class TerraDyne : ModuleRules
{
	public TerraDyne(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bUseUnity = false;

		// Isolated prototype (TerraDyneOSS_Proto_270c311) only.
		// UBT 5.8 ModuleRules has no per-file exclude API; this definition
		// skips the two automation files that include stripped Fab example types.
		// Remaining Private/Tests/*.cpp still compile. Not a TerraDyne product flag.
		PrivateDefinitions.Add("TERRADYNE_OSS_ISOLATED_EXCLUDE_EXAMPLE_TESTS=1");


		// Core dependencies
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"Landscape",
			"RenderCore",
			"RHI",
			"GeometryCore",
			"UMG",
			"Projects"
		});

		// Plugin dependencies (Must be enabled in .uproject)
		PublicDependencyModuleNames.AddRange(new string[] {
			"GeometryFramework"
		});

		// Private dependencies
		PrivateDependencyModuleNames.AddRange(new string[] {
			"GeometryScriptingCore",
			"Slate",
            "SlateCore",
            "DeveloperSettings",
            "ApplicationCore",
            "NavigationSystem",
            "MeshDescription",
            "StaticMeshDescription"
		});

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("UnrealEd");
			PrivateDependencyModuleNames.Add("Foliage");
		}
	}
}
