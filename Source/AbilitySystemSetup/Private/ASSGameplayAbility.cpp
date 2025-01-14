// Fill out your copyright notice in the Description page of Project Settings.

#include "ASSGameplayAbility.h"

#include "GCUtils_Log.h"
#include "AbilitySystemComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogASSGameplayAbility, Log, All);

UASSGameplayAbility::UASSGameplayAbility(const FObjectInitializer& inObjectInitializer)
    : Super(inObjectInitializer)
{
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    bServerRespectsRemoteAbilityCancellation = false;
    NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnlyTermination;
}

void UASSGameplayAbility::EndAbility(
    const FGameplayAbilitySpecHandle inSpecHandle,
    const FGameplayAbilityActorInfo* inActorInfo,
    const FGameplayAbilityActivationInfo inActivationInfo,
    bool inShouldReplicateEndAbility,
    bool inWasCanceled)
{
    // Call our `ASSEndAbility()` at a safe point.

    if (!IsEndAbilityValid(inSpecHandle, inActorInfo))
    {
        Super::EndAbility(inSpecHandle, inActorInfo, inActivationInfo, inShouldReplicateEndAbility, inWasCanceled);
        return;
    }

    if (ScopeLockCount > 0)
    {
        Super::EndAbility(inSpecHandle, inActorInfo, inActivationInfo, inShouldReplicateEndAbility, inWasCanceled);
        return;
    }

    // TODO: Add a return-early check similar to `bIsAbilityEnding`. Also, document the small duplication of engine
    // code. Also, try to eliminate our custom `ASSEndAbility()` function and maybe replace it with helper functions or something.

    // This is the safe point to do end ability logic.
    ASSEndAbility(inSpecHandle, inActorInfo, inActivationInfo, inShouldReplicateEndAbility, inWasCanceled);
}

void UASSGameplayAbility::ASSEndAbility(
    const FGameplayAbilitySpecHandle inSpecHandle,
    const FGameplayAbilityActorInfo* inActorInfo,
    const FGameplayAbilityActivationInfo inActivationInfo,
    bool inShouldReplicateEndAbility,
    bool inWasCanceled)
{
    // End the ability. Note: Skip `UASSGameplayAbility::EndAbility()` as that would just continue calling us again until
    // we stack overflow.
    Super::EndAbility(inSpecHandle, inActorInfo, inActivationInfo, inShouldReplicateEndAbility, inWasCanceled);
}
