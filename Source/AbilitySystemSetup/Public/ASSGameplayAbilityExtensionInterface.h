// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct FGameplayAbilitySpecHandle;
struct FGameplayAbilityActorInfo;
struct FGameplayAbilityActivationInfo;
class UGameplayAbility;

/**
 * @brief Interface that goes on your base `UGameplayAbility` class to replace the
 *        engine's virtual functions with improved ones.
 * @see `FASSGameplayAbilityExtensionStruct`.
 */
class ABILITYSYSTEMSETUP_API IASSGameplayAbilityExtensionInterface
{
public:

    /**
     * @brief Improved replacement for `UGameplayAbility::EndAbility()`. Will
     *        not be called more than once per ability end and includes all
     *        other safety checks that the base gameplay ability class performs.
     */
    virtual void ASSEndAbility(
        const FGameplayAbilitySpecHandle& inSpecHandle,
        const FGameplayAbilityActorInfo& inActorInfo,
        const FGameplayAbilityActivationInfo& inActivationInfo,
        const bool inShouldReplicateEndAbility,
        const bool inWasCanceled);

    /**
     * @brief Get the gameplay ability that is implementing this interface.
     * @note The immediate subclass of this interface should mark their override of
     *       this function as `final`, as there are no other ability instances to
     *       return per subclass.
     */
    virtual UGameplayAbility& GetImplementor() = 0;
};
