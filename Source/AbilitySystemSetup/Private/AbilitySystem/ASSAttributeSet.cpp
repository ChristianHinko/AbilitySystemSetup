// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ASSAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "GameplayAbilities/Public/GameplayEffectExtension.h"



////////////////////////////////////////////////////////
/// UASSAttributeSet
////////////////////////////////////////////////////////

UASSAttributeSet::UASSAttributeSet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}


////////////////////////////////////////////////////////
/// FASSAttributeSetInitter
////////////////////////////////////////////////////////

void FASSAttributeSetInitter::PreloadAttributeSetData(const TArray<UCurveTable*>& CurveData)
{

}

void FASSAttributeSetInitter::InitAttributeSetDefaults(UAbilitySystemComponent* AbilitySystemComponent, FName GroupName, int32 Level, bool bInitialInit) const
{

}

void FASSAttributeSetInitter::ApplyAttributeDefault(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAttribute& InAttribute, FName GroupName, int32 Level) const
{

}

TArray<float> FASSAttributeSetInitter::GetAttributeSetValues(UClass* AttributeSetClass, FProperty* AttributeProperty, FName GroupName) const
{
	return TArray<float>();
}
