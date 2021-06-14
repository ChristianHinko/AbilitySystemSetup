// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AttributeSets/AS_Damage.h"

#include "Net/UnrealNetwork.h"



void UAS_Damage::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);


	FDoRepLifetimeParams Params;
	Params.Condition = COND_None;
	Params.RepNotifyCondition = REPNOTIFY_Always;

	Params.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(UAS_Damage, OutgoingDamage, Params);
}

UAS_Damage::UAS_Damage()
	: OutgoingDamage(100)
{


}

// OnReps
void UAS_Damage::OnRep_OutgoingDamage(const FGameplayAttributeData& ServerBaseValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAS_Damage, OutgoingDamage, ServerBaseValue);
}
