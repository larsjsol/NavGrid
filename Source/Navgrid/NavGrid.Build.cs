using UnrealBuildTool;
using System.IO;

public class NavGrid : ModuleRules
{
    public NavGrid(ReadOnlyTargetRules TargetRules) : base(TargetRules) {
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "AIModule" });
        PrivatePCHHeaderFile = "Private/NavGridPrivatePCH.h";

        if (TargetRules.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[] { "UnrealED" });
        }

        PublicIncludePaths.AddRange(
            new string[] {
                Path.Combine(ModuleDirectory, "Public"),
                Path.Combine(ModuleDirectory, "Classes"),
        // ... add public include paths required here ...
    }
    );


        PrivateIncludePaths.AddRange(
            new string[] {
                 Path.Combine(ModuleDirectory, "Private"),
				// ... add other private include paths required here ...
			}
            );
    }
}