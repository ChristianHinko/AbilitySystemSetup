// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"

#include "AbilitySet.generated.h"


class UGameplayAbility;
class UGameplayEffect;
class UAttributeSet;
struct FGameplayAbilitySpecHandle;
struct FActiveGameplayEffectHandle;



/**
 * UAbilitySystemGrantHandles
 * 
 * Keeps track of what has been granted by the Ability Set.
 */
UCLASS(BlueprintType)
class ABILITYSYSTEMSETUP_API UAbilitySystemGrantHandles : public UObject
{
	GENERATED_BODY()

	friend class UAbilitySystemGrantSet;
public:
	UAbilitySystemGrantHandles(const FObjectInitializer& ObjectInitializer);


	void RemoveFromAbilitySystemComponent(UAbilitySystemComponent* ASC);

protected:
	UPROPERTY()
		TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	UPROPERTY()
		TArray<FActiveGameplayEffectHandle> ActiveEffectHandles;

	UPROPERTY()
		TArray<UAttributeSet*> GrantedAttributeSets;

};


/**
 * UAbilitySystemGrantSet
 * 
 * Non-mutable "ability set" for granting Gameplay Abilities, Gameplay Effects, and Attribute Sets.
 */
UCLASS(BlueprintType, Blueprintable, Const)
class ABILITYSYSTEMSETUP_API UAbilitySystemGrantSet : public UObject
{
	GENERATED_BODY()

public:
	UAbilitySystemGrantSet(const FObjectInitializer& ObjectInitializer);


	/** Grants the Ability Set to the specified Ability System Component and outputs their handles that can be used later for removal. */
	void GrantToAbilitySystemComponent(UAbilitySystemComponent* ASC, UObject* SourceObject, UAbilitySystemGrantHandles*& OutGrantHandles) const;

protected:
	/** Abilities to give on grant NOTE: These Abilities are assigned EAbilityInputID::None and a level of 1 */
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
		TArray<const TSubclassOf<UGameplayAbility>> GrantedAbilities;
	
	/** Effects to apply on grant (e.g. GE_InitCharacter, GE_HealthRegen) NOTE: These Effects are assigned a level of 1 */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
		TArray<const TSubclassOf<UGameplayEffect>> GrantedEffects;
	
	/** Attribute Sets to create and add on grant */
	UPROPERTY(EditDefaultsOnly, Category = "AttributeSets")
		TArray<const TSubclassOf<UAttributeSet>> GrantedAttributeSets;

};
