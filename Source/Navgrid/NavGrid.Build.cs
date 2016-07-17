using UnrealBuildTool;
using System.IO;
 
public class NavGrid : ModuleRules
{
    public NavGrid(TargetInfo Target) {
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
        PrivateDependencyModuleNames.AddRange(new string[] { "UnrealED" });
    }
}