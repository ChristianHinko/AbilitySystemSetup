// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"



/**
 * This module's Native Gameplay Tags wrapped in a namespace.
 */
namespace ASSNativeGameplayTags
{
	// Ability
	ABILITYSYSTEMSETUP_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Type_DisableAutoActivationFromInput)

	// Input Actions
	ABILITYSYSTEMSETUP_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputAction_ConfirmTarget)
	ABILITYSYSTEMSETUP_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputAction_CancelTarget)
}
