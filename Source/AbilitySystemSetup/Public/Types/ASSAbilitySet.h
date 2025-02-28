// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"

#include "ASSAbilitySet.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UAttributeSet;
struct FGameplayAbilitySpecHandle;
struct FActiveGameplayEffectHandle;

/**
 * Stores granted handles from a specific Ability Set.
 * Used often for cleanup of an AbilitySystemComponent (e.g. Character death).
 */
USTRUCT()
struct ABILITYSYSTEMSETUP_API FASSAbilitySetGrantedHandles
{
    GENERATED_BODY()

    friend class UASSAbilitySet;
public:
    /** Remove granted Attribute Sets, remove granted Effects, and clear granted Abilities (e.g. death of Avatar Actor). */
    void RemoveFromAbilitySystemComponent();

public:
    void Clear();

protected:
    UPROPERTY(Transient)
    TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;
    UPROPERTY(Transient)
    TArray<FActiveGameplayEffectHandle> ActiveEffectHandles;
    UPROPERTY(Transient)
    TArray<TObjectPtr<UAttributeSet>> GrantedAttributeSets;


    UPROPERTY(Transient)
    TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
};


/**
 * Defines a group for GAS related clases with funtionality to grant them (abilities, effects and attribute sets).
 */
UCLASS(Blueprintable, Const)
class ABILITYSYSTEMSETUP_API UASSAbilitySet : public UObject
{
    GENERATED_BODY()

public:
    /** Grants the Ability Set while outputing handles that can be used later for removal. */
    void GrantToAbilitySystemComponent(UAbilitySystemComponent& InASC, UObject& InSourceObject, FASSAbilitySetGrantedHandles& OutGrantedHandles) const;

protected:
    /** Abilities to give on grant NOTE: These Abilities are given a level of 1. */
    UPROPERTY(EditDefaultsOnly, Category = "AbilitySystemSetup | Abilities")
    TArray<const TSubclassOf<UGameplayAbility>> GrantedAbilities;

    /** Effects to apply on grant (e.g. GE_InitCharacter, GE_HealthRegen) NOTE: These Effects are assigned a level of 1. */
    UPROPERTY(EditDefaultsOnly, Category = "AbilitySystemSetup | Effects")
    TArray<const TSubclassOf<UGameplayEffect>> GrantedEffects;

    /** Attribute Sets to create and add on grant. */
    UPROPERTY(EditDefaultsOnly, Category = "AbilitySystemSetup | AttributeSets")
    TArray<const TSubclassOf<UAttributeSet>> GrantedAttributeSets;

};
