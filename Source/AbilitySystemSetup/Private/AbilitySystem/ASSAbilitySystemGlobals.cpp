// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ASSAbilitySystemGlobals.h"

#include "AbilitySystem/ASSGameplayAbilityTypes.h"
#include "AbilitySystem/ASSGameplayEffectTypes.h"



UASSAbilitySystemGlobals::UASSAbilitySystemGlobals()
{

}


FGameplayAbilityActorInfo* UASSAbilitySystemGlobals::AllocAbilityActorInfo() const
{
	return new FASSGameplayAbilityActorInfo();
}
FGameplayEffectContext* UASSAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FASSGameplayEffectContext();
}
