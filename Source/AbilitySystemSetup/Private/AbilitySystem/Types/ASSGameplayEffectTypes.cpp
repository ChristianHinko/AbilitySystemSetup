// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Types/ASSGameplayEffectTypes.h"



FASSGameplayEffectContext::FASSGameplayEffectContext()
{

}

bool FASSGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
    return Super::NetSerialize(Ar, Map, bOutSuccess);
}
