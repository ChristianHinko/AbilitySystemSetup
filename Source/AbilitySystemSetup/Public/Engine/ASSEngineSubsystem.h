// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"

#include "ASSEngineSubsystem.generated.h"



/**
 *
 */
UCLASS()
class ABILITYSYSTEMSETUP_API UASSEngineSubsystem : public UEngineSubsystem
{
    GENERATED_BODY()

public:
    //  BEGIN USubsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    //  END USubsystem Interface
};
