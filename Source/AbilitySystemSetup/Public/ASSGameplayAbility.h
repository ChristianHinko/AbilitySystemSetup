// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ASSGameplayAbilityExtensionInterface.h"
#include "ASSGameplayAbilityExtensionStruct.h"

#include "ASSGameplayAbility.generated.h"

/**
 *
 */
UCLASS()
class ABILITYSYSTEMSETUP_API UASSGameplayAbility : public UGameplayAbility, public IASSGameplayAbilityExtensionInterface
{
    GENERATED_BODY()

public:

    UASSGameplayAbility(const FObjectInitializer& inObjectInitializer);

protected:

    FASSGameplayAbilityExtensionStruct ASSGameplayAbilityExtensionStruct;

protected:

    // ~ UGameplayAbility overrides.
    virtual void EndAbility(
        const FGameplayAbilitySpecHandle inSpecHandle,
        const FGameplayAbilityActorInfo* inActorInfo,
        const FGameplayAbilityActivationInfo inActivationInfo,
        bool inShouldReplicateEndAbility,
        bool inWasCanceled) override final;
    // ~ UGameplayAbility overrides.

public:

    // ~ IASSGameplayAbilityExtensionInterface overrides.
    virtual UGameplayAbility& GetImplementor() override final;
    // ~ IASSGameplayAbilityExtensionInterface overrides.
};
