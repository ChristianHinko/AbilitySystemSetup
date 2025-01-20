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
