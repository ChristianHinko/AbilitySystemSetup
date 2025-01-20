// Fill out your copyright notice in the Description page of Project Settings.

#include "ActorComponents/ASSActorComponent_PawnAvatarActorExtension.h"

#include "AbilitySystemComponent.h"
#include "ISEngineSubsystem_InputActionAssetReferences.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "ASSUtils.h"
#include "GCUtils_Log.h"

DEFINE_LOG_CATEGORY_STATIC(LogASSPawnAvatarActorExtensionComponent, Log, All);

void FASSActorComponent_PawnAvatarActorExtension::OnAvatarActorBeginDestroy()
{
    UISEngineSubsystem_InputActionAssetReferences& inputActionAssetReferenceSubsystem = UISEngineSubsystem_InputActionAssetReferences::GetChecked(*GEngine);

    inputActionAssetReferenceSubsystem.OnInputActionAddedDelegate.Remove(OnInputActionAddedDelegateHandle);
    inputActionAssetReferenceSubsystem.OnInputActionRemovedDelegate.Remove(OnInputActionRemovedDelegateHandle);
}

void FASSActorComponent_PawnAvatarActorExtension::InitializeAbilitySystemComponent(UAbilitySystemComponent& inASC, AActor& avatarActor)
{
#if !NO_LOGGING || DO_CHECK
    if (avatarActor.IsA<APawn>() == false)
    {
        GC_LOG_STR_UOBJECT(&avatarActor,
            LogASSPawnAvatarActorExtensionComponent,
            Error,
            WriteToString<256>(TEXT("Incorrect usage of this extention struct. Avatar actor `"), avatarActor.GetFName().ToString(), TEXT("` is not a pawn."))
            );
        check(0);
    }
#endif // #if !NO_LOGGING || DO_CHECK

    Super::InitializeAbilitySystemComponent(inASC, avatarActor);
}

void FASSActorComponent_PawnAvatarActorExtension::UninitializeAbilitySystemComponent(AActor& avatarActor)
{
    Super::UninitializeAbilitySystemComponent(avatarActor);

    if (UEnhancedInputComponent* playerEnhancedInputComponent = Cast<UEnhancedInputComponent>(avatarActor.InputComponent))
    {
        UnBindAllInputActions(*playerEnhancedInputComponent);
    }
}

void FASSActorComponent_PawnAvatarActorExtension::OnAvatarActorControllerChanged(AActor& avatarActor)
{
    if (UAbilitySystemComponent* asc = AbilitySystemComponent.Get())
    {
        if (asc->GetAvatarActor() == &avatarActor)
        {
            check(asc->AbilityActorInfo->OwnerActor == asc->GetOwnerActor()); // The owner of the asc matches the owner actor from the actor info.

            asc->RefreshAbilityActorInfo(); // Update our the actor info's player controller.
        }
#if !NO_LOGGING
        else
        {
            GC_LOG_STR_UOBJECT(&avatarActor,
                LogASSPawnAvatarActorExtensionComponent,
                Error,
                GCUtils::Materialize(TStringBuilder<256>()) << TEXT("Tried `") << GET_FUNCTION_NAME_CHECKED(UAbilitySystemComponent, RefreshAbilityActorInfo) << TEXT("()`, but the passed in actor was not the avatar actor.")
                );
        }
#endif
    }
    else
    {
        // In the case of ASC being on the PlayerState, this is expected to hit on the client for initial possessions (Controller gets replicated before PlayerState)
    }
}

