// Fill out your copyright notice in the Description page of Project Settings.

#include "ASSGameplayAbility.h"

#include "ISNativeGameplayTags.h"
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

void UASSGameplayAbility::OnAvatarSet(
    const FGameplayAbilityActorInfo* inActorInfo,
    const FGameplayAbilitySpec& inSpec)
{
    Super::OnAvatarSet(inActorInfo, inSpec);

#if !NO_LOGGING || DO_ENSURE
    if (GetAssetTags().IsEmpty())
    {
        GC_LOG_STR_UOBJECT(
            this,
            LogASSGameplayAbility,
            Warning,
            TEXT("Ability implementor forgot to assign an Ability Tag to this ability. We try to enforce activating abilities by tag for organization reasons"));
        ensure(0);
    }

    if (GetAssetTags().HasTag(ISNativeGameplayTags::InputAction) == false)
    {
        GC_LOG_STR_UOBJECT(
            this,
            LogASSGameplayAbility,
            Warning,
            TEXT("Ability implementor forgot to assign an input action Ability Tag to this ability. We enforce this so that a given an input action can identify any abilities it activates. If the ability isn't intended to be activated by input you can suppress this with InputAction.None tag."));
        ensure(0);
    }
#endif // #if !NO_LOGGING || DO_ENSURE
}

void UASSGameplayAbility::OnGiveAbility(
    const FGameplayAbilityActorInfo* inActorInfo,
    const FGameplayAbilitySpec& inSpec)
{
    Super::OnGiveAbility(inActorInfo, inSpec);

    // Passive abilities should auto activate when given
    if (bIsPassiveAbility)
    {
        TryActivatePassiveAbility(inActorInfo, inSpec);
    }
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
    // End the ability.
    EndAbility(inSpecHandle, inActorInfo, inActivationInfo, inShouldReplicateEndAbility, inWasCanceled);
}

void UASSGameplayAbility::TryActivatePassiveAbility(
    const FGameplayAbilityActorInfo* inActorInfo,
    const FGameplayAbilitySpec& inSpec) const
{
    if (!bIsPassiveAbility)
    {
        check(0); // passive ability function was called but this ability isn't passive
        return;
    }

    // TODO: Try to remove usage of `FGameplayAbilitySpec::ActivationInfo` as it's deprecated and non-instance only.
    const bool bIsPredicting = (inSpec.ActivationInfo.ActivationMode == EGameplayAbilityActivationMode::Predicting);
    if (inActorInfo && !inSpec.IsActive() && !bIsPredicting)
    {
        UAbilitySystemComponent* ASC = inActorInfo->AbilitySystemComponent.Get();
        const AActor* AvatarActor = inActorInfo->AvatarActor.Get();

        // If avatar actor is torn off or about to die, don't try to activate it.
        if (ASC && AvatarActor && !AvatarActor->GetTearOff() && (AvatarActor->GetLifeSpan() <= 0.0f))
        {
            const bool bIsLocalExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalPredicted) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalOnly);
            const bool bIsServerExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerOnly) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerInitiated);

            const bool bClientShouldActivate = inActorInfo->IsLocallyControlled() && bIsLocalExecution;
            const bool bServerShouldActivate = inActorInfo->IsNetAuthority() && bIsServerExecution;

            if (bClientShouldActivate || bServerShouldActivate)
            {
                ASC->TryActivateAbility(inSpec.Handle);
            }
        }
    }
}
