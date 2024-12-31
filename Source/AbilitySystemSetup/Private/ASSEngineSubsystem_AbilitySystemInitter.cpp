// Fill out your copyright notice in the Description page of Project Settings.

#include "ASSEngineSubsystem_AbilitySystemInitter.h"

#include "AbilitySystemGlobals.h"
#include "GCUtils_Log.h"

DEFINE_LOG_CATEGORY_STATIC(LogASSEngineSubsystem_AbilitySystemInitter, Log, All);

void UASSEngineSubsystem_AbilitySystemInitter::Initialize(FSubsystemCollectionBase& inSubsystemCollection)
{
    Super::Initialize(inSubsystemCollection);

    GC_LOG_STR_UOBJECT(
        this,
        LogASSEngineSubsystem_AbilitySystemInitter,
        Verbose,
        TEXT("Subsystem initializing.")
        );

    // Make sure ability system globals is initted once as part of project setup to load global data tables and tags.
    UAbilitySystemGlobals& abilitySystemGlobals = UAbilitySystemGlobals::Get();
    if (abilitySystemGlobals.IsAbilitySystemGlobalsInitialized() == false)
    {
        GC_LOG_STR_UOBJECT(
            this,
            LogASSEngineSubsystem_AbilitySystemInitter,
            Log,
            TEXT("Initing global data for ability system globals.")
            );
        abilitySystemGlobals.InitGlobalData();
    }
}
