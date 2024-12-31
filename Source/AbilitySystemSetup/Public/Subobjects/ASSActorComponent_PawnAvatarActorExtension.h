// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subobjects/ASSActorComponent_AvatarActorExtension.h"

#include "ASSActorComponent_PawnAvatarActorExtension.generated.h"

struct FGameplayTag;
class UInputAction;
class UInputComponent;
class UEnhancedInputComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogASSPawnAvatarActorExtensionComponent, Log, All)

/**
 * Pawn version that handles updating GAS during controller changes and sets up GAS input events.
 *
 * ----------------------------------
 *                Setup
 * ----------------------------------
 *
 * Required callsites:
 *        PossessedBy()
 *            - Call OnOwnerControllerChanged() after the Super call
 *        UnPossessed()
 *            - Call OnOwnerControllerChanged() after the Super call
 *        OnRep_Controller()
 *            - Call OnOwnerControllerChanged() after the Super call
 *        SetupPlayerInputComponent()
 *            - Call OnOwnerSetupPlayerInputComponent() after the Super call at the end of the function
 *        DestroyPlayerInputComponent()
 *            - Call OnOwnerDestroyPlayerInputComponent() after the Super call at the end of the function
 *
 *
 * Recomended callsites for Pawn with ASC on the Player State:
 *        PossessedBy()
 *            - Call InitializeAbilitySystemComponent() after the Super call
 *        UnPossessed()
 *            - Call UninitializeAbilitySystemComponent() before the Super call
 *        OnRep_PlayerState()
 *            - If IsPlayerControlled()
 *                - Call InitializeAbilitySystemComponent() after the Super call IF we have a valid Player State
 *                - Call UninitializeAbilitySystemComponent() after the Super call IF we do not have a valid Player State
 *
 * Recomended callsites for Pawn with ASC on itself:
 *        See ASSActorComponent_AvatarActorExtension for setup
 */
UCLASS()
class ABILITYSYSTEMSETUP_API UASSActorComponent_PawnAvatarActorExtension : public UASSActorComponent_AvatarActorExtension
{
    GENERATED_BODY()

public:
    UASSActorComponent_PawnAvatarActorExtension(const FObjectInitializer& inObjectInitializer);

protected:
    //  BEGIN UActorComponent interface
    virtual void OnRegister() override;
    //  END   UActorComponent interface

public: // Extension functions for owner to call

    //  BEGIN UASSActorComponent_AvatarActorExtension interface
    virtual void UninitializeAbilitySystemComponent() override;
    //  END   UASSActorComponent_AvatarActorExtension interface

    /**
     * @brief Should be called by the owning pawn when the Pawn's Controller changes i.e. PossessedBy(), UnPossessed(), and OnRep_Controller().
     */
    void OnOwnerControllerChanged();

    /**
     * @brief Should be called at the end of owning Pawn's SetupPlayerInputComponent() event.
     * @param inPlayerInputComponent: The player input component to set up GAS input events for.
     */
    void OnOwnerSetupPlayerInputComponent(UInputComponent& inPlayerInputComponent);

    /**
     * @brief Should be called at the end of owning Pawn's DestroyPlayerInputComponent() event.
     */
    void OnOwnerDestroyPlayerInputComponent();

protected:

    /**
     * @brief Hooks up and tracks binding handles for the given input action. This will bind to all ETriggerEvents (currently just ETriggerEvent::Started and ETriggerEvent::Completed).
     * @param inPlayerEnhancedInputComponent: The enhanced input component to bind our events for.
     * @param inInputAction: The input action asset we want to hook up.
     * @param inInputActionTag: The identification tag for this input action. Passed through to the input event as a payload parameter.
     */
    void BindInputAction(UEnhancedInputComponent& inPlayerEnhancedInputComponent, const UInputAction& inInputAction, const FGameplayTag& inInputActionTag);

    /**
     * @brief Unhooks and untracks the binding handles for the given input action.
     * @param inPlayerEnhancedInputComponent: The enhanced input component to unbind our events for.
     * @param inInputAction: The input action asset we want to unhook.
     * @param inInputActionTag: The identification tag for this input action.
     */
    void UnBindInputAction(UEnhancedInputComponent& inPlayerEnhancedInputComponent, const UInputAction& inInputAction, const FGameplayTag& inInputActionTag);

    /**
     * @brief Unhooks and untracks all binding handles for the given enhanced input component.
     * @param inPlayerEnhancedInputComponent: The enhanced input component we unbind all our events from.
     */
    void UnBindAllInputActions(UEnhancedInputComponent& inPlayerEnhancedInputComponent);

    /**
     * @brief The callback for all pressed input events.
     * @param inInputActionTag: The tag used to identify the pressed input action.
     */
    void OnPressedInputAction(const FGameplayTag inInputActionTag);

    /**
     * @brief The callback for all released input events.
     * @param inInputActionTag: The tag used to identify the released input action.
     */
    void OnReleasedInputAction(const FGameplayTag inInputActionTag);

private:
    // Store our binding handles so that, if a plugin input action gets removed during the game we can unbind from it.
    TMap<TWeakObjectPtr<const UInputAction>, uint32> PressedInputActionBindingHandles;
    TMap<TWeakObjectPtr<const UInputAction>, uint32> ReleasedInputActionBindingHandles;
};
