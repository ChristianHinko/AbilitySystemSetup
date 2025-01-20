// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <concepts>
#include <type_traits>
#include "ASSGameplayAbilityExtensionInterface.h"

class UGameplayAbility;
struct FGameplayAbilitySpecHandle;
struct FGameplayAbilityActorInfo;
struct FGameplayAbilityActivationInfo;

/**
 * @brief Struct to go on your base `UGameplayAbility` class as a data member to
 *        replace the engine's virtual functions with improved ones.
 * @note Your ability class must implement the `IASSGameplayAbilityExtensionInterface` class.
 * @see `IASSGameplayAbilityExtensionInterface`.
 */
struct ABILITYSYSTEMSETUP_API FASSGameplayAbilityExtensionStruct
{
public:

    FASSGameplayAbilityExtensionStruct();

public:

    bool IsEndAbilitySafe(
        const UGameplayAbility& inAbility,
        const FGameplayAbilitySpecHandle& inSpecHandle,
        const FGameplayAbilityActorInfo* inActorInfo,
        const FGameplayAbilityActivationInfo& inActivationInfo,
        bool inShouldReplicateEndAbility,
        bool inWasCanceled
        ) const;

    /**
     * @brief Function for safely dispatching the end-ability call to your
     *        ability's `IASSGameplayAbilityExtensionInterface` function.
     * @note You should mark your `UGameplayAbility::EndAbility()` override as `final` to
     *       enforce usage of the improved function.
     */
    template <std::derived_from<UGameplayAbility> TAbility>
    void EndAbility_ReplaceSuper(
        TAbility& inAbility,
        const FGameplayAbilitySpecHandle& inSpecHandle,
        const FGameplayAbilityActorInfo* inActorInfo,
        const FGameplayAbilityActivationInfo& inActivationInfo,
        bool inShouldReplicateEndAbility,
        bool inWasCanceled
        );

private:

    void EndAbility_ReplaceSuper(
        UGameplayAbility& inAbility,
        IASSGameplayAbilityExtensionInterface& inASSGameplayAbilityExtensionInterface,
        const FGameplayAbilitySpecHandle& inSpecHandle,
        const FGameplayAbilityActorInfo* inActorInfo,
        const FGameplayAbilityActivationInfo& inActivationInfo,
        bool inShouldReplicateEndAbility,
        bool inWasCanceled
        );

private:

    /**
     * @brief For tracking recursive calls to `EndAbility()`.
     */
    int8 CurrentEndAbilityRecursionDepth = -1;
};

template <std::derived_from<UGameplayAbility> TAbility>
void FASSGameplayAbilityExtensionStruct::EndAbility_ReplaceSuper(
    TAbility& inAbility,
    const FGameplayAbilitySpecHandle& inSpecHandle,
    const FGameplayAbilityActorInfo* inActorInfo,
    const FGameplayAbilityActivationInfo& inActivationInfo,
    bool inShouldReplicateEndAbility,
    bool inWasCanceled
    )
{
    static_assert(std::is_base_of_v<IASSGameplayAbilityExtensionInterface, TAbility>, "Ability class must inherit from our extension interface.");

    IASSGameplayAbilityExtensionInterface& extensionInterface = static_cast<IASSGameplayAbilityExtensionInterface&>(inAbility);

    EndAbility_ReplaceSuper(
        inAbility,
        extensionInterface,
        inSpecHandle,
        inActorInfo,
        inActivationInfo,
        inShouldReplicateEndAbility,
        inWasCanceled);
}
