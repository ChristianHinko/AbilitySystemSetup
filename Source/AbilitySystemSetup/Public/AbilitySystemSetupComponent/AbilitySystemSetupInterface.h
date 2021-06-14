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
 * Contains events for:
 *		- Creating attribute sets for your ASC
 *		- Registering them with your ASC
 *		- Granting starting abilities for your ASC by spec handle
 * 
 * @SEE "AbilitySystemSetupInterface.cpp" for example implementations of these events!
 * 
 */
class ABILITYSYSTEMSETUP_API IAbilitySystemSetupInterface
{
	GENERATED_BODY()

	friend class UAbilitySystemSetupComponent;
public:
	virtual UAbilitySystemSetupComponent* GetAbilitySystemSetup() const = 0;

protected:
	/**
	 * Called on the server and client. Override this to create new Attribute Sets using NewObject().
	 * Note: See example implementation of this event in "AbilitySystemSetupInterface.cpp"
	 */
	virtual void CreateAttributeSets() = 0;
	/**
	 * Called on the server and client. Override this to register your created Attribute Sets with the ASC using AddAttributeSetSubobject().
	 * Note: See example implementation of this event in "AbilitySystemSetupInterface.cpp"
	 */
	virtual void RegisterAttributeSets() = 0;
	/**
	 * Called on server only. This is the earliest place you can grant Abilities. This is meant for granting Abilities and assigning them with Spec Handles.
	 * Note: See example implementation of this event in "AbilitySystemSetupInterface.cpp"
	 */
	virtual void GrantStartingAbilities() = 0;

};
