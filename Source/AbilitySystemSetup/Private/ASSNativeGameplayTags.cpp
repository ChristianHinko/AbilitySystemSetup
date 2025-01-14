// Fill out your copyright notice in the Description page of Project Settings.

#include "ASSNativeGameplayTags.h"

namespace ASSNativeGameplayTags
{
    // Abilities.
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Type_Passive, TEXT("Ability.Type.Passive"), "Enables passive ability activation for an ability on given.");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Type_DisableAutoActivationFromInput, TEXT("Ability.Type.DisableAutoActivationFromInput"), "Disables automatic activation of the ability from input. Input events are still available for the ability.");

    // Input actions.
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputAction_ConfirmTarget, TEXT("InputAction.ConfirmTarget"), "");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputAction_CancelTarget, TEXT("InputAction.CancelTarget"), "");
}
