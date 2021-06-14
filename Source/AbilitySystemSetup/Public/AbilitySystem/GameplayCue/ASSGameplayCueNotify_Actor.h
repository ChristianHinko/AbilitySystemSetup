// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"

#include "ASSGameplayCueNotify_Actor.generated.h"



/**
 * Base gameplay cue actor class
 */
UCLASS()
class ABILITYSYSTEMSETUP_API AASSGameplayCueNotify_Actor : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	AASSGameplayCueNotify_Actor();

};
