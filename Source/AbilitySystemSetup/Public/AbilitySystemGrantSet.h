// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"

#include "AbilitySystemGrantSet.generated.h"


class UGameplayAbility;
class UGameplayEffect;
class UAttributeSet;
struct FGameplayAbilitySpecHandle;
struct FActiveGameplayEffectHandle;



/**
 * FAbilitySystemGrantHandles
 * 
 * Keeps track of what has been granted by the UAbilitySystemGrantSet.
 * Provides removal of granted handles.
 */
USTRUCT()
struct ABILITYSYSTEMSETUP_API FAbilitySystemGrantHandles
{
	GENERATED_BODY()

	friend class UAbilitySystemGrantSet;
public:
	/** Remove granted Attribute Sets, remove granted Effects, and clear granted Abilities (e.g. death of Avatar Actor) */
	void RemoveFromAbilitySystemComponent();

	/** Transfer granted Attribute Sets, Effects, and Abilities to a new Ability System Component (e.g. possessing a new Avatar Actor during gameplay) */
	void TransferTo(UAbilitySystemComponent* NewASC);

protected:
	UPROPERTY()
		TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

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
UCLASS(Blueprintable, Const)
class ABILITYSYSTEMSETUP_API UAbilitySystemGrantSet : public UObject
{
	GENERATED_BODY()

public:
	/** Grants the grant set to the specified Ability System Component and outputs their handles that can be used later for removal. */
	void GrantToAbilitySystemComponent(UAbilitySystemComponent* ASC, UObject* SourceObject, FAbilitySystemGrantHandles& OutGrantHandles) const;

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
