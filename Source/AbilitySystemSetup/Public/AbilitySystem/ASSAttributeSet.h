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

};

/**
 * Our custom FAttributeSetInitter.
 * 
 * NOT YET IMPLEMENTED!!!!!!
 * We can maybe use this for Curves with infinite scaling as you level up rather than Curve Tables with discrete values (from FAttributeSetInitterDiscreteLevels)
 */
struct ABILITYSYSTEMSETUP_API FASSAttributeSetInitter : public FAttributeSetInitter
{
public:
	virtual void PreloadAttributeSetData(const TArray<UCurveTable*>& CurveData) override;
	virtual void InitAttributeSetDefaults(UAbilitySystemComponent* AbilitySystemComponent, FName GroupName, int32 Level, bool bInitialInit) const override;
	virtual void ApplyAttributeDefault(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAttribute& InAttribute, FName GroupName, int32 Level) const override;
	virtual TArray<float> GetAttributeSetValues(UClass* AttributeSetClass, FProperty* AttributeProperty, FName GroupName) const override;

};
