// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

using UnrealBuildTool;

public class PWGASCoreEditor : ModuleRules
{
    public PWGASCoreEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "UnrealEd",
                "PropertyEditor",
                "GameplayTags",
                "PWGasCore",
                "GameplayTagsEditor", 
                "AssetRegistry",
                "AssetTools",
                "GameplayAbilities",
                "DeveloperSettings",
                "InputCore",  
            }
        );
    }
}