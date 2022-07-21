// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilitySystem/Types/ASSAbilitySet.h"

#include "ASSActorComponent_AbilitySystemSetup.generated.h"


class UAbilitySystemComponent;
struct FGameplayTag;
class UInputAction;



DECLARE_MULTICAST_DELEGATE_OneParam(FAbilitySystemSetupDelegate, UAbilitySystemComponent* const);


/**
 * Provides common GAS initialization/uninitialization logic with Ability Sets granted while initialized.
 * This component is to be used by avatar actors only.
 * 
 * For initialization, it sets us up as the AvatarActor for the ASC and grants Ability Sets (allowing you to choose 
 * starting Abilities, Effects, and Attribute Sets in BP). For uninitialization it ungrants the granted Ability Sets, 
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
 *		- DestroyPlayerInputComponent()
 *			- Call DestroyPlayerInputComponent() after the Super call at the end of the function
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
class ABILITYSYSTEMSETUP_API UASSActorComponent_AbilitySystemSetup : public UActorComponent
{
	GENERATED_BODY()

public:
	UASSActorComponent_AbilitySystemSetup(const FObjectInitializer& ObjectInitializer);


	// Ability sets to grant to this pawn's ability system.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySets")
		TArray<TSubclassOf<UASSAbilitySet>> AbilitySets;


	// ----- Functions provided for owner -----
	/** Sets the Avatar Actor with the ASC */
	void InitializeAbilitySystemComponent(UAbilitySystemComponent* InASC);
	/** Clears the Avatar Actor from the ASC */
	void UninitializeAbilitySystemComponent();
	/** Called by the owning pawn when the Pawn's Controller changes i.e. PossessedBy(), UnPossessed(), and OnRep_Controller() */
	void HandleControllerChanged();
	/** Called at the end of your Pawn's SetupPlayerInputComponent() event */
	void SetupPlayerInputComponent(UInputComponent* InPlayerInputComponent);
	/** Called at the end of your Pawn's DestroyPlayerInputComponent() event */
	void DestroyPlayerInputComponent();


	/** Broadcasted when the Ability System is set up and ready to go */
	FAbilitySystemSetupDelegate OnInitializeAbilitySystemComponentDelegate;
	/** Server and client event for removing all Loose AvatarActor-related Tags. */
	FAbilitySystemSetupDelegate RemoveLooseAvatarRelatedTagsDelegate;

	/** Are we initialized with an ASC */
	bool IsInitializedWithASC() const { return bInitialized; }

protected:
	//  BEGIN UActorComponent interface
	virtual void OnRegister() override;
	//  END UActorComponent interface

	void OnPressedInputAction(const FGameplayTag InInputActionTag) const;
	void OnReleasedInputAction(const FGameplayTag InInputActionTag) const;

private:
	/** Broadcasts event to allow external sources to cleanup any Loose Gameplay Tags they were managing */
	void RemoveLooseAvatarRelatedTags();



	// ----- Internal members -----
	/** The initialized ASC */
	UPROPERTY()
		TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	/** Abilities, Active Effects, and Attribute Sets to keep track of so we can clean them up from our ASC on UnPossess */
	TArray<FASSAbilitySetGrantedHandles> GrantedHandles;
	/** Indicates that the list of AbilitySets has been granted */
	uint8 bGrantedAbilitySets : 1;
	/** Indicates that we are initialized with an Ability System Component */
	uint8 bInitialized : 1;

	// Store our binding handles so that, if a plugin Input Action gets removed during the game, we can unbind from it.
	TMap<TWeakObjectPtr<const UInputAction>, uint32> PressedInputActionBindingHandles;
	TMap<TWeakObjectPtr<const UInputAction>, uint32> ReleasedInputActionBindingHandles;
};
