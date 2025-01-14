// Fill out your copyright notice in the Description page of Project Settings.

#include "ASSUtils.h"

#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "Types/ASSGameplayTargetDataFilter.h"
#include "AbilitySystemComponent.h"
#include "GCUtils_Log.h"
#include "Abilities/GameplayAbility.h"
#include "Abilities/GameplayAbilityTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogASSUtils, Log, All);

namespace
{
    /**
     * @brief Internal function for calling on the inaccessable `UGameplayAbility::EndAbility()` function`.
     */
    template
        <
        void (UGameplayAbility::* pointerToMember)(const FGameplayAbilitySpecHandle, const FGameplayAbilityActorInfo*, const FGameplayAbilityActivationInfo, bool, bool) =
            &UGameplayAbility::EndAbility
        >
    void CallEndAbilityInternal(
        UGameplayAbility& inGameplayAbility,
        const FGameplayAbilitySpecHandle& inSpecHandle,
        const FGameplayAbilityActorInfo* inActorInfo,
        const FGameplayAbilityActivationInfo& inActivationInfo,
        const bool inShouldReplicateEndAbility,
        const bool inWasCanceled)
    {
        (inGameplayAbility.*pointerToMember)(inSpecHandle, inActorInfo, inActivationInfo, inShouldReplicateEndAbility, inWasCanceled);
    }
}

UAttributeSet* ASSUtils::GetAttributeSet(const UAbilitySystemComponent* asc, const TSubclassOf<UAttributeSet> attributeSetClass)
{
    // Potentially find an Attribute Set of this class
    UAttributeSet* const * const foundAttributeSet = asc->GetSpawnedAttributes().FindByPredicate(
        [&attributeSetClass](const UAttributeSet* attributeSet)
        {
            return (attributeSet->GetClass() == attributeSetClass);
        }
    );

    if (foundAttributeSet)
    {
        // Found it!
        return *foundAttributeSet;
    }

    return nullptr;
}

void ASSUtils::ExecuteGameplayCueLocal(const UAbilitySystemComponent* asc, const FGameplayTag& gameplayCueTag, const FGameplayCueParameters& gameplayCueParameters)
{
    UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(asc->GetOwner(), gameplayCueTag, EGameplayCueEvent::Type::Executed, gameplayCueParameters);
}

void ASSUtils::AddGameplayCueLocal(const UAbilitySystemComponent* asc, const FGameplayTag& gameplayCueTag, const FGameplayCueParameters& gameplayCueParameters)
{
    UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(asc->GetOwner(), gameplayCueTag, EGameplayCueEvent::Type::OnActive, gameplayCueParameters);
}

void ASSUtils::RemoveGameplayCueLocal(const UAbilitySystemComponent* asc, const FGameplayTag& gameplayCueTag, const FGameplayCueParameters& gameplayCueParameters)
{
    UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(asc->GetOwner(), gameplayCueTag, EGameplayCueEvent::Type::Removed, gameplayCueParameters);
}

void ASSUtils::GetActiveAbilitiesWithTags(const UAbilitySystemComponent* asc, const FGameplayTagContainer& tags, TArray<UGameplayAbility*>& outActiveAbilities)
{
    // Get the active specs by tags
    TArray<FGameplayAbilitySpec*> activeAbilitySpecsByTags;
    asc->GetActivatableGameplayAbilitySpecsByAllMatchingTags(tags, activeAbilitySpecsByTags, false);

    // Get all of the instances of these specs
    for (const FGameplayAbilitySpec* spec : activeAbilitySpecsByTags)
    {
        for (UGameplayAbility* activeAbility : spec->GetAbilityInstances())
        {
            outActiveAbilities.Add(activeAbility);
        }
    }
}

FGameplayAbilitySpecHandle ASSUtils::FindAbilitySpecHandleFromClass(UAbilitySystemComponent* asc, const TSubclassOf<UGameplayAbility> abilityClass, const UObject* sourceObject)
{
    // Find by class
    FScopedAbilityListLock activeScopeLock(*asc); // ABILITYLIST_SCOPE_LOCK
    for (const FGameplayAbilitySpec& spec : asc->GetActivatableAbilities())
    {
        if (spec.Ability->GetClass() == abilityClass)
        {
            // Use the specified source object
            if (sourceObject && spec.SourceObject != sourceObject)
            {
                continue;
            }

            // Found it
            return spec.Handle;
        }
    }

    return FGameplayAbilitySpecHandle();
}

void ASSUtils::GiveAbilities(UAbilitySystemComponent* asc, const TArray<FGameplayAbilitySpec>& abilities)
{
    if (!asc)
    {
        GC_LOG_STR_UOBJECT(asc, LogASSUtils, Warning, TEXT("The variable asc was null when trying to give list of abilities. Did nothing"));
        return;
    }
    if (asc->IsOwnerActorAuthoritative() == false)
    {
        GC_LOG_STR_UOBJECT(asc, LogASSUtils, Warning, TEXT("Called without Authority. Did nothing"));
        return;
    }

    for (const FGameplayAbilitySpec& specToGive : abilities)
    {
        if (asc->GetActivatableAbilities().ContainsByPredicate(
            [&specToGive](const FGameplayAbilitySpec& spec)
            {
                return spec.Ability == specToGive.Ability;
            }
        ) == false)
        {
            asc->GiveAbility(specToGive);
        }
    }
}

void ASSUtils::AbilityLocalInputPressedForSpec(UAbilitySystemComponent* asc, FGameplayAbilitySpec& spec, const bool allowAbilityActivation)
{
    // [duplicate-code-engine] Duplicate logic from enigine 5.5 UAbilitySystemComponent::AbilityLocalInputPressed.
    //      Only difference it that we are provided a spec rather than looping through them and choosing one by an ability
    //      input id (we don't use GAS's input id enum system). I wish GAS would break their function up to be more modular
    //      since we just want this part that cares about the spec, but until they do that we have to duplicate this part of their function.
    //      We also introduced the allowAbilityActivation in our implementation (super small addition).

    if (spec.Ability)
    {
        spec.InputPressed = true;
        if (spec.IsActive())
        {
            if (spec.Ability->bReplicateInputDirectly && asc->IsOwnerActorAuthoritative() == false)
            {
                asc->ServerSetInputPressed(spec.Handle);
            }

            asc->AbilitySpecInputPressed(spec);

PRAGMA_DISABLE_DEPRECATION_WARNINGS
            // Fixing this up to use the instance activation, but this function should be deprecated as it cannot work with InstancedPerExecution
            UE_CLOG(spec.Ability->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::InstancedPerExecution, LogASSUtils, Warning, TEXT("%hs: %s is InstancedPerExecution. This is unreliable for Input as you may only interact with the latest spawned Instance"), __func__, *GetNameSafe(spec.Ability));
            TArray<UGameplayAbility*> Instances = spec.GetAbilityInstances();
            const FGameplayAbilityActivationInfo& ActivationInfo = Instances.IsEmpty() ? spec.ActivationInfo : Instances.Last()->GetCurrentActivationInfoRef();
PRAGMA_ENABLE_DEPRECATION_WARNINGS
            // Invoke the InputPressed event. This is not replicated here. If someone is listening, they may replicate the InputPressed event to the server.
            asc->InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, spec.Handle, ActivationInfo.GetActivationPredictionKey());
        }
        else
        {
            if (allowAbilityActivation) // [modified-code-engine] Wrapped in if statement with our `allowAbilityActivation` parameter.
            {
                // Ability is not active, so try to activate it
                asc->TryActivateAbility(spec.Handle);
            }
        }
    }
}

