// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"

#include "ASSGameplayAbility.generated.h"

/**
 * Base Gameplay Ability
 */
UCLASS()
class ABILITYSYSTEMSETUP_API UASSGameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:

    UASSGameplayAbility(const FObjectInitializer& inObjectInitializer);

protected:

    //  BEGIN UGameplayAbility interface
    virtual void EndAbility(
        const FGameplayAbilitySpecHandle inSpecHandle,
        const FGameplayAbilityActorInfo* inActorInfo,
        const FGameplayAbilityActivationInfo inActivationInfo,
        bool inShouldReplicateEndAbility,
        bool inWasCanceled) override final;
    //  END UGameplayAbility interface

protected:

    /**
     * The engine's EndAbility() is not safe to override out of the box. There are several checks that must be done before
     * using it as an event.
     * This version is called at a safe point for ability subclass logic to use as an event.
     */
    virtual void ASSEndAbility(
        const FGameplayAbilitySpecHandle inSpecHandle,
        const FGameplayAbilityActorInfo* inActorInfo,
        const FGameplayAbilityActivationInfo inActivationInfo,
        bool inShouldReplicateEndAbility,
        bool inWasCanceled);

public:

    /** Passive abilities are auto activated on given */
    bool bIsPassiveAbility = false;
};
