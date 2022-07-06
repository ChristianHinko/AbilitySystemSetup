// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"



/**
 * Base AbilityInputID enum for your game to extend.
 * 
 * This base enum provides the enumerations ``Unset`` and ``NoInput``. This is to check if an Ability's Input ID is Unset and force you
 * to explicitly specify an input ID for every Ability.
 */
UENUM()
enum class EASSAbilityInputID : uint8
{
	/** This means the Ability implementor forgot to set an AbilityInputID in their Ability's constructor (``Unset`` is every Ability's default value) */
	Unset,
	/** This means the Ability is triggered without input (probably gameplay code activation instead) */
	NoInput,


	/** Use this value as your first enumeration in your game's enum (extending this enum) */
	MAX					UMETA(Hidden)
};
