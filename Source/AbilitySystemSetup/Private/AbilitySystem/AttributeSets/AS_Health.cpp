// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AttributeSets/AS_Health.h"

#include "Net/UnrealNetwork.h"
#include "GameplayAbilities/Public/GameplayEffectExtension.h"



void UAS_Health::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);


	FDoRepLifetimeParams Params;
	Params.Condition = COND_None;
	Params.RepNotifyCondition = REPNOTIFY_Always;

	Params.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(UAS_Health, MaxHealth, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UAS_Health, Health, Params);
	//	IncomingDamage and Healing not replicated since it's a 'meta' attribute
}

UAS_Health::UAS_Health()
	: MaxHealth(100)//,
	//Health(GetMaxHealth())
{
	SetSoftAttributeDefaults();


}

void UAS_Health::SetSoftAttributeDefaults()
{
	Super::SetSoftAttributeDefaults();

	Health = GetMaxHealth();
}

bool UAS_Health::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	Super::PreGameplayEffectExecute(Data);


	const FGameplayAttribute AttributeToModify = Data.EvaluatedData.Attribute;



	if (AttributeToModify == GetIncomingDamageAttribute())
	{
		//Handle extra Attribute Modifications here (ie. armor buff, damage vulnerability)


	}

	if (AttributeToModify == GetHealingAttribute())
	{
		//Handle extra Attribute Modifications here (ie. less healing, extra healing)


	}


	return true;
}
void UAS_Health::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);


	const FGameplayAttribute ModifiedAttribute = Data.EvaluatedData.Attribute;



	if (ModifiedAttribute == GetIncomingDamageAttribute())
	{
		const float damageToApply = IncomingDamage.GetCurrentValue();
		SetIncomingDamage(0.f);

		SetHealth(FMath::Clamp(GetHealth() - damageToApply, 0.f, GetMaxHealth()));

	}

	if (ModifiedAttribute == GetHealingAttribute())
	{
		const float healingToApply = Healing.GetCurrentValue();
		SetHealing(0.f);

		SetHealth(FMath::Clamp(GetHealth() + healingToApply, 0.f, GetMaxHealth()));

	}
}


// OnReps

void UAS_Health::OnRep_MaxHealth(const FGameplayAttributeData& ServerBaseValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAS_Health, MaxHealth, ServerBaseValue);
}
void UAS_Health::OnRep_Health(const FGameplayAttributeData& ServerBaseValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAS_Health, Health, ServerBaseValue);
}
