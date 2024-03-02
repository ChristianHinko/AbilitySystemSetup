// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ASSAbilitySystemGlobals.h"

#include "AbilitySystem/Types/ASSGameplayAbilityTypes.h"
#include "AbilitySystem/Types/ASSGameplayEffectTypes.h"
#include "AbilitySystem/ASSAttributeSet.h"



UASSAbilitySystemGlobals::UASSAbilitySystemGlobals(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
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

//void UASSAbilitySystemGlobals::AllocAttributeSetInitter()
//{
//    GlobalAttributeSetInitter = TSharedPtr<FAttributeSetInitter>(new FASSAttributeSetInitter());
//}
