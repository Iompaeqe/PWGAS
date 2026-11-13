// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

using UnrealBuildTool;

public class PWGASCoreTests : ModuleRules
{
    public PWGASCoreTests(ReadOnlyTargetRules Target) : base(Target)
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
                "GameplayTags",
                "GameplayAbilities",
                "PWGasCore",
                "PWSharedTypes",
                "PWContracts",
                "PWAbility",
                "PWTeams",
                "PWAttributes",
                "AIModule",
            }
        );
    }
}
