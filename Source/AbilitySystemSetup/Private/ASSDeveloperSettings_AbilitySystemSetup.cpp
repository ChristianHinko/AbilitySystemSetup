// Fill out your copyright notice in the Description page of Project Settings.


#include "ASSDeveloperSettings_AbilitySystemSetup.h"



UASSDeveloperSettings_AbilitySystemSetup::UASSDeveloperSettings_AbilitySystemSetup(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilityInputIDEnumName = TEXT("EAbilityInputID");
	ConfirmTargetInputActionName = TEXT("ConfirmTarget");
	CancelTargetInputActionName = TEXT("CancelTarget");
}
