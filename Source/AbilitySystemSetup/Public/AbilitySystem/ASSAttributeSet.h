// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"

#include "ASSAttributeSet.generated.h"



// Uses macros from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


/**
 * Base Attribute Set
 */
UCLASS()
class ABILITYSYSTEMSETUP_API UASSAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:


protected:
	/** Helper function to proportionally adjust the value of an attribute when it's associated max attribute changes. (i.e. When MaxHealth increases, Health increases by an amount that maintains the same percentage as before) */
	void AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty);


	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	/**
	 * This event is called whenever a default stats Effect has been applied (a Gameplay Effect with the asset tag "Effect.Initialization").
	 * Useful for non-Attribute typed properties that exist on the Attribute Set.
	 * Could also be useful for any Attributes that require more complex default value calculations BUT you should use an Execution Calculation for that.
	 * NOTE: Server only event!
	 */
	virtual void OnDefaultStatsEffectApplied() { };

};


/**
 * Our custom FAttributeSetInitter.
 * 
 * Not implemented. Optional struct some games may find useful.
 * 
 * Possible reasons to use an FAttributeSetInitter
 *  - Curve tables defining descrete attribute values for each ASC level (this would use FAttributeSetInitterDiscreteLevels)
 *	- We can maybe use this for Curves with infinite scaling as you level up
 */
struct ABILITYSYSTEMSETUP_API FASSAttributeSetInitter : public FAttributeSetInitter
{
public:
	virtual void PreloadAttributeSetData(const TArray<UCurveTable*>& CurveData) override;
	virtual void InitAttributeSetDefaults(UAbilitySystemComponent* AbilitySystemComponent, FName GroupName, int32 Level, bool bInitialInit) const override;
	virtual void ApplyAttributeDefault(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAttribute& InAttribute, FName GroupName, int32 Level) const override;
	virtual TArray<float> GetAttributeSetValues(UClass* AttributeSetClass, FProperty* AttributeProperty, FName GroupName) const override;

};
