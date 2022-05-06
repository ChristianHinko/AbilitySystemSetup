// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystemSetup.h"



#define LOCTEXT_NAMESPACE "FAbilitySystemSetupModule"

void FAbilitySystemSetupModule::StartupModule()
{
	Super::StartupModule();


	UE_LOG(LogAbilitySystemSetupModule, Verbose, TEXT("%s module start up!"), *FString(ABILITYSYSTEMSETUP_MODULE_NAME))
}

void FAbilitySystemSetupModule::ShutdownModule()
{
	Super::ShutdownModule();


	UE_LOG(LogAbilitySystemSetupModule, Verbose, TEXT("%s module shutting down."), *FString(ABILITYSYSTEMSETUP_MODULE_NAME))
}

#undef LOCTEXT_NAMESPACE


IMPLEMENT_MODULE(FAbilitySystemSetupModule, AbilitySystemSetup)
