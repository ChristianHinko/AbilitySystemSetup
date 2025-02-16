// Fill out your copyright notice in the Description page of Project Settings.

#include "Examples/ASSGameplayAbility_Example.h"

void UASSGameplayAbility_Example::EndAbility(
    const FGameplayAbilitySpecHandle specHandle,
    const FGameplayAbilityActorInfo* actorInfo,
    const FGameplayAbilityActivationInfo activationInfo,
    const bool shouldReplicateEndAbility,
    const bool wasCanceled)
{
    ASSGameplayAbilityExtensionStruct.EndAbility_ReplaceSuper(
        *this,
        specHandle,
        actorInfo,
        activationInfo,
        shouldReplicateEndAbility,
        wasCanceled);
}

void UASSGameplayAbility_Example::CallBaseEndAbility(
    const FGameplayAbilitySpecHandle& specHandle,
    const FGameplayAbilityActorInfo* actorInfo,
    const FGameplayAbilityActivationInfo& activationInfo,
    const bool shouldReplicateEndAbility,
    const bool wasCanceled)
{
    UGameplayAbility::EndAbility(specHandle, actorInfo, activationInfo, shouldReplicateEndAbility, wasCanceled);
}
