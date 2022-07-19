// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystemSetupModule.h"

#include "ISDeveloperSettings_InputSetup.h"



#define LOCTEXT_NAMESPACE "FAbilitySystemSetupModule"

void FAbilitySystemSetupModule::StartupModule()
{
	// Add this plugin's Input Actions
	UISDeveloperSettings_InputSetup* InputSetupDeveloperSettings = GetMutableDefault<UISDeveloperSettings_InputSetup>();
	if (IsValid(InputSetupDeveloperSettings))
	{
		InputSetupDeveloperSettings->AddRuntimeInputAction(ASSNativeGameplayTags::InputAction_ConfirmTarget, TSoftObjectPtr<const UInputAction>(FSoftObjectPath(TEXT("/AbilitySystemSetup/Input/IA_ConfirmTarget.IA_ConfirmTarget"))));
		InputSetupDeveloperSettings->AddRuntimeInputAction(ASSNativeGameplayTags::InputAction_CancelTarget, TSoftObjectPtr<const UInputAction>(FSoftObjectPath(TEXT("/AbilitySystemSetup/Input/IA_CancelTarget.IA_CancelTarget"))));
	}
}
void FAbilitySystemSetupModule::ShutdownModule()
{
	// Remove this plugin's Input Actions
	UISDeveloperSettings_InputSetup* InputSetupDeveloperSettings = GetMutableDefault<UISDeveloperSettings_InputSetup>();
	if (IsValid(InputSetupDeveloperSettings))
	{
		InputSetupDeveloperSettings->RemoveRuntimeInputAction(ASSNativeGameplayTags::InputAction_ConfirmTarget);
		InputSetupDeveloperSettings->RemoveRuntimeInputAction(ASSNativeGameplayTags::InputAction_CancelTarget);
	}
}

#undef LOCTEXT_NAMESPACE


IMPLEMENT_MODULE(FAbilitySystemSetupModule, AbilitySystemSetup)
