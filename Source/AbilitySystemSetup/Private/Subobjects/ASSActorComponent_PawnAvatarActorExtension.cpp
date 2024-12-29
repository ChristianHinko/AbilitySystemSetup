// Fill out your copyright notice in the Description page of Project Settings.


#include "Subobjects/ASSActorComponent_PawnAvatarActorExtension.h"

#include "AbilitySystemComponent.h"
#include "Subsystems/ISEngineSubsystem_ObjectReferenceLibrary.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "AbilitySystem/ASSAbilitySystemBlueprintLibrary.h"
#include "GCUtils_Log.h"

DEFINE_LOG_CATEGORY(LogASSPawnAvatarActorExtensionComponent)

UASSActorComponent_PawnAvatarActorExtension::UASSActorComponent_PawnAvatarActorExtension(const FObjectInitializer& inObjectInitializer)
    : Super(inObjectInitializer)
{

}

void UASSActorComponent_PawnAvatarActorExtension::OnRegister()
{
    Super::OnRegister();


#if !NO_LOGGING || DO_CHECK
    if (GetOwner()->IsA<APawn>() == false)
    {
        GC_LOG_STR_UOBJECT(this,
            LogASSPawnAvatarActorExtensionComponent,
            Error,
            WriteToString<256>(TEXT("Incorrect usage of this component. Component owner ["), GCUtils::String::GetUObjectNameSafe(GetOwner()), TEXT("] is not a Pawn."))
            );
        check(0);
    }
#endif // !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
}

void UASSActorComponent_PawnAvatarActorExtension::UninitializeAbilitySystemComponent()
{
    Super::UninitializeAbilitySystemComponent();

    checkf(GetOwner(), TEXT("Can't remove our bindings from owner's input component as owner is NULL."));
    UEnhancedInputComponent* playerEnhancedInputComponent = Cast<UEnhancedInputComponent>(GetOwner()->InputComponent);
    if (IsValid(playerEnhancedInputComponent))
    {
        UnBindAllInputActions(*playerEnhancedInputComponent);
    }
}