//  BEGIN Input setup
void FASSActorComponent_PawnAvatarActorExtension::OnAvatarActorSetupPlayerInputComponent(UInputComponent& playerInputComponent, AActor& avatarActor)
{
    // Bind to all input actions so we can inform the ability system when ability inputs have been pressed/released.
    UISEngineSubsystem_InputActionAssetReferences& inputActionAssetReferenceSubsystem = UISEngineSubsystem_InputActionAssetReferences::GetChecked(*GEngine);

    // Bind to all currently-known input actions.
    UEnhancedInputComponent* playerEnhancedInputComponent = Cast<UEnhancedInputComponent>(&playerInputComponent);
    if (ensureAlways(IsValid(playerEnhancedInputComponent)))
    {
        const TMap<FGameplayTag, TObjectPtr<const UInputAction>>& tagToInputActionMap = inputActionAssetReferenceSubsystem.GetAllInputActions();
        for (const TPair<FGameplayTag, TObjectPtr<const UInputAction>>& tagInputToActionPair : tagToInputActionMap)
        {
            const UInputAction* inputAction = tagInputToActionPair.Value;
            check(inputAction);

            BindInputAction(*playerEnhancedInputComponent, *inputAction, tagInputToActionPair.Key, avatarActor);
        }
    }

    static auto getEnhancedInputComponentFromActor =
        [](AActor& avatarActor) -> UEnhancedInputComponent*
        {
            ensure(avatarActor.InputComponent);
            UEnhancedInputComponent* playerEnhancedInputComponent = Cast<UEnhancedInputComponent>(avatarActor.InputComponent);
            if (!ensureMsgf(playerEnhancedInputComponent, TEXT("Currently we only support enhanced input component.")))
            {
                return playerEnhancedInputComponent;
            }

            return nullptr;
        };

    // NOTE: We capture the avatar actor as a reference. This is safe since we
    //       make sure to remove from the subsystem's delegate before the avatar
    //       actor is destroyed (from `EndPlay`).

    // When input actions are added during the game, bind to them.
    OnInputActionAddedDelegateHandle = inputActionAssetReferenceSubsystem.OnInputActionAddedDelegate.AddWeakLambda(&avatarActor,
            [this, &avatarActor](const FGameplayTag& tag, const UInputAction& inputAction)
            {
                if (UEnhancedInputComponent* enhancedInputComponent = getEnhancedInputComponentFromActor(avatarActor))
                {
                    BindInputAction(*enhancedInputComponent, inputAction, tag, avatarActor);
                }
            }
        );

    // When input actions are removed during the game, unbind from them.
    OnInputActionRemovedDelegateHandle = inputActionAssetReferenceSubsystem.OnInputActionRemovedDelegate.AddWeakLambda(&avatarActor,
            [this, &avatarActor](const FGameplayTag& tag, const UInputAction& inputAction)
            {
                if (UEnhancedInputComponent* enhancedInputComponent = getEnhancedInputComponentFromActor(avatarActor))
                {
                    UnBindInputAction(*enhancedInputComponent, inputAction, tag, avatarActor);
                }
            }
        );
}

void FASSActorComponent_PawnAvatarActorExtension::OnAvatarActorDestroyPlayerInputComponent()
{
    // The InputComponent is destroyed which means all of its bindings are destroyed too. So update our handle lists.
    PressedInputActionBindingHandles.Empty();
    ReleasedInputActionBindingHandles.Empty();
}

void FASSActorComponent_PawnAvatarActorExtension::BindInputAction(UEnhancedInputComponent& inPlayerEnhancedInputComponent, const UInputAction& inInputAction, const FGameplayTag& inInputActionTag, const AActor& avatarActor)
{
    const uint32 pressedBindingHandle = inPlayerEnhancedInputComponent.BindActionValueLambda(
        &inInputAction,
        ETriggerEvent::Started,
        [this, inInputActionTag](const FInputActionValue& inputActionValue)
        {
            OnPressedInputAction(inInputActionTag);
        }).GetHandle();

    const uint32 releasedBindingHandle = inPlayerEnhancedInputComponent.BindActionValueLambda(
        &inInputAction,
        ETriggerEvent::Completed,
        [this, inInputActionTag](const FInputActionValue& inputActionValue)
        {
            OnReleasedInputAction(inInputActionTag);
        }).GetHandle();

    PressedInputActionBindingHandles.Add(&inInputAction, pressedBindingHandle);
    ReleasedInputActionBindingHandles.Add(&inInputAction, releasedBindingHandle);

    GC_LOG_STR_UOBJECT(&avatarActor,
        LogASSPawnAvatarActorExtensionComponent,
        Log,
        GCUtils::Materialize(TStringBuilder<256>()) << TEXT("Binding to newly added input action `") << inInputActionTag.GetTagName() << TEXT("` for calling GAS input events.")
        );
}

