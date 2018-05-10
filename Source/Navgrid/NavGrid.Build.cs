using UnrealBuildTool;
using System.IO;
 
public class NavGrid : ModuleRules
{
    public NavGrid(ReadOnlyTargetRules TargetRules) : base(TargetRules) {
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "AIModule" });

        if (TargetRules.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[] { "UnrealED" });
        }

        PublicIncludePaths.AddRange(
            new string[] {
                "NavGrid/Public",
                "NavGrid/Classes",
        // ... add public include paths required here ...
    }
    );


        PrivateIncludePaths.AddRange(
            new string[] {
                "NavGrid/Private",
				// ... add other private include paths required here ...
			}
            );
    }
}