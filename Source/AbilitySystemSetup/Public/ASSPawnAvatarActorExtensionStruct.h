// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ASSAvatarActorExtensionStruct.h"

#include "ASSPawnAvatarActorExtensionStruct.generated.h"

struct FGameplayTag;
class UInputAction;
class UInputComponent;
class UEnhancedInputComponent;

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
 *            - Call OnAvatarActorDestroyPlayerInputComponent() after the Super call at the end of the function
 *
 *
 * Recomended callsites for pawn avatar actor with ASC on the player state:
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
 *        See FASSAvatarActorExtensionStruct for setup
 */
USTRUCT()
struct ABILITYSYSTEMSETUP_API FASSPawnAvatarActorExtensionStruct : public FASSAvatarActorExtensionStruct
{
    GENERATED_BODY()

public:
    
    /**
     * @brief Should be called by the avatar actor on end play (before destruction).
     */
    void OnAvatarActorBeginDestroy();

public: // Extension functions for avatar actor to call

    //  BEGIN FASSAvatarActorExtensionStruct interface
    virtual void InitializeAbilitySystemComponent(UAbilitySystemComponent& inASC, AActor& avatarActor);
    virtual void UninitializeAbilitySystemComponent(AActor& avatarActor) override;
    //  END   FASSAvatarActorExtensionStruct interface

    /**
     * @brief Should be called by the pawn avatar actor when its Controller changes i.e. PossessedBy(), UnPossessed(), and OnRep_Controller().
     */
    void OnAvatarActorControllerChanged(AActor& avatarActor);

    /**
     * @brief Should be called at the end of owning avatar actor's SetupPlayerInputComponent() event.
     * @param inPlayerInputComponent: The player input component to set up GAS input events for.
     */
    void OnAvatarActorSetupPlayerInputComponent(UInputComponent& playerInputComponent, AActor& avatarActor);

    /**
     * @brief Should be called at the end of avater actor's DestroyPlayerInputComponent() event.
     */
    void OnAvatarActorDestroyPlayerInputComponent();

protected:

    /**
     * @brief Hooks up and tracks binding handles for the given input action. This will bind to all ETriggerEvents (currently just ETriggerEvent::Started and ETriggerEvent::Completed).
     * @param inPlayerEnhancedInputComponent: The enhanced input component to bind our events for.
     * @param inInputAction: The input action asset we want to hook up.
     * @param inInputActionTag: The identification tag for this input action. Passed through to the input event as a payload parameter.
     */
    void BindInputAction(UEnhancedInputComponent& inPlayerEnhancedInputComponent, const UInputAction& inInputAction, const FGameplayTag& inInputActionTag, const AActor& avatarActor);

    /**
     * @brief Unhooks and untracks the binding handles for the given input action.
     * @param inPlayerEnhancedInputComponent: The enhanced input component to unbind our events for.
     * @param inInputAction: The input action asset we want to unhook.
     * @param inInputActionTag: The identification tag for this input action.
     */
    void UnBindInputAction(UEnhancedInputComponent& inPlayerEnhancedInputComponent, const UInputAction& inInputAction, const FGameplayTag& inInputActionTag, const AActor& avatarActor);

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

    FDelegateHandle OnInputActionAddedDelegateHandle;
    FDelegateHandle OnInputActionRemovedDelegateHandle;

    // Store our binding handles so that, if a plugin input action gets removed during the game we can unbind from it.
    TMap<TWeakObjectPtr<const UInputAction>, uint32> PressedInputActionBindingHandles;
    TMap<TWeakObjectPtr<const UInputAction>, uint32> ReleasedInputActionBindingHandles;
};
