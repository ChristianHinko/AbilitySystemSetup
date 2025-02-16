// Fill out your copyright notice in the Description page of Project Settings.

#include "ASSGameplayAbilityExtensionInterface.h"

#include "Abilities/GameplayAbility.h"

void IASSGameplayAbilityExtensionInterface::ASSEndAbility(
    const FGameplayAbilitySpecHandle& specHandle,
    const FGameplayAbilityActorInfo& actorInfo,
    const FGameplayAbilityActivationInfo& activationInfo,
    const bool shouldReplicateEndAbility,
    const bool wasCanceled)
{
    CallBaseEndAbility(
        specHandle,
        &actorInfo,
        activationInfo,
        shouldReplicateEndAbility,
        wasCanceled);
}