void ASSUtils::AbilityLocalInputReleasedForSpec(UAbilitySystemComponent* asc, FGameplayAbilitySpec& spec)
{
    // [duplicate-code-engine] Duplicate logic from enigine 5.5 UAbilitySystemComponent::AbilityLocalInputReleased.
    //      Only difference it that we are provided a spec rather than looping through them and choosing one by an ability
    //      input id (we don't use GAS's input id enum system). I wish GAS would break their function up to be more modular
    //      since we just want this part that cares about the spec, but until they do that we have to duplicate this part of their function.

    spec.InputPressed = false;
    if (spec.Ability && spec.IsActive())
    {
        if (spec.Ability->bReplicateInputDirectly && asc->IsOwnerActorAuthoritative() == false)
        {
            asc->ServerSetInputReleased(spec.Handle);
        }

        asc->AbilitySpecInputReleased(spec);

PRAGMA_DISABLE_DEPRECATION_WARNINGS
        // Fixing this up to use the instance activation, but this function should be deprecated as it cannot work with InstancedPerExecution
        UE_CLOG(spec.Ability->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::InstancedPerExecution, LogASSUtils, Warning, TEXT("%hs: %s is InstancedPerExecution. This is unreliable for Input as you may only interact with the latest spawned Instance"), __func__, *GetNameSafe(spec.Ability));
        TArray<UGameplayAbility*> Instances = spec.GetAbilityInstances();
        const FGameplayAbilityActivationInfo& ActivationInfo = Instances.IsEmpty() ? spec.ActivationInfo : Instances.Last()->GetCurrentActivationInfoRef();
PRAGMA_ENABLE_DEPRECATION_WARNINGS
        asc->InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, spec.Handle, ActivationInfo.GetActivationPredictionKey());
    }
}

void ASSUtils::TargetConfirmForAbility(UAbilitySystemComponent* asc, const UGameplayAbility* ability)
{
    // [duplicate-code-engine] The following is coppied code from UAbilitySystemComponent::TargetConfirm()
    //      but with an ability check added.

    // Callbacks may modify the spawned target actor array so iterate over a copy instead
    TArray<AGameplayAbilityTargetActor*> localTargetActors = asc->SpawnedTargetActors;
    asc->SpawnedTargetActors.Reset();
    for (AGameplayAbilityTargetActor* targetActor : localTargetActors)
    {
        if (targetActor)
        {
            if (targetActor->IsConfirmTargetingAllowed())
            {
                if (targetActor->OwningAbility == ability) // [modified-code-engine] Wrapped in this if statement to only trigger confirm for specific ability.
                {
                    //TODO: There might not be any cases where this bool is false
                    if (!targetActor->bDestroyOnConfirmation)
                    {
                        asc->SpawnedTargetActors.Add(targetActor);
                    }
                    targetActor->ConfirmTargeting();
                }
            }
            else
            {
                asc->SpawnedTargetActors.Add(targetActor);
            }
        }
    }
}

void ASSUtils::TargetCancelForAbility(UAbilitySystemComponent* asc, const UGameplayAbility* ability)
{
    // [duplicate-code-engine] The following is coppied code from UAbilitySystemComponent::TargetCancel()
    //      but with an ability check added.
    
    // Callbacks may modify the spawned target actor array so iterate over a copy instead
    TArray<AGameplayAbilityTargetActor*> localTargetActors = asc->SpawnedTargetActors;
    asc->SpawnedTargetActors.Reset();
    for (AGameplayAbilityTargetActor* targetActor : localTargetActors)
    {
        if (targetActor)
        {
            if (targetActor->OwningAbility == ability) // [modified-code-engine] Wrapped in this if statement to only trigger cancel for specific ability.
            {
                targetActor->CancelTargeting();
            }
            else // [modified-code-engine] Add this else statement.
            {
                asc->SpawnedTargetActors.Add(targetActor);
            }
        }
    }
}

FGameplayTargetDataFilterHandle ASSUtils::MakeASSFilterHandle(const FASSGameplayTargetDataFilter& abilitySystemSetupFilter, AActor* selfActor)
{
    FGameplayTargetDataFilterHandle filterHandle;
    FASSGameplayTargetDataFilter* newFilter = new FASSGameplayTargetDataFilter(abilitySystemSetupFilter);
    newFilter->InitializeFilterContext(selfActor);
    filterHandle.Filter = TSharedPtr<FGameplayTargetDataFilter>(newFilter);
    return filterHandle;
}

FGameplayTargetDataFilterHandle ASSUtils::MakeMultiFilterHandle(const FASSGameplayTargetDataFilter_MultiFilter& multiFilter, AActor* selfActor)
{
    FGameplayTargetDataFilterHandle filterHandle;
    FASSGameplayTargetDataFilter_MultiFilter* newFilter = new FASSGameplayTargetDataFilter_MultiFilter(multiFilter);
    newFilter->InitializeFilterContext(selfActor);
    filterHandle.Filter = TSharedPtr<FGameplayTargetDataFilter>(newFilter);
    return filterHandle;
}

bool ASSUtils::TryActivateAbilityPassive(
    UAbilitySystemComponent& inAbilitySystemComponent,
    const FGameplayAbilitySpec& inAbilitySpec)
{
    // TODO: Try to remove usage of `FGameplayAbilitySpec::ActivationInfo` as it's deprecated and non-instance only.
    const bool isPredicting = inAbilitySpec.ActivationInfo.ActivationMode == EGameplayAbilityActivationMode::Predicting;
    if (isPredicting)
    {
        return false;
    }

    if (inAbilitySpec.IsActive())
    {
        return false;
    }

    check(inAbilitySystemComponent.AbilityActorInfo.IsValid());
    const FGameplayAbilityActorInfo& actorInfo = *inAbilitySystemComponent.AbilityActorInfo.Get();

    {
        const AActor* avatarActor = actorInfo.AvatarActor.Get();

        if (!avatarActor)
        {
            return false;
        }

        const bool isTornOff = avatarActor->GetTearOff();
        if (isTornOff)
        {
            return false;
        }

        if (avatarActor->GetLifeSpan() > 0.f)
        {
            return false;
        }
    }

    {
        const UGameplayAbility* abilityCDO = inAbilitySpec.Ability;

        if (!ensure(abilityCDO))
        {
            return false;
        }

        const EGameplayAbilityNetExecutionPolicy::Type abilityExecutionPolicy = abilityCDO->GetNetExecutionPolicy();

        const bool shouldActivateLocal = actorInfo.IsLocallyControlled() && IsLocalActivatedExecution(abilityExecutionPolicy);
        const bool shouldActivateServer = actorInfo.IsNetAuthority() && IsServerActivatedExecution(abilityExecutionPolicy);
        if (!shouldActivateLocal && !shouldActivateServer)
        {
            return false;
        }
    }

    constexpr bool shouldAllowRemoteActivation = true;
    return inAbilitySystemComponent.TryActivateAbility(inAbilitySpec.Handle, shouldAllowRemoteActivation);
}

bool ASSUtils::IsLocalActivatedExecution(
    const EGameplayAbilityNetExecutionPolicy::Type inAbilityExecutionPolicy)
{
    switch (inAbilityExecutionPolicy)
    {
    case EGameplayAbilityNetExecutionPolicy::LocalPredicted:
    case EGameplayAbilityNetExecutionPolicy::LocalOnly:
        return true;
    }

    return false;
}

bool ASSUtils::IsServerActivatedExecution(
    const EGameplayAbilityNetExecutionPolicy::Type inAbilityExecutionPolicy)
{
    switch (inAbilityExecutionPolicy)
    {
    case EGameplayAbilityNetExecutionPolicy::ServerInitiated:
    case EGameplayAbilityNetExecutionPolicy::ServerOnly:
        return true;
    }

    return false;
}

void ASSUtils::CallEndAbility(
    UGameplayAbility& inGameplayAbility,
    const FGameplayAbilitySpecHandle& inSpecHandle,
    const FGameplayAbilityActorInfo* inActorInfo,
    const FGameplayAbilityActivationInfo& inActivationInfo,
    const bool inShouldReplicateEndAbility,
    const bool inWasCanceled)
{
    CallEndAbilityInternal(
        inGameplayAbility,
        inSpecHandle,
        inActorInfo,
        inActivationInfo,
        inShouldReplicateEndAbility,
        inWasCanceled);
}
void ASSUtils::CallEndAbility(
    UGameplayAbility& inGameplayAbility,
    const bool inShouldReplicateEndAbility,
    const bool inWasCanceled)
{
    CallEndAbility(
        inGameplayAbility,
        inGameplayAbility.GetCurrentAbilitySpecHandle(),
        inGameplayAbility.GetCurrentActorInfo(),
        inGameplayAbility.GetCurrentActivationInfoRef(),
        inShouldReplicateEndAbility,
        inWasCanceled);
}
