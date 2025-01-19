// Fill out your copyright notice in the Description page of Project Settings.

#include "ASSGameplayAbilityExtensionStruct.h"

#include "Abilities/GameplayAbility.h"
#include "GCUtils_Log.h"
#include "AbilitySystemComponent.h"
#include "ASSUtils.h"
#include "ASSGameplayAbilityExtensionInterface.h"
#include "ASSGameplayAbility.h"

DEFINE_LOG_CATEGORY_STATIC(LogASSGameplayAbilityExtensionStruct, Log, All);

namespace
{
    /**
     * @brief [inaccessible-access-engine] Call `UGameplayAbility::IsEndAbilityValid`.
     */
    template
        <
        bool (UGameplayAbility::* pointerToMember)(const FGameplayAbilitySpecHandle, const FGameplayAbilityActorInfo*) const =
            &UGameplayAbility::IsEndAbilityValid
        >
    bool CallIsEndAbilityValidInternal(
        const UGameplayAbility& inGameplayAbility,
        const FGameplayAbilitySpecHandle& inSpecHandle,
        const FGameplayAbilityActorInfo* inActorInfo)
    {
        return (inGameplayAbility.*pointerToMember)(inSpecHandle, inActorInfo);
    }
}

FASSGameplayAbilityExtensionStruct::FASSGameplayAbilityExtensionStruct()
{
}

bool FASSGameplayAbilityExtensionStruct::IsEndAbilitySafe(
    const UGameplayAbility& inAbility,
    const FGameplayAbilitySpecHandle& inSpecHandle,
    const FGameplayAbilityActorInfo* inActorInfo,
    const FGameplayAbilityActivationInfo& inActivationInfo,
    bool inShouldReplicateEndAbility,
    bool inWasCanceled
    ) const
{
    // [duplicate-code-engine] Say it's not safe to perform end ability logic if this end ability is not valid.
    if (!CallIsEndAbilityValidInternal(inAbility, inSpecHandle, inActorInfo))
    {
        return false;
    }

    // [duplicate-code-engine] Say it's not safe to perform end ability logic if this ability's scope lock count is greater than zero.
    if (ASSUtils::GetGameplayAbilityScopeLockCount(inAbility) > 0)
    {
        return false;
    }

    // Say it's not safe to perform end ability logic if this is just a recursive call to it. Note: This is similar to
    // the engine's `UGameplayAbility::bIsAbilityEnding` bool but more generic and usable for subclasses.
    if (CurrentEndAbilityRecursionDepth > 0)
    {
        // End ability logic has already occurred. We don't want to run it more than once.
        return false;
    }

    return true;
}

void FASSGameplayAbilityExtensionStruct::EndAbility_ReplaceSuper(
    UGameplayAbility& inAbility,
    IASSGameplayAbilityExtensionInterface& inASSGameplayAbilityExtensionInterface,
    const FGameplayAbilitySpecHandle& inSpecHandle,
    const FGameplayAbilityActorInfo* inActorInfo,
    const FGameplayAbilityActivationInfo& inActivationInfo,
    bool inShouldReplicateEndAbility,
    bool inWasCanceled
    )
{
    TGuardValue scopedCurrentEndAbilityRecursionDepthValueGuard(
        CurrentEndAbilityRecursionDepth,
        CurrentEndAbilityRecursionDepth + 1);

    if (!IsEndAbilitySafe(inAbility, inSpecHandle, inActorInfo, inActivationInfo, inShouldReplicateEndAbility, inWasCanceled))
    {
        return;
    }

    // It is safe to execute end ability logic. Dispatch call our interface's end ability function.

    inASSGameplayAbilityExtensionInterface.ASSEndAbility(
        inSpecHandle,
        *inActorInfo,
        inActivationInfo,
        inShouldReplicateEndAbility,
        inWasCanceled);
}
