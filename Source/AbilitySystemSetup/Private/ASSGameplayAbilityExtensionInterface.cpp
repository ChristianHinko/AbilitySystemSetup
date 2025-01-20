// Fill out your copyright notice in the Description page of Project Settings.

#include "ASSGameplayAbilityExtensionInterface.h"

#include "Abilities/GameplayAbility.h"

namespace
{
    /**
     * @brief Call the base implementation of `EndAbility()` without dispatching
     *        the call to the final override.
     * @note [inaccessible-access-engine] This function externally accesses the
     *       function `UGameplayAbility::EndAbility` to be able to call it directly.
     */
    void CallBaseEndAbilityInternal(
        UGameplayAbility& inAbility,
        const FGameplayAbilitySpecHandle& inSpecHandle,
        const FGameplayAbilityActorInfo* inActorInfo,
        const FGameplayAbilityActivationInfo& inActivationInfo,
        const bool inShouldReplicateEndAbility,
        const bool inWasCanceled);
}

void IASSGameplayAbilityExtensionInterface::ASSEndAbility(
    const FGameplayAbilitySpecHandle& inSpecHandle,
    const FGameplayAbilityActorInfo& inActorInfo,
    const FGameplayAbilityActivationInfo& inActivationInfo,
    const bool inShouldReplicateEndAbility,
    const bool inWasCanceled)
{
    UGameplayAbility& ability = GetImplementor();

    CallBaseEndAbilityInternal(
        ability,
        inSpecHandle,
        &inActorInfo,
        inActivationInfo,
        inShouldReplicateEndAbility,
        inWasCanceled);
}

namespace
{
    void CallBaseEndAbilityInternal(
        UGameplayAbility& inAbility,
        const FGameplayAbilitySpecHandle& inSpecHandle,
        const FGameplayAbilityActorInfo* inActorInfo,
        const FGameplayAbilityActivationInfo& inActivationInfo,
        const bool inShouldReplicateEndAbility,
        const bool inWasCanceled)
    {
        class UASSGameplayAbility_Dummy : public UGameplayAbility
        {
        public:

            void CallBaseEndAbility(
                const FGameplayAbilitySpecHandle& inSpecHandle,
                const FGameplayAbilityActorInfo* inActorInfo,
                const FGameplayAbilityActivationInfo& inActivationInfo,
                const bool inShouldReplicateEndAbility,
                const bool inWasCanceled)
            {
                UGameplayAbility::EndAbility(
                    inSpecHandle,
                    inActorInfo,
                    inActivationInfo,
                    inShouldReplicateEndAbility,
                    inWasCanceled);
            }
        };

        // Note: We have to go through a non-static member function and use the `this` pointer
        // because, although the class has protected access to `UGameplayAbility::EndAbility`, it's
        // not allowed to call that function on behalf of other instances. That's how protected
        // member access works.
        static_cast<UASSGameplayAbility_Dummy&>(inAbility).CallBaseEndAbility(
            inSpecHandle,
            inActorInfo,
            inActivationInfo,
            inShouldReplicateEndAbility,
            inWasCanceled);
    }
}
