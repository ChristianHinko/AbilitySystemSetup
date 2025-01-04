// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"

#include "ASSAbilitySystemComponent.generated.h"



/**
 * Base Ability System Component
 */
UCLASS()
class ABILITYSYSTEMSETUP_API UASSAbilitySystemComponent : public UAbilitySystemComponent
{
    GENERATED_BODY()

public:
    UASSAbilitySystemComponent(const FObjectInitializer& ObjectInitializer);

    virtual bool ShouldDoServerAbilityRPCBatch() const override { return true; }

    virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;

    /** Not tested yet!!!!!!! Beware. Could also be better optimized I'm sure. Anyways this function resets the ASC as if it were new again. */
    void FullReset();
};
