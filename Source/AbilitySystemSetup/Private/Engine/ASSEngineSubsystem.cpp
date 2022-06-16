// Fill out your copyright notice in the Description page of Project Settings.


#include "Engine/ASSEngineSubsystem.h"

#include "AbilitySystemGlobals.h"



void UASSEngineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);


	UAbilitySystemGlobals& AbilitySystemGlobals = UAbilitySystemGlobals::Get();

	if (AbilitySystemGlobals.IsAbilitySystemGlobalsInitialized())
	{
		// Idk when this would happen but just in case
		UE_LOG(LogAbilitySystemSetup, Warning, TEXT("%s(): AbilitySystemGlobals already Initted for some reason - idk when this would happen"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	// Should be called once as part of project setup to load global data tables and tags
	AbilitySystemGlobals.InitGlobalData();
}
