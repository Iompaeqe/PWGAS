// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
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
				"GameplayMessageRuntime",
				"Niagara"
				// ... add private dependencies that you statically link with here ...	
			}
		);


		string TeamsPluginPath = Path.Combine(ModuleDirectory, "../../../../PWTeams/PWTeams.uplugin");
		if (File.Exists(TeamsPluginPath))
		{
			PublicDefinitions.Add("WITH_PWTEAMS=1");
			PrivateDependencyModuleNames.Add("PWTeams");
		}
		else
		{
			PublicDefinitions.Add("WITH_PWTEAMS=0");
		}

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);
	}
}