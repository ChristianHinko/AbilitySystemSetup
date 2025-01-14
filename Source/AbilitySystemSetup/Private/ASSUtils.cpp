// Fill out your copyright notice in the Description page of Project Settings.

#include "ASSUtils.h"

#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "Types/ASSGameplayTargetDataFilter.h"
#include "AbilitySystemComponent.h"
#include "GCUtils_Log.h"
#include "Abilities/GameplayAbility.h"

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

void ASSUtils::AbilityLocalInputPressedForSpec(UAbilitySystemComponent* asc, FGameplayAbilitySpec& gameplayAbilitySpec, const bool allowAbilityActivation)
{
    gameplayAbilitySpec.InputPressed = true;
    if (gameplayAbilitySpec.IsActive())
    {
        if (gameplayAbilitySpec.Ability->bReplicateInputDirectly && asc->IsOwnerActorAuthoritative() == false)
        {
            asc->ServerSetInputPressed(gameplayAbilitySpec.Handle);
        }

        asc->AbilitySpecInputPressed(gameplayAbilitySpec);

        // TODO: Stop using `FGameplayAbilitySpec::ActivationInfo` as it's deprecated and not per-instance.
        asc->InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, gameplayAbilitySpec.Handle, gameplayAbilitySpec.ActivationInfo.GetActivationPredictionKey());
    }
    else
    {
        if (allowAbilityActivation)
        {
            asc->TryActivateAbility(gameplayAbilitySpec.Handle);
        }
    }
}

void ASSUtils::AbilityLocalInputReleasedForSpec(UAbilitySystemComponent* asc, FGameplayAbilitySpec& gameplayAbilitySpec)
{
    gameplayAbilitySpec.InputPressed = false;
    if (gameplayAbilitySpec.IsActive())
    {
        if (gameplayAbilitySpec.Ability->bReplicateInputDirectly && asc->IsOwnerActorAuthoritative() == false)
        {
            asc->ServerSetInputReleased(gameplayAbilitySpec.Handle);
        }

        asc->AbilitySpecInputReleased(gameplayAbilitySpec);

        // TODO: Stop using `FGameplayAbilitySpec::ActivationInfo` as it's deprecated and not per-instance.
        asc->InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, gameplayAbilitySpec.Handle, gameplayAbilitySpec.ActivationInfo.GetActivationPredictionKey());
    }
}

void ASSUtils::TargetConfirmForAbility(UAbilitySystemComponent* asc, const UGameplayAbility* ability)
{
    // NOTE: The following is coppied code from UAbilitySystemComponent::TargetConfirm() but with an ability check added.

    // Callbacks may modify the spawned target actor array so iterate over a copy instead
    TArray<AGameplayAbilityTargetActor*> localTargetActors = asc->SpawnedTargetActors;
    asc->SpawnedTargetActors.Reset();
    for (AGameplayAbilityTargetActor* targetActor : localTargetActors)
    {
        if (targetActor)
        {
            if (targetActor->IsConfirmTargetingAllowed())
            {
                if (targetActor->OwningAbility == ability) // =@MODIFIED MARKER@= wrapped in this if statement
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
    // NOTE: The following is coppied code from UAbilitySystemComponent::TargetCancel() but with an ability check added.
    
    // Callbacks may modify the spawned target actor array so iterate over a copy instead
    TArray<AGameplayAbilityTargetActor*> localTargetActors = asc->SpawnedTargetActors;
    asc->SpawnedTargetActors.Reset();
    for (AGameplayAbilityTargetActor* targetActor : localTargetActors)
    {
        if (targetActor)
        {
            if (targetActor->OwningAbility == ability) // =@MODIFIED MARKER@= wrapped in this if statement
            {
                targetActor->CancelTargeting();
            }
            else // =@MODIFIED MARKER@= add this else statement
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
        inGameplayAbility.GetCurrentAbilitySpecHandle(),
        inGameplayAbility.GetCurrentActorInfo(),
        inGameplayAbility.GetCurrentActivationInfoRef(),
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
