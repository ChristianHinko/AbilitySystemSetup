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
	 * Get an Attribute Set by class off of the given ASC
	 */
	UFUNCTION(Category = "AttributeSet")
		static UAttributeSet* GetAttributeSet(const UAbilitySystemComponent* ASC, const TSubclassOf<UAttributeSet> AttributeSetClass);
	/**
	 * Get an Attribute Set by class off of the given ASC
	 */
	template <typename T>
	static UAttributeSet* GetAttributeSet(const UAbilitySystemComponent* ASC);
	/**
	 * Get an Attribute Set by class off of the given ASC.
	 * Returns the desired Attribute Set in its type.
	 */
	template <typename T>
	static T* GetAttributeSetCasted(const UAbilitySystemComponent* ASC);


	// Gameplay cue helpers for running them locally
	static void ExecuteGameplayCueLocal(const UAbilitySystemComponent* InASC, const FGameplayTag& GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);
	static void AddGameplayCueLocal(const UAbilitySystemComponent* InASC, const FGameplayTag& GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);
	static void RemoveGameplayCueLocal(const UAbilitySystemComponent* InASC, const FGameplayTag& GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);

	/** Returns a list of currently active ability instances that match the tags */
	static void GetActiveAbilitiesWithTags(const UAbilitySystemComponent* InASC, const FGameplayTagContainer& GameplayTagContainer, TArray<UGameplayAbility*>& ActiveAbilities);

	/** Returns an ability spec handle from a class. If modifying call MarkAbilitySpecDirty */
	static FGameplayAbilitySpecHandle FindAbilitySpecHandleFromClass(UAbilitySystemComponent* InASC, TSubclassOf<UGameplayAbility> AbilityClass, UObject* OptionalSourceObject = nullptr);


	static void GiveAbilities(UAbilitySystemComponent* ASCToAddTo, const TArray<FGameplayAbilitySpec>& Abilities);


	/**
	 * Create a handle for filtering target data, filling out all fields
	 */
	UFUNCTION(BlueprintPure, Category = "Filter")
		static FGameplayTargetDataFilterHandle MakeASSFilterHandle(const FASSGameplayTargetDataFilter& ASSFilter, AActor* SelfActor);
	/**
	 * Create a handle for filtering target data, filling out all fields
	 */
	UFUNCTION(BlueprintPure, Category = "Filter")
		static FGameplayTargetDataFilterHandle MakeMultiFilterHandle(const FGTDF_MultiFilter& MultiFilter, AActor* SelfActor);

};


template <typename T>
UAttributeSet* UASSAbilitySystemBlueprintLibrary::GetAttributeSet(const UAbilitySystemComponent* ASC)
{
	return GetAttributeSet(ASC, T::StaticClass());
}
template <typename T>
T* UASSAbilitySystemBlueprintLibrary::GetAttributeSetCasted(const UAbilitySystemComponent* ASC)
{
	return Cast<T>(GetAttributeSet<T>(ASC));
}
