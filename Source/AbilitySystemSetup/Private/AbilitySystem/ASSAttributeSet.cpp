// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ASSAttributeSet.h"

#include "AbilitySystem/ASSAbilitySystemComponent.h"
#include "GameplayAbilities/Public/GameplayEffectExtension.h"
#include "AbilitySystem/AttributeSets/GameplayEffect_DefaultAttributes.h"



//														EXAMPLE ATTRIBUTE SET CONSTRUCTOR
//											- illustrates how to use SetSoftAttributeDefaults() -
#if 0
UASSAttributeSet::UASSAttributeSet()
	: MaxHealth(150),					// A hard attribute default set to a hard value
	//Health(GetMaxHealth())			// A soft attribute default (DON'T DO THIS HERE, do it in SetSoftAttributeDefaults() instead)
{
	SetSoftAttributeDefaults();		// Call this at the beginning of the constructor, right after the hard defaults are set
	
	
}

void UASSAttributeSet::SetSoftAttributeDefaults()
{
	Health = GetMaxHealth()				// Soft attribute defaults in this event rather than directly in the constructor
}
#endif


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

void UASSAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EffectSpec.Def->IsA(UGameplayEffect_DefaultAttributes::StaticClass()))
	{
		SetSoftAttributeDefaults();
		ClientSetSoftAttributeDefaults(); // PostGameplayEffectExecute() is server only, call SetSoftAttributeDefaults() on the client so he can have correct defaults too. (this is kind of annoying because what if attributes are modified before this RPC gets recieved)
	}
}

void UASSAttributeSet::ClientSetSoftAttributeDefaults_Implementation()
{
	SetSoftAttributeDefaults();
}
