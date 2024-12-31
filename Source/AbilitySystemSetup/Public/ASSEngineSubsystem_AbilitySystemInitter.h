// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"

#include "ASSEngineSubsystem_AbilitySystemInitter.generated.h"

/**
 * @brief Engine subsystem that automatically inits ability system global data on startup.
 */
UCLASS()
class ABILITYSYSTEMSETUP_API UASSEngineSubsystem_AbilitySystemInitter : public UEngineSubsystem
{
    GENERATED_BODY()

public:

    // ~ USubsystem overrides.
    virtual void Initialize(FSubsystemCollectionBase& inSubsystemCollection) override;
    // ~ USubsystem overrides.
};
