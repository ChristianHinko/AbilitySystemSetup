// Fill out your copyright notice in the Description page of Project Settings.

#include "ASSGameplayAbility.h"

#include "Utilities/ISNativeGameplayTags.h"

UASSGameplayAbility::UASSGameplayAbility(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    bServerRespectsRemoteAbilityCancellation = false;
    NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnlyTermination;

    bPassiveAbility = false;
}

void UASSGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    Super::OnAvatarSet(ActorInfo, Spec);

    // TODO: Try to remove usage of `EGameplayAbilityInstancingPolicy::NonInstanced`. This code looks like it
    // might be unnecessary now too since non-instanced is deprecated.

    // Fix the engine accidently calling OnAvatarSet() on CDO instead of calling it on the instances
    if (GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced && !IsInstantiated())
    {
        // Broken call to OnAvatarSet().
        // Do the fix - call our version on each instance.
        for (UGameplayAbility* Ability : Spec.GetAbilityInstances())
        {
            UASSGameplayAbility* ASSAbility = Cast<UASSGameplayAbility>(Ability);
            if (IsValid(ASSAbility))
            {
                ASSAbility->ASSOnAvatarSet(ActorInfo, Spec);
            }
        }
        return;
    }

    // Nothing went wrong. Call our version.
    // If we are NonInstanced, then being the CDO is okay.
    // If we are instanced, then the engine called us from UGameplayAbility::OnGiveAbility().
    ASSOnAvatarSet(ActorInfo, Spec);
    return;
}

void UASSGameplayAbility::ASSOnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    // Safe event for on avatar set
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    if (GetAssetTags().IsEmpty())
    {
        UE_LOG(LogASSAbilitySetup, Warning, TEXT("%s() Ability implementor forgot to assign an Ability Tag to this ability. We try to enforce activating abilities by tag for organization reasons"), ANSI_TO_TCHAR(__FUNCTION__));
        check(0);
    }
    if (GetAssetTags().HasTag(ISNativeGameplayTags::InputAction) == false)
    {
        UE_LOG(LogASSAbilitySetup, Warning, TEXT("%s() Ability implementor forgot to assign an input action Ability Tag to this ability. We enforce this so that a given an input action can identify any abilities it activates. If the ability isn't intended to be activated by input you can suppress this with InputAction.None tag."), ANSI_TO_TCHAR(__FUNCTION__));
        check(0);
    }
#endif // !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
}

void UASSGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
    Super::OnGiveAbility(ActorInfo, Spec);

    // Passive abilities should auto activate when given
    if (bPassiveAbility)
    {
        TryActivatePassiveAbility(ActorInfo, Spec);
    }
}

bool UASSGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }

    return true;
}

void UASSGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    //  BEGIN Copied from Super (for Blueprint support)
    if (bHasBlueprintActivate)
    {
        // A Blueprinted ActivateAbility function must call CommitAbility somewhere in its execution chain.
        K2_ActivateAbility();
    }
    else if (bHasBlueprintActivateFromEvent)
    {
        if (TriggerEventData)
        {
            // A Blueprinted ActivateAbility function must call CommitAbility somewhere in its execution chain.
            K2_ActivateAbilityFromEvent(*TriggerEventData);
        }
        else
        {
            UE_LOG(LogASSAbilitySetup, Warning, TEXT("Ability %s expects event data but none is being supplied. Use Activate Ability instead of Activate Ability From Event."), *GetName());
            bool bReplicateEndAbility = false;
            bool bWasCancelled = true;
            EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
        }
    }
    //  END Copied from Super (for Blueprint support)
}

void UASSGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    // Call our ASSEndAbility() at a safe point
    if (IsEndAbilityValid(Handle, ActorInfo))
    {
        if (ScopeLockCount > 0)
        {
            WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &ThisClass::EndAbility, Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled));
            return;
        }

        // This is the safe point to do end ability event logic
        ASSEndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
    }

    // End the ability
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UASSGameplayAbility::ASSEndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    // Safe event for end ability
}

void UASSGameplayAbility::ExternalEndAbility()
{
    check(CurrentActorInfo);

    const bool bReplicateEndAbility = true;
    const bool bWasCancelled = false;
    EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), bReplicateEndAbility, bWasCancelled);
}

void UASSGameplayAbility::TryActivatePassiveAbility(const FGameplayAbilityActorInfo* InActorInfo, const FGameplayAbilitySpec& InSpec) const
{
    if (!bPassiveAbility)
    {
        check(0); // passive ability function was called but this ability isn't passive
        return;
    }

    // TODO: Try to remove usage of `FGameplayAbilitySpec::ActivationInfo` as it's deprecated and non-instance only.
    const bool bIsPredicting = (InSpec.ActivationInfo.ActivationMode == EGameplayAbilityActivationMode::Predicting);
    if (InActorInfo && !InSpec.IsActive() && !bIsPredicting)
    {
        UAbilitySystemComponent* ASC = InActorInfo->AbilitySystemComponent.Get();
        const AActor* AvatarActor = InActorInfo->AvatarActor.Get();

        // If avatar actor is torn off or about to die, don't try to activate it.
        if (ASC && AvatarActor && !AvatarActor->GetTearOff() && (AvatarActor->GetLifeSpan() <= 0.0f))
        {
            const bool bIsLocalExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalPredicted) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalOnly);
            const bool bIsServerExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerOnly) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerInitiated);

            const bool bClientShouldActivate = InActorInfo->IsLocallyControlled() && bIsLocalExecution;
            const bool bServerShouldActivate = InActorInfo->IsNetAuthority() && bIsServerExecution;

            if (bClientShouldActivate || bServerShouldActivate)
            {
                ASC->TryActivateAbility(InSpec.Handle);
            }
        }
    }
}
