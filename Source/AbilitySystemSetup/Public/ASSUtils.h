// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class UAbilitySystemComponent;
class UAttributeSet;
class UGameplayAbility;

struct FGameplayAbilitySpec;
struct FGameplayAbilitySpecHandle;
struct FGameplayCueParameters;
struct FGameplayTargetDataFilterHandle;
struct FASSGameplayTargetDataFilter;
struct FASSGameplayTargetDataFilter_MultiFilter;

/**
 * Ability System Setup utilities.
 */
namespace ASSUtils
{
    /**
     * Get an Attribute Set by class off of the given ASC.
     */
    ABILITYSYSTEMSETUP_API UAttributeSet* GetAttributeSet(const UAbilitySystemComponent* asc, const TSubclassOf<UAttributeSet> attributeSetClass);
    /**
     * Templated version of GetAttributeSet().
     */
    template <class T>
    UAttributeSet* GetAttributeSet(const UAbilitySystemComponent* asc);
    /**
     * Templated version of GetAttributeSet().
     * Returns the desired Attribute Set in its type.
     */
    template <class T>
    T* GetAttributeSetCasted(const UAbilitySystemComponent* asc);

    // Gameplay Cue helpers for running them locally
    ABILITYSYSTEMSETUP_API void ExecuteGameplayCueLocal(const UAbilitySystemComponent* asc, const FGameplayTag& gameplayCueTag, const FGameplayCueParameters& gameplayCueParameters);
    ABILITYSYSTEMSETUP_API void AddGameplayCueLocal(const UAbilitySystemComponent* asc, const FGameplayTag& gameplayCueTag, const FGameplayCueParameters& gameplayCueParameters);
    ABILITYSYSTEMSETUP_API void RemoveGameplayCueLocal(const UAbilitySystemComponent* asc, const FGameplayTag& gameplayCueTag, const FGameplayCueParameters& gameplayCueParameters);

    /** Returns a list of currently active Ability instances that match the Tags */
    ABILITYSYSTEMSETUP_API void GetActiveAbilitiesWithTags(const UAbilitySystemComponent* asc, const FGameplayTagContainer& tags, TArray<UGameplayAbility*>& outActiveAbilities);

    /** Returns an Ability spec handle from a class. If modifying call MarkAbilitySpecDirty() */
    ABILITYSYSTEMSETUP_API FGameplayAbilitySpecHandle FindAbilitySpecHandleFromClass(UAbilitySystemComponent* asc, const TSubclassOf<UGameplayAbility> abilityClass, const UObject* sourceObject = nullptr);

    /** Give a list of Abilities by specs */
    ABILITYSYSTEMSETUP_API void GiveAbilities(UAbilitySystemComponent* asc, const TArray<FGameplayAbilitySpec>& abilities);

    /**
     * Tell Ability System that Ability input has been pressed.
     * Similar to UAbilitySystemComponent::AbilityLocalInputPressed() but without InputID enum nonsense.
     */
    ABILITYSYSTEMSETUP_API void AbilityLocalInputPressedForSpec(UAbilitySystemComponent* asc, FGameplayAbilitySpec& gameplayAbilitySpec, const bool allowAbilityActivation = true);
    /**
     * Tell Ability System that Ability input has been pressed.
     * Similar to UAbilitySystemComponent::AbilityLocalInputReleased() but without InputID enum nonsense.
     */
    ABILITYSYSTEMSETUP_API void AbilityLocalInputReleasedForSpec(UAbilitySystemComponent* asc, FGameplayAbilitySpec& gameplayAbilitySpec);

    /**
     * UAbilitySystemComponent::TargetConfirm() but modified to only confirm targeting on
     * the Target Actors associated with the given ability.
     */
    ABILITYSYSTEMSETUP_API void TargetConfirmForAbility(UAbilitySystemComponent* asc, const UGameplayAbility* ability);
    /**
     * UAbilitySystemComponent::TargetCancel() but modified to re-add the Target Actors that
     * are not associated with the given ability.
     */
    ABILITYSYSTEMSETUP_API void TargetCancelForAbility(UAbilitySystemComponent* asc, const UGameplayAbility* ability);

    /**
     * Create a handle for filtering target data, filling out all fields
     */
    ABILITYSYSTEMSETUP_API FGameplayTargetDataFilterHandle MakeASSFilterHandle(const FASSGameplayTargetDataFilter& abilitySystemSetupFilter, AActor* selfActor);
    /**
     * Create a handle for filtering target data, filling out all fields
     */
    ABILITYSYSTEMSETUP_API FGameplayTargetDataFilterHandle MakeMultiFilterHandle(const FASSGameplayTargetDataFilter_MultiFilter& multiFilter, AActor* selfActor);

};

template <class T>
UAttributeSet* ASSUtils::GetAttributeSet(const UAbilitySystemComponent* asc)
{
    return GetAttributeSet(asc, T::StaticClass());
}
template <class T>
T* ASSUtils::GetAttributeSetCasted(const UAbilitySystemComponent* asc)
{
    return Cast<T>(GetAttributeSet<T>(asc));
}
