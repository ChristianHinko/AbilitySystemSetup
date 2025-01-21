// Fill out your copyright notice in the Description page of Project Settings.

#include "Examples/ASSActorComponent_AbilitySystemComponentExample.h"

UASSActorComponent_AbilitySystemComponentExample::UASSActorComponent_AbilitySystemComponentExample(const FObjectInitializer& inObjectInitializer)
    : Super(inObjectInitializer)
{
}

#if DO_CHECK
void UASSActorComponent_AbilitySystemComponentExample::BindToInputComponent(UInputComponent* inputComponent)
{
    AbilitySystemComponentExtention.BindToInputComponent_ReplaceSuper(inputComponent);
}

void UASSActorComponent_AbilitySystemComponentExample::BindAbilityActivationToInputComponent(UInputComponent* inputComponent, FGameplayAbilityInputBinds bindInfo)
{
    AbilitySystemComponentExtention.BindAbilityActivationToInputComponent_ReplaceSuper(inputComponent, bindInfo);
}

void UASSActorComponent_AbilitySystemComponentExample::AbilityLocalInputPressed(int32 inputID)
{
    AbilitySystemComponentExtention.AbilityLocalInputPressed_ReplaceSuper(inputID);
}

void UASSActorComponent_AbilitySystemComponentExample::AbilityLocalInputReleased(int32 inputID)
{
    AbilitySystemComponentExtention.AbilityLocalInputReleased_ReplaceSuper(inputID);
}
#endif // #if DO_CHECK

#if DO_CHECK || !NO_LOGGING
void UASSActorComponent_AbilitySystemComponentExample::OnGiveAbility(FGameplayAbilitySpec& abilitySpec)
{
    Super::OnGiveAbility(abilitySpec);

    AbilitySystemComponentExtention.OnGiveAbility_PostSuper(*this, abilitySpec);
}
#endif // #if DO_CHECK || !NO_LOGGING
