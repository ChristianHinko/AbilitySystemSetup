// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"

#include "ASSGameplayCueNotify_Static.generated.h"



/**
 * Base static gameplay cue class
 */
UCLASS()
class ABILITYSYSTEMSETUP_API UASSGameplayCueNotify_Static : public UGameplayCueNotify_Static
{
    GENERATED_BODY()

public:
    UASSGameplayCueNotify_Static();

};
