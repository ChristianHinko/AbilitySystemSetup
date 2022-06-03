// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h" // a very common include for Attribute Sets

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
	UASSAttributeSet(const FObjectInitializer& ObjectInitializer);

protected:

};


/**
 * Our custom FAttributeSetInitter.
 * 
 * Optional struct some games may find useful. Not implemented.
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
