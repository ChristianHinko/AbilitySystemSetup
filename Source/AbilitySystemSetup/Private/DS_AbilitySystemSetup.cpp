// Fill out your copyright notice in the Description page of Project Settings.


#include "DS_AbilitySystemSetup.h"



UDS_AbilitySystemSetup::UDS_AbilitySystemSetup(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilityInputIDEnumName = TEXT("EAbilityInputID");
	ConfirmTargetInputActionName = TEXT("ConfirmTarget");
	CancelTargetInputActionName = TEXT("CancelTarget");
}
