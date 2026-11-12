// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

using UnrealBuildTool;

public class PWGasCore : ModuleRules
{
	public PWGasCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]
			{
				// ... add public include paths required here ...
			}
		);


		PrivateIncludePaths.AddRange(
			new string[]
			{
				// ... add other private include paths required here ...
			}
		);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"GameplayAbilities",
				"GameplayTags",
				"DeveloperSettings",
				"GameplayTasks",
				"PWSharedTypes",
				"PWContracts",
				// ... add other public dependencies that you statically link with here ...
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"GameplayAbilities",
				"GameplayTags",
				"DeveloperSettings",
				"PWContracts", 
				"Niagara"
				// ... add private dependencies that you statically link with here ...	
			}
		);


		PrivateDependencyModuleNames.Add("PWTeams");

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);
	}
}