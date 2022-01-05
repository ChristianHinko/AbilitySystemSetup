// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ASSAttributeSet.h"

#include "AbilitySystem/ASSAbilitySystemComponent.h"
#include "GameplayAbilities/Public/GameplayEffectExtension.h"



void UASSAttributeSet::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty)
{
	UAbilitySystemComponent* AbilitySystemComponent = GetOwningAbilitySystemComponent();
	const float CurrentMaxValue = MaxAttribute.GetCurrentValue();
	if (!FMath::IsNearlyEqual(CurrentMaxValue, NewMaxValue) && AbilitySystemComponent)
	{
		// Change current value to maintain the current Val / Max percent
		const float CurrentValue = AffectedAttribute.GetCurrentValue();
		float NewDelta = (CurrentMaxValue > 0.f) ? (CurrentValue * NewMaxValue / CurrentMaxValue) - CurrentValue : NewMaxValue;

		AbilitySystemComponent->ApplyModToAttributeUnsafe(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
	}
}


////////////////////////////////////////////////////////////////
/// FASSAttributeSetInitter
////////////////////////////////////////////////////////////////

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
