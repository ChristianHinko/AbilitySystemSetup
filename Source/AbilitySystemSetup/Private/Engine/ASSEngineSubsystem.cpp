// Fill out your copyright notice in the Description page of Project Settings.


#include "Engine/ASSEngineSubsystem.h"

#include "AbilitySystemGlobals.h"



void UASSEngineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);


	UAbilitySystemGlobals::Get().InitGlobalData();
}
