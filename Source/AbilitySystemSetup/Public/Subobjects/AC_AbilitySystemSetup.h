// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilitySystem/Types/AbilitySet.h"

#include "AC_AbilitySystemSetup.generated.h"


class UAbilitySystemComponent;



DECLARE_MULTICAST_DELEGATE_OneParam(FAbilitySystemSetupDelegate, UAbilitySystemComponent* const);


/**
 * Provides common GAS initialization/uninitialization logic with AbilitySets granted while initialized.
 * This component is to be used by avatar actors only.
 * 
 * For initialization, it sets us up as the AvatarActor for the ASC and grants AbilitySets (allowing you to choose 
 * starting Abilities, Effects, and Attribute Sets in BP). For uninitialization it ungrants the granted AbilitySets, 
 * gives external sources an opportunity to remove Loose Gameplay Tags (this is the only manual cleanup),
 * and disassociates us from the ASC.
 * 
 * This component does not automate anything. You have to manually call on provided functions for anything to happen......
 * 
 * 
 * ----------------------------------
 *				Setup
 * ----------------------------------
 * 
 * Required for all Pawns:
 *		- PossessedBy()
 *			- Call HandleControllerChanged() after the Super call
 *		- UnPossessed()
 *			- Call HandleControllerChanged() after the Super call
 *		- OnRep_Controller()
 *			- Call HandleControllerChanged() after the Super call
 *		- SetupPlayerInputComponent()
 *			- Call SetupPlayerInputComponent() after the Super call at the end of the function
 * 
 * 
 * Recomended places for initialization and uninitialization:
 *		Pawn with ASC on the Player State:
 *			- PossessedBy()
 *				- Call InitializeAbilitySystemComponent() after the Super call
 *			- UnPossessed()
 *				- Call UninitializeAbilitySystemComponent() before the Super call
 *			- OnRep_PlayerState()
 *				- If IsPlayerControlled()
 *					- Call InitializeAbilitySystemComponent() after the Super call IF we have a valid Player State
 *					- Call UninitializeAbilitySystemComponent() after the Super call IF we do not have a valid Player State
 *		
 *		Pawn with ASC on itself:
 *			- PostInitializeComponents()
 *				- Call InitializeAbilitySystemComponent() after the Super call
 *			- EndPlay()
 *				- Call UninitializeAbilitySystemComponent() before the Super call
 *		
 *		Actor with ASC:
 *			- PostInitializeComponents()
 *				- Call InitializeAbilitySystemComponent() after the Super call
 *			- EndPlay()
 *				- Call UninitializeAbilitySystemComponent() before the Super call
 */
UCLASS(ClassGroup=(AbilitySystemSetup), meta=(BlueprintSpawnableComponent))
class ABILITYSYSTEMSETUP_API UAC_AbilitySystemSetup : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UAC_AbilitySystemSetup(const FObjectInitializer& ObjectInitializer);


	// Ability sets to grant to this pawn's ability system.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySets")
		TArray<TSubclassOf<UAbilitySet>> AbilitySets;


	// ----- Functions provided for owner -----
	/** Sets the Avatar Actor with the ASC */
	void InitializeAbilitySystemComponent(UAbilitySystemComponent* ASC);
	/** Clears the Avatar Actor from the ASC */
	void UninitializeAbilitySystemComponent();
	/** Called by the owning pawn when the Pawn's Controller changes i.e. PossessedBy(), UnPossessed(), and OnRep_Controller() */
	void HandleControllerChanged();
	/** Called at the end of your Pawn's SetupPlayerInputComponent() event */
	void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent);



	/** Broadcasted when the Ability System is set up and ready to go */
	FAbilitySystemSetupDelegate OnInitializeAbilitySystemComponentDelegate;
	/** Server and client event for removing all Loose AvatarActor-related Tags. */
	FAbilitySystemSetupDelegate RemoveLooseAvatarRelatedTagsDelegate;

protected:
	// BEGIN UActorComponent interface
	virtual void OnRegister() override;
	// END UActorComponent interface

private:
	/** Makes the input events work for GAS */
	void BindAbilitySystemInput(UInputComponent* InputComponent);
	/** Broadcasts event to allow external sources to cleanup any Loose Gameplay Tags they were managing */
	void RemoveLooseAvatarRelatedTags();


	
	// ----- Internal members -----
	/** Abilities, Active Effects, and Attribute Sets to keep track of so we can clean them up from our ASC on UnPossess */
	TArray<FAbilitySetGrantedHandles> GrantHandles;
	/** Indicates that the list of AbilitySets has been granted */
	uint8 bGrantedAbilitySets : 1;
	/** Indicates that input has already been binded with the Ability System */
	uint8 bAbilitySystemInputBinded : 1;
	UPROPERTY()
		TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
};
