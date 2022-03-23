// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"



#if 0
/**
 * AbilityInputID enum =@REVIEW MARKER@=
 * 
 * Enum that makes GAS aware of which Abilities are binded to which input in your project settings. Recommended spot to include this would be in your game's custom PCH (Precompiled Header). This
 * is so that you don't always have to include the file your input enum is in when writing an Ability (since our workflow enforces setting an input enum value in constructor of each Ability).
 * 
 * The enums ``Unset`` at ``0`` and ``NoInput`` at ``1`` are MANDITORY in your enum so that the Ability can enforce good practice. We have a system so that if an Ability's Input ID is Unset, it will throw an assertion,
 * forcing you to give each Ability an Input ID. This is good practice since using AbilityInputIDs integrates stuff more into GAS.
 * 
 * Do not forget to update your enum whenever you modify the inputs in your DefaultInput.ini. They must match exactly.
 */
UENUM()
enum EAbilityInputID // NOTE: i would want to use an enum class here but there is no implicit conversion with them. And our base ASSGameplayAbility has to store AbilityInputID as an integer so explicitly casting would be a pain.
{
	/** This means the Ability implementor forgot to set an AbilityInputID in their Ability's constructor (``Unset`` is every Ability's default value) */
	Unset,
	/** This means the Ability is triggered without input (probably gameplay code activation instead) */
	NoInput,


	Run,
	Jump,
	Crouch,

	Interact,
	DropItem,

	PrimaryFire,
	SecondaryFire,
	Reload,

	FirstItem,
	SecondItem,
	ThirdItem,
	FourthItem,
	FifthItem,
	SwitchWeapon,
	NextItem,
	PreviousItem,

	Pause,
	ScoreSheet,


	// MAX
	MAX					UMETA(Hidden)
};
#endif
