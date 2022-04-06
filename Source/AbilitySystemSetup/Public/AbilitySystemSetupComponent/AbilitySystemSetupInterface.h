// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "AbilitySystemSetupInterface.generated.h"



// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UAbilitySystemSetupInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Is made to be implemented by Actors with the UAbilitySystemSetupComponent.
 * 
 * Server events for giving starting Abilities and Attribute Sets via C++.
 * 
 * See AbilitySystemSetupInterface.cpp for example implementations of these events!
 */
class ABILITYSYSTEMSETUP_API IAbilitySystemSetupInterface
{
	GENERATED_BODY()

	friend class UAbilitySystemSetupComponent;
public:
	virtual UAbilitySystemSetupComponent* GetAbilitySystemSetupComponent() const = 0;

protected:
	/**
	 * Server only event for giving starting Attribute Sets via C++.
	 * NOTE: Remember to use UObject::Rename() so that we can remove them on UnPossessed.
	 * NOTE: See example implementation of this event in "AbilitySystemSetupInterface.cpp".
	 */
	virtual void AddAttributeSets() { }
	/**
	 * Server only event for giving starting abilities via C++.
	 * NOTE: See example implementation of this event in "AbilitySystemSetupInterface.cpp".
	 */
	virtual void GiveStartingAbilities() { }
};
