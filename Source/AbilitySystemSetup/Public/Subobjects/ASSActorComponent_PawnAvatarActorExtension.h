// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subobjects/ASSActorComponent_AvatarActorExtension.h"

#include "ASSActorComponent_PawnAvatarActorExtension.generated.h"


struct FGameplayTag;
class UInputAction;



/**
 * Pawn version that handles updating GAS during controller changes and sets up GAS input events.
 * 
 * ----------------------------------
 *				Setup
 * ----------------------------------
 * 
 * Required callsites:
 *		PossessedBy()
 *			- Call HandleControllerChanged() after the Super call
 *		UnPossessed()
 *			- Call HandleControllerChanged() after the Super call
 *		OnRep_Controller()
 *			- Call HandleControllerChanged() after the Super call
 *		SetupPlayerInputComponent()
 *			- Call SetupPlayerInputComponent() after the Super call at the end of the function
 *		DestroyPlayerInputComponent()
 *			- Call DestroyPlayerInputComponent() after the Super call at the end of the function
 * 
 * 
 * Recomended callsites for Pawn with ASC on the Player State:
 *		PossessedBy()
 *			- Call InitializeAbilitySystemComponent() after the Super call
 *		UnPossessed()
 *			- Call UninitializeAbilitySystemComponent() before the Super call
 *		OnRep_PlayerState()
 *			- If IsPlayerControlled()
 *				- Call InitializeAbilitySystemComponent() after the Super call IF we have a valid Player State
 *				- Call UninitializeAbilitySystemComponent() after the Super call IF we do not have a valid Player State
 * 
 * Recomended callsites for Pawn with ASC on itself:
 *		See ASSActorComponent_AvatarActorExtension for setup
 */
UCLASS()
class ABILITYSYSTEMSETUP_API UASSActorComponent_PawnAvatarActorExtension : public UASSActorComponent_AvatarActorExtension
{
	GENERATED_BODY()

public:
	UASSActorComponent_PawnAvatarActorExtension(const FObjectInitializer& ObjectInitializer);


	/** Called by the owning pawn when the Pawn's Controller changes i.e. PossessedBy(), UnPossessed(), and OnRep_Controller() */
	void HandleControllerChanged();
	/** Called at the end of your Pawn's SetupPlayerInputComponent() event */
	void SetupPlayerInputComponent(UInputComponent* InPlayerInputComponent);
	/** Called at the end of your Pawn's DestroyPlayerInputComponent() event */
	void DestroyPlayerInputComponent();

	virtual void UninitializeAbilitySystemComponent() override;

protected:
	//  BEGIN UActorComponent interface
	virtual void OnRegister() override;
	//  END UActorComponent interface

	void OnPressedInputAction(const FGameplayTag InInputActionTag) const;
	void OnReleasedInputAction(const FGameplayTag InInputActionTag) const;

private:
	// Store our binding handles so that, if a plugin Input Action gets removed during the game, we can unbind from it.
	TMap<TWeakObjectPtr<const UInputAction>, uint32> PressedInputActionBindingHandles;
	TMap<TWeakObjectPtr<const UInputAction>, uint32> ReleasedInputActionBindingHandles;
};
