// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"

#include "ASSAbilityTask.generated.h"



/**
 * Base Ability Task class
 */
UCLASS()
class ABILITYSYSTEMSETUP_API UASSAbilityTask : public UAbilityTask
{
    GENERATED_BODY()

public:
    UASSAbilityTask(const FObjectInitializer& ObjectInitializer);

};
