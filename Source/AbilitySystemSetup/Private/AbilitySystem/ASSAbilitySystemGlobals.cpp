// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ASSAbilitySystemGlobals.h"

#include "AbilitySystem/Types/ASSGameplayAbilityTypes.h"
#include "AbilitySystem/Types/ASSGameplayEffectTypes.h"



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
