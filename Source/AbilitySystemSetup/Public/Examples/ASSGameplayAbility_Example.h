// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ASSGameplayAbilityExtensionInterface.h"
#include "ASSGameplayAbilityExtensionStruct.h"

#include "ASSGameplayAbility_Example.generated.h"

/**
 * @brief Example base class for implementing our extended ability functionality. Feel
 *        free to subclass if lazy.
 */
UCLASS()
class ABILITYSYSTEMSETUP_API UASSGameplayAbility_Example : public UGameplayAbility, public IASSGameplayAbilityExtensionInterface
{
    GENERATED_BODY()

protected:

    FASSGameplayAbilityExtensionStruct ASSGameplayAbilityExtensionStruct;

protected:

    // ~ UGameplayAbility overrides.
    virtual void EndAbility(
        const FGameplayAbilitySpecHandle specHandle,
        const FGameplayAbilityActorInfo* actorInfo,
        const FGameplayAbilityActivationInfo activationInfo,
        bool shouldReplicateEndAbility,
        bool wasCanceled) override final;
    // ~ UGameplayAbility overrides.

protected:

    // ~ IASSGameplayAbilityExtensionInterface overrides.
    virtual void CallBaseEndAbility(
        const FGameplayAbilitySpecHandle& specHandle,
        const FGameplayAbilityActorInfo* actorInfo,
        const FGameplayAbilityActivationInfo& activationInfo,
        const bool shouldReplicateEndAbility,
        const bool wasCanceled) override final;
    // ~ IASSGameplayAbilityExtensionInterface overrides.
};
