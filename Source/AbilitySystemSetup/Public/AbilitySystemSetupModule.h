// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

//#include "AbilitySystemSetup.generated.h" // if we ever need reflection in here



#define ABILITYSYSTEMSETUP_MODULE_NAME TEXT("AbilitySystemSetup")

/**
 * 
 */
class FAbilitySystemSetupModule : public FDefaultModuleImpl
{
public:
	virtual bool IsGameModule() const override
	{
		return false;
	}

	static inline FAbilitySystemSetupModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FAbilitySystemSetupModule>(ABILITYSYSTEMSETUP_MODULE_NAME);
	}
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded(ABILITYSYSTEMSETUP_MODULE_NAME);
	}

protected:
	//  BEGIN IModuleInterface Interface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//  END IModuleInterface Interface

};
