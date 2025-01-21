// Fill out your copyright notice in the Description page of Project Settings.

#include "Examples/ASSGameplayAbility_Example.h"

void UASSGameplayAbility_Example::EndAbility(
    const FGameplayAbilitySpecHandle inSpecHandle,
    const FGameplayAbilityActorInfo* inActorInfo,
    const FGameplayAbilityActivationInfo inActivationInfo,
    bool inShouldReplicateEndAbility,
    bool inWasCanceled)
{
    ASSGameplayAbilityExtensionStruct.EndAbility_ReplaceSuper(
        *this,
        inSpecHandle,
        inActorInfo,
        inActivationInfo,
        inShouldReplicateEndAbility,
        inWasCanceled);
}

UGameplayAbility& UASSGameplayAbility_Example::GetImplementor()
{
    return *this;
}