void UASSActorComponent_PawnAvatarActorExtension::OnOwnerControllerChanged()
{
    if (UAbilitySystemComponent* asc = AbilitySystemComponent.Get())
    {
        if (asc->GetAvatarActor() == GetOwner())
        {
            check(asc->AbilityActorInfo->OwnerActor == asc->GetOwnerActor()); // The owner of the ASC matches the OwnerActor from the ActorInfo

            asc->RefreshAbilityActorInfo(); // Update our ActorInfo's PlayerController
        }
#if !NO_LOGGING
        else
        {
            GC_LOG_STR_UOBJECT(this,
                LogASSPawnAvatarActorExtensionComponent,
                Error,
                GCUtils::Materialize(TStringBuilder<256>()) << TEXT("Tried `") << GET_FUNCTION_NAME_CHECKED(UAbilitySystemComponent, RefreshAbilityActorInfo) << TEXT("()`, but the actor with this component was not the avatar actor.")
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
void UASSActorComponent_PawnAvatarActorExtension::OnOwnerSetupPlayerInputComponent(UInputComponent& inPlayerInputComponent)
{
    // Bind to all Input Actions so we can tell the ability system when ability inputs have been pressed/released
    check(GEngine);
    UISEngineSubsystem_ObjectReferenceLibrary* inputSetupObjectReferenceLibrary = GEngine->GetEngineSubsystem<UISEngineSubsystem_ObjectReferenceLibrary>();
    if (!ensureAlways(IsValid(inputSetupObjectReferenceLibrary)))
    {
        return;
    }

    // Bind to all known Input Actions
    UEnhancedInputComponent* playerEnhancedInputComponent = Cast<UEnhancedInputComponent>(&inPlayerInputComponent);
    if (ensureAlways(IsValid(playerEnhancedInputComponent)))
    {
        const TMap<FGameplayTag, TWeakObjectPtr<const UInputAction>>& inputActionTagMap = inputSetupObjectReferenceLibrary->GetAllInputActions();
        for (const TPair<FGameplayTag, TWeakObjectPtr<const UInputAction>>& tagInputActionPair : inputActionTagMap)
        {
            const UInputAction* inputAction = tagInputActionPair.Value.Get();
            if (ensureAlways(IsValid(inputAction)))
            {
                BindInputAction(*playerEnhancedInputComponent, *inputAction, tagInputActionPair.Key);
            }
        }
    }


    // When Input Actions are added during the game, bind to them.
    inputSetupObjectReferenceLibrary->OnInputActionAdded.AddWeakLambda(this,
            [this](const TPair<FGameplayTag, TWeakObjectPtr<const UInputAction>>& inTagInputActionPair)
            {
                check(GetOwner());
                UEnhancedInputComponent* playerEnhancedInputComponent = Cast<UEnhancedInputComponent>(GetOwner()->InputComponent);
                if (ensureAlways(IsValid(playerEnhancedInputComponent)))
                {
                    const UInputAction* inputAction = inTagInputActionPair.Value.Get();
                    if (IsValid(inputAction))
                    {
                        BindInputAction(*playerEnhancedInputComponent, *inputAction, inTagInputActionPair.Key);
                    }
                }
            }
        );

    // When Input Actions are removed during the game, unbind from them.
    inputSetupObjectReferenceLibrary->OnInputActionRemoved.AddWeakLambda(this,
            [this](const TPair<FGameplayTag, TWeakObjectPtr<const UInputAction>>& inTagInputActionPair)
            {
                check(GetOwner());
                UEnhancedInputComponent* playerEnhancedInputComponent = Cast<UEnhancedInputComponent>(GetOwner()->InputComponent);
                if (IsValid(playerEnhancedInputComponent))
                {
                    const UInputAction* inputAction = inTagInputActionPair.Value.Get();
                    if (IsValid(inputAction))
                    {
                        UnBindInputAction(*playerEnhancedInputComponent, *inputAction, inTagInputActionPair.Key);
                    }
                }
            }
        );
}

void UASSActorComponent_PawnAvatarActorExtension::OnOwnerDestroyPlayerInputComponent()
{
    // The InputComponent is destroyed which means all of its bindings are destroyed too. So update our handle lists.
    PressedInputActionBindingHandles.Empty();
    ReleasedInputActionBindingHandles.Empty();
}

void UASSActorComponent_PawnAvatarActorExtension::BindInputAction(UEnhancedInputComponent& inPlayerEnhancedInputComponent, const UInputAction& inInputAction, const FGameplayTag& inInputActionTag)
{
    const uint32 pressedBindingHandle = inPlayerEnhancedInputComponent.BindAction(&inInputAction, ETriggerEvent::Started, this, &ThisClass::OnPressedInputAction, inInputActionTag).GetHandle();
    const uint32 releasedBindingHandle = inPlayerEnhancedInputComponent.BindAction(&inInputAction, ETriggerEvent::Completed, this, &ThisClass::OnReleasedInputAction, inInputActionTag).GetHandle();

    PressedInputActionBindingHandles.Add(&inInputAction, pressedBindingHandle);
    ReleasedInputActionBindingHandles.Add(&inInputAction, releasedBindingHandle);

    GC_LOG_STR_UOBJECT(this,
        LogASSPawnAvatarActorExtensionComponent,
        Log,
        GCUtils::Materialize(TStringBuilder<256>()) << TEXT("Binding to newly added input action [") << inInputActionTag.GetTagName() << TEXT("] for calling GAS input events.")
        );
}

void UASSActorComponent_PawnAvatarActorExtension::UnBindInputAction(UEnhancedInputComponent& inPlayerEnhancedInputComponent, const UInputAction& inInputAction, const FGameplayTag& inInputActionTag)
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

    GC_LOG_STR_UOBJECT(this,
        LogASSPawnAvatarActorExtensionComponent,
        Log,
        GCUtils::Materialize(TStringBuilder<256>()) << TEXT("Input action [") << inInputActionTag.GetTagName() << TEXT("] removed. Stopping the calling of GAS input events.")
        );
}

void UASSActorComponent_PawnAvatarActorExtension::UnBindAllInputActions(UEnhancedInputComponent& inPlayerEnhancedInputComponent)
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

void UASSActorComponent_PawnAvatarActorExtension::OnPressedInputAction(const FGameplayTag inInputActionTag)
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
            UASSAbilitySystemBlueprintLibrary::AbilityLocalInputPressedForSpec(asc, *GameplayAbilitySpecPtr, bAllowAbilityActivation);
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
void UASSActorComponent_PawnAvatarActorExtension::OnReleasedInputAction(const FGameplayTag inInputActionTag)
{
    if (UAbilitySystemComponent* asc = AbilitySystemComponent.Get())
    {
        TArray<FGameplayAbilitySpec*> GameplayAbilitySpecs;
        asc->GetActivatableGameplayAbilitySpecsByAllMatchingTags(inInputActionTag.GetSingleTagContainer(), GameplayAbilitySpecs, false);
        for (FGameplayAbilitySpec* GameplayAbilitySpecPtr : GameplayAbilitySpecs)
        {
            // Tell ASC about ability input released.
            UASSAbilitySystemBlueprintLibrary::AbilityLocalInputReleasedForSpec(asc, *GameplayAbilitySpecPtr);
        }
    }
}
//  END Input setup
