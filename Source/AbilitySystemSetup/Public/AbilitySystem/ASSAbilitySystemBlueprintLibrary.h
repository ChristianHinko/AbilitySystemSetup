// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/TargetActor/ASSGameplayAbilityTargetDataFilter.h"

#include "ASSAbilitySystemBlueprintLibrary.generated.h"



/**
 * Base AbilitySystemBlueprintLibrary
 */
UCLASS()
class ABILITYSYSTEMSETUP_API UASSAbilitySystemBlueprintLibrary : public UAbilitySystemBlueprintLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Get an Attribute Set by class off of the given ASC.
	 */
	UFUNCTION(BlueprintPure, Category = "AttributeSet")
		static UAttributeSet* GetAttributeSet(const UAbilitySystemComponent* InASC, const TSubclassOf<UAttributeSet> InAttributeSetClass);
	/**
	 * Templated version of GetAttributeSet().
	 */
	template <class T>
	static UAttributeSet* GetAttributeSet(const UAbilitySystemComponent* InASC);
	/**
	 * Templated version of GetAttributeSet().
	 * Returns the desired Attribute Set in its type.
	 */
	template <class T>
	static T* GetAttributeSetCasted(const UAbilitySystemComponent* InASC);


	// Gameplay Cue helpers for running them locally
	static void ExecuteGameplayCueLocal(const UAbilitySystemComponent* InASC, const FGameplayTag& InGameplayCueTag, const FGameplayCueParameters& InGameplayCueParameters);
	static void AddGameplayCueLocal(const UAbilitySystemComponent* InASC, const FGameplayTag& InGameplayCueTag, const FGameplayCueParameters& InGameplayCueParameters);
	static void RemoveGameplayCueLocal(const UAbilitySystemComponent* InASC, const FGameplayTag& InGameplayCueTag, const FGameplayCueParameters& InGameplayCueParameters);

	/** Returns a list of currently active Ability instances that match the Tags */
	static void GetActiveAbilitiesWithTags(const UAbilitySystemComponent* InASC, const FGameplayTagContainer& InTags, TArray<UGameplayAbility*>& OutActiveAbilities);

	/** Returns an Ability spec handle from a class. If modifying call MarkAbilitySpecDirty() */
	static FGameplayAbilitySpecHandle FindAbilitySpecHandleFromClass(UAbilitySystemComponent* InASC, const TSubclassOf<UGameplayAbility> InAbilityClass, const UObject* InSourceObject = nullptr);

	/** Give a list of Abilities by specs */
	static void GiveAbilities(UAbilitySystemComponent* InASC, const TArray<FGameplayAbilitySpec>& InAbilities);


	/**
	 * UAbilitySystemComponent::TargetConfirm() but modified to only confirm targeting on
	 * the Target Actors associated with the given InAbility.
	 */
	virtual void TargetConfirmByAbility(UAbilitySystemComponent* InASC, const UGameplayAbility* InAbility);
	/**
	 * UAbilitySystemComponent::TargetCancel() but modified to re-add the Target Actors that
	 * are not associated with the given InAbility.
	 */
	virtual void TargetCancelByAbility(UAbilitySystemComponent* InASC, const UGameplayAbility* InAbility);


	/**
	 * Create a handle for filtering target data, filling out all fields
	 */
	UFUNCTION(BlueprintPure, Category = "Filter")
		static FGameplayTargetDataFilterHandle MakeASSFilterHandle(const FASSGameplayTargetDataFilter& InASSFilter, AActor* InSelfActor);
	/**
	 * Create a handle for filtering target data, filling out all fields
	 */
	UFUNCTION(BlueprintPure, Category = "Filter")
		static FGameplayTargetDataFilterHandle MakeMultiFilterHandle(const FASSGameplayTargetDataFilter_MultiFilter& InMultiFilter, AActor* InSelfActor);

};


template <class T>
UAttributeSet* UASSAbilitySystemBlueprintLibrary::GetAttributeSet(const UAbilitySystemComponent* InASC)
{
	return GetAttributeSet(InASC, T::StaticClass());
}
template <class T>
T* UASSAbilitySystemBlueprintLibrary::GetAttributeSetCasted(const UAbilitySystemComponent* InASC)
{
	return Cast<T>(GetAttributeSet<T>(InASC));
}
