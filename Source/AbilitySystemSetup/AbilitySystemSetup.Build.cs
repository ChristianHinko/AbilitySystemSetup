// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class AbilitySystemSetup : ModuleRules
{
	public AbilitySystemSetup(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core" });
		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"CoreUObject",
				"Engine",
				"InputCore",
				"GameplayAbilities",
				"GameplayTags",
				"GameplayTasks",
				"NetCore", // for push model
				"HelperLibraries",
				"DeveloperSettings"
			}
		);
	}
}