void FASSActorComponent_PawnAvatarActorExtension::UnBindInputAction(UEnhancedInputComponent& inPlayerEnhancedInputComponent, const UInputAction& inInputAction, const FGameplayTag& inInputActionTag, const AActor& avatarActor)
{
    if (const uint32* pressedHandle = PressedInputActionBindingHandles.Find(&inInputAction))
    {
        inPlayerEnhancedInputComponent.RemoveBindingByHandle(*pressedHandle);
        PressedInputActionBindingHandles.Remove(&inInputAction);
    }
    if (const uint32* releasedHandle = ReleasedInputActionBindingHandles.Find(&inInputAction))
    {
        inPlayerEnhancedInputComponent.RemoveBindingByHandle(*releasedHandle);
        PressedInputActionBindingHandles.Remove(&inInputAction);
    }

    GC_LOG_STR_UOBJECT(&avatarActor,
        LogASSPawnAvatarActorExtensionComponent,
        Log,
        GCUtils::Materialize(TStringBuilder<256>()) << TEXT("Input action `") << inInputActionTag.GetTagName() << TEXT("` removed. Stopping the calling of GAS input events.")
        );
}

void FASSActorComponent_PawnAvatarActorExtension::UnBindAllInputActions(UEnhancedInputComponent& inPlayerEnhancedInputComponent)
{
    for (const TPair<TWeakObjectPtr<const UInputAction>, uint32>& pressedInputActionBindingHandle : PressedInputActionBindingHandles)
    {
        inPlayerEnhancedInputComponent.RemoveBindingByHandle(pressedInputActionBindingHandle.Value);
    }
    PressedInputActionBindingHandles.Empty();

    for (const TPair<TWeakObjectPtr<const UInputAction>, uint32>& releasedInputActionBindingHandle : ReleasedInputActionBindingHandles)
    {
        inPlayerEnhancedInputComponent.RemoveBindingByHandle(releasedInputActionBindingHandle.Value);
    }
    ReleasedInputActionBindingHandles.Empty();
}

void FASSActorComponent_PawnAvatarActorExtension::OnPressedInputAction(const FGameplayTag inInputActionTag)
{
    if (UAbilitySystemComponent* asc = AbilitySystemComponent.Get())
    {
        TArray<FGameplayAbilitySpec*> GameplayAbilitySpecs;
        asc->GetActivatableGameplayAbilitySpecsByAllMatchingTags(inInputActionTag.GetSingleTagContainer(), GameplayAbilitySpecs, false);
        for (FGameplayAbilitySpec* GameplayAbilitySpecPtr : GameplayAbilitySpecs)
        {
            // Tell ASC about ability input pressed.
            check(GameplayAbilitySpecPtr->Ability);
            const bool bAllowAbilityActivation = GameplayAbilitySpecPtr->Ability->GetAssetTags().HasTag(ASSNativeGameplayTags::Ability_Type_DisableAutoActivationFromInput) == false;
            ASSUtils::AbilityLocalInputPressedForSpec(asc, *GameplayAbilitySpecPtr, bAllowAbilityActivation);
        }

        if (inInputActionTag == ASSNativeGameplayTags::InputAction_ConfirmTarget)
        {
            // Tell ASC about Confirm pressed.
            asc->LocalInputConfirm();
        }
        if (inInputActionTag == ASSNativeGameplayTags::InputAction_CancelTarget)
        {
            // Tell ASC about Cancel pressed.
            asc->LocalInputCancel();
        }
    }
}

void FASSActorComponent_PawnAvatarActorExtension::OnReleasedInputAction(const FGameplayTag inInputActionTag)
{
    if (UAbilitySystemComponent* asc = AbilitySystemComponent.Get())
    {
        TArray<FGameplayAbilitySpec*> GameplayAbilitySpecs;
        asc->GetActivatableGameplayAbilitySpecsByAllMatchingTags(inInputActionTag.GetSingleTagContainer(), GameplayAbilitySpecs, false);
        for (FGameplayAbilitySpec* GameplayAbilitySpecPtr : GameplayAbilitySpecs)
        {
            // Tell ASC about ability input released.
            ASSUtils::AbilityLocalInputReleasedForSpec(asc, *GameplayAbilitySpecPtr);
        }
    }
}
//  END Input setup
