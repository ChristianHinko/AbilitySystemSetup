// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class AbilitySystemSetup : ModuleRules
{
	public AbilitySystemSetup(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PrivatePCHHeaderFile = "Private/AbilitySystemSetupPrivatePCH.h";

		PublicDependencyModuleNames.AddRange(new string[] { "Core" });
		PrivateDependencyModuleNames.AddRange(new string[] { "CoreUObject", "Engine" });

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"GameplayAbilities",
				"GameplayTasks",
				"GameplayTags",
				"NetCore", // for push model
				"GameCore",
				"EnhancedInput",
				"InputSetup"
			}
		);
	}
}
