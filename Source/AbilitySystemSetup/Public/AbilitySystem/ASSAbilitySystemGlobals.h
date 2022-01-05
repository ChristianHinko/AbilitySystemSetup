// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"

#include "ASSAbilitySystemGlobals.generated.h"



/**
 * Base AbilitySystemGlobals
 */
UCLASS()
class ABILITYSYSTEMSETUP_API UASSAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

public:
	UASSAbilitySystemGlobals();


	virtual FGameplayAbilityActorInfo* AllocAbilityActorInfo() const override;
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;

	virtual void AllocAttributeSetInitter() override;

};
