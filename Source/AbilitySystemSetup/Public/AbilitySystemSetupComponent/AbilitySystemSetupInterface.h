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
 * Provides events for:
 *		- Adding Attribute Sets to the ASC
 *		- Giving starting Abilities to the ASC
 * 
 * @SEE "AbilitySystemSetupInterface.cpp" for example implementations of these events!
 * 
 */
class ABILITYSYSTEMSETUP_API IAbilitySystemSetupInterface
{
	GENERATED_BODY()

	friend class UAbilitySystemSetupComponent;
public:
	virtual UAbilitySystemSetupComponent* GetAbilitySystemSetupComponent() const = 0;

protected:
	/**
	 * The earliest place you can add Attribute Sets. Remember to use UObject::Rename() so that we can remove them on UnPossessed.
	 * NOTE: Server only event.
	 * NOTE: You probably do not need this event - just use the UAbilitySystemSetupComponent::StartingAttributeSets.
	 * NOTE: See example implementation of this event in "AbilitySystemSetupInterface.cpp"
	 */
	virtual void AddAttributeSets() { }
	/**
	 * The earliest place you can give Abilities. This is meant for giving Abilities and assigning Spec Handles to them.
	 * NOTE: Server only event.
	 * NOTE: See example implementation of this event in "AbilitySystemSetupInterface.cpp"
	 */
	virtual void GiveStartingAbilities() { }

};
