// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

/**
 * This module's native gameplay tags wrapped in a namespace.
 */
namespace ASSNativeGameplayTags
{
    // Abilities.
    ABILITYSYSTEMSETUP_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Type_Passive);
    ABILITYSYSTEMSETUP_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Type_DisableAutoActivationFromInput);

    // Input actions.
    ABILITYSYSTEMSETUP_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputAction_ConfirmTarget);
    ABILITYSYSTEMSETUP_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputAction_CancelTarget);
}
