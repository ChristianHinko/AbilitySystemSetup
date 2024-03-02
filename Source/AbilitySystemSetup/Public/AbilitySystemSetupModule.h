// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

//#include "AbilitySystemSetup.generated.h" // uncomment if we ever need reflection here



/**
 *
 */
class FAbilitySystemSetupModule : public FDefaultModuleImpl
{
protected:
    //  BEGIN IModuleInterface Interface
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    //  END IModuleInterface Interface
};
