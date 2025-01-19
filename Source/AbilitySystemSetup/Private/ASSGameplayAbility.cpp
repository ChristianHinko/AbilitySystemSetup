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
    ASSGameplayAbilityExtensionStruct.EndAbility_ReplaceSuper(
        *this,
        inSpecHandle,
        inActorInfo,
        inActivationInfo,
        inShouldReplicateEndAbility,
        inWasCanceled);
}

UGameplayAbility& UASSGameplayAbility::GetImplementor()
{
    return *this;
}
