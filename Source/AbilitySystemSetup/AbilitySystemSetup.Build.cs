// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class AbilitySystemSetup : ModuleRules
{
	public AbilitySystemSetup(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"InputCore",
				"GameplayAbilities",
				"HelperLibraries",
				"GameplayTags",
				"GameplayTasks",
				"DeveloperSettings",
				"NetCore" // for push model
			}
		);
	}
}
