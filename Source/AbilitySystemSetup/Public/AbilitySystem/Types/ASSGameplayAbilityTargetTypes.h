// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetTypes.h"

#include "ASSGameplayAbilityTargetTypes.generated.h"



/**
 * Base Target Data struct
 */
USTRUCT()
struct ABILITYSYSTEMSETUP_API FASSGameplayAbilityTargetData : public FGameplayAbilityTargetData
{
    GENERATED_BODY()

    FASSGameplayAbilityTargetData();


    virtual UScriptStruct* GetScriptStruct() const override { return StaticStruct(); }
};
