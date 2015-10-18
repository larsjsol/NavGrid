using UnrealBuildTool;
using System.IO;
 
public class NavGrid : ModuleRules
{
    public NavGrid(TargetInfo Target) {
        PrivateIncludePaths.AddRange(new string[] { "NavGrid/Private" });
        PublicIncludePaths.AddRange(new string[] { "NavGrid/Public" });
        DynamicallyLoadedModuleNames.AddRange(new string[] { "NavGrid" });

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
        PrivateDependencyModuleNames.AddRange(new string[] { "UnrealED" });
    }
}