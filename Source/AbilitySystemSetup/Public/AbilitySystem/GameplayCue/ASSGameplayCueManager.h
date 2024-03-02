// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueManager.h"

#include "ASSGameplayCueManager.generated.h"



/**
 * Our base GameplayCueManager
 */
UCLASS()
class ABILITYSYSTEMSETUP_API UASSGameplayCueManager : public UGameplayCueManager
{
    GENERATED_BODY()

public:
    virtual bool ShouldAsyncLoadRuntimeObjectLibraries() const override;

    virtual void FlushPendingCues() override;

};
