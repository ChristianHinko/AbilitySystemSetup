// Fill out your copyright notice in the Description page of Project Settings.


#include "Subobjects/ASSActorComponent_PawnAvatarActorExtension.h"

#include "AbilitySystemComponent.h"
#include "Subsystems/ISEngineSubsystem_ObjectReferenceLibrary.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "InputTriggers.h"
#include "AbilitySystem/ASSAbilitySystemBlueprintLibrary.h"



UASSActorComponent_PawnAvatarActorExtension::UASSActorComponent_PawnAvatarActorExtension(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UASSActorComponent_PawnAvatarActorExtension::OnRegister()
{
	Super::OnRegister();


#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (GetOwner()->IsA<APawn>() == false)
	{
		UE_LOG(LogASSAvatarExtensionComponent, Error, TEXT("%s() Incorrect use of the pawn-specific %s. Component owner is not a Pawn: [%s]"), ANSI_TO_TCHAR(__FUNCTION__), TNameOf<UASSActorComponent_PawnAvatarActorExtension>::GetName(), *GetNameSafe(GetOwner()));
		check(0);
	}
#endif // !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
}

void UASSActorComponent_PawnAvatarActorExtension::OnOwnerControllerChanged()
{
	if (AbilitySystemComponent.IsValid())
	{
		if (AbilitySystemComponent->GetAvatarActor() == GetOwner())
		{
			check(AbilitySystemComponent->AbilityActorInfo->OwnerActor == AbilitySystemComponent->GetOwnerActor());	// the owner of the AbilitySystemComponent matches the OwnerActor from the ActorInfo

			AbilitySystemComponent->RefreshAbilityActorInfo(); // update our ActorInfo's PlayerController
		}
		else
		{
			UE_LOG(LogASSAvatarExtensionComponent, Error, TEXT("%s() Tried RefreshAbilityActorInfo(), but the actor with this component was not the avatar actor"), ANSI_TO_TCHAR(__FUNCTION__));
		}
	}
	else
	{
		// In the case of ASC being on the PlayerState, this is expected to hit on the client for initial possessions (Controller gets replicated before PlayerState)
	}
}

//  BEGIN Input setup
void UASSActorComponent_PawnAvatarActorExtension::OnOwnerSetupPlayerInputComponent(UInputComponent* InPlayerInputComponent)
{
	// Bind to all Input Actions so we can tell the ability system when ability inputs have been pressed/released
	UISEngineSubsystem_ObjectReferenceLibrary* ISObjectReferenceLibrary = GEngine->GetEngineSubsystem<UISEngineSubsystem_ObjectReferenceLibrary>();
	if (IsValid(ISObjectReferenceLibrary))
	{
		// Bind to all known Input Actions
		UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(InPlayerInputComponent);
		if (IsValid(PlayerEnhancedInputComponent))
		{
			const TMap<FGameplayTag, TWeakObjectPtr<const UInputAction>>& InputActionTagMap = ISObjectReferenceLibrary->GetAllInputActions();
			for (const TPair<FGameplayTag, TWeakObjectPtr<const UInputAction>>& TagInputActionPair : InputActionTagMap)
			{
				const UInputAction* InputAction = TagInputActionPair.Value.Get();
				if (IsValid(InputAction))
				{
					const uint32 PressedBindingHandle = PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Started, this, &ThisClass::OnPressedInputAction, TagInputActionPair.Key).GetHandle();
					const uint32 ReleasedBindingHandle = PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Completed, this, &ThisClass::OnReleasedInputAction, TagInputActionPair.Key).GetHandle();

					PressedInputActionBindingHandles.Add(InputAction, PressedBindingHandle);
					ReleasedInputActionBindingHandles.Add(InputAction, ReleasedBindingHandle);
				}
			}
		}


		// When Input Actions are added during the game, bind to them.
		ISObjectReferenceLibrary->OnInputActionAdded.AddWeakLambda(this,
				[this](const TPair<FGameplayTag, TWeakObjectPtr<const UInputAction>>& InTagInputActionPair)
				{
					UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(GetOwner()->InputComponent);
					if (IsValid(PlayerEnhancedInputComponent))
					{
						const UInputAction* InputAction = InTagInputActionPair.Value.Get();
						if (IsValid(InputAction))
						{
							const uint32 PressedBindingHandle = PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Started, this, &ThisClass::OnPressedInputAction, InTagInputActionPair.Key).GetHandle();
							const uint32 ReleasedBindingHandle = PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Completed, this, &ThisClass::OnReleasedInputAction, InTagInputActionPair.Key).GetHandle();

							PressedInputActionBindingHandles.Add(InputAction, PressedBindingHandle);
							ReleasedInputActionBindingHandles.Add(InputAction, ReleasedBindingHandle);

							UE_LOG(LogASSAbilitySystemInputSetup, Log, TEXT("%s() Binding to newly-added InputAction [%s] for calling GAS input events."), ANSI_TO_TCHAR(__FUNCTION__), *(InTagInputActionPair.Key.ToString()));
						}
					}
				}
			);

		// When Input Actions are removed during the game, unbind from them.
		ISObjectReferenceLibrary->OnInputActionRemoved.AddWeakLambda(this,
				[this](const TPair<FGameplayTag, TWeakObjectPtr<const UInputAction>>& InTagInputActionPair)
				{
					UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(GetOwner()->InputComponent);
					if (IsValid(PlayerEnhancedInputComponent))
					{
						const UInputAction* InputAction = InTagInputActionPair.Value.Get();
						if (IsValid(InputAction))
						{
							if (const uint32* PressedHandle = PressedInputActionBindingHandles.Find(InputAction))
							{
								PlayerEnhancedInputComponent->RemoveBindingByHandle(*PressedHandle);
								PressedInputActionBindingHandles.Remove(InputAction);
							}
							if (const uint32* ReleasedHandle = ReleasedInputActionBindingHandles.Find(InputAction))
							{
								PlayerEnhancedInputComponent->RemoveBindingByHandle(*ReleasedHandle);
								PressedInputActionBindingHandles.Remove(InputAction);
							}

							UE_LOG(LogASSAbilitySystemInputSetup, Log, TEXT("%s() InputAction [%s] removed. Stopping the calling of GAS input events."), ANSI_TO_TCHAR(__FUNCTION__), *(InTagInputActionPair.Key.ToString()));
						}
					}
				}
			);
	}
}

void UASSActorComponent_PawnAvatarActorExtension::OnOwnerDestroyPlayerInputComponent()
{
	// The InputComponent is destroyed which means all of its bindings are destroyed too. So update our handle lists.
	PressedInputActionBindingHandles.Empty();
	ReleasedInputActionBindingHandles.Empty();
}

void UASSActorComponent_PawnAvatarActorExtension::UninitializeAbilitySystemComponent()
{


	Super::UninitializeAbilitySystemComponent();

	check(GetOwner()); // we need our owner in order to remove our bindings from its input component
	UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(GetOwner()->InputComponent);
	if (IsValid(PlayerEnhancedInputComponent))
	{
		for (const TPair<TWeakObjectPtr<const UInputAction>, uint32>& PressedInputActionBindingHandle : PressedInputActionBindingHandles)
		{
			PlayerEnhancedInputComponent->RemoveBindingByHandle(PressedInputActionBindingHandle.Value);
		}
		PressedInputActionBindingHandles.Empty();

		for (const TPair<TWeakObjectPtr<const UInputAction>, uint32>& ReleasedInputActionBindingHandle : ReleasedInputActionBindingHandles)
		{
			PlayerEnhancedInputComponent->RemoveBindingByHandle(ReleasedInputActionBindingHandle.Value);
		}
		ReleasedInputActionBindingHandles.Empty();
	}
}

void UASSActorComponent_PawnAvatarActorExtension::OnPressedInputAction(const FGameplayTag InInputActionTag) const
{
	if (AbilitySystemComponent.IsValid())
	{
		TArray<FGameplayAbilitySpec*> GameplayAbilitySpecs;
		AbilitySystemComponent->GetActivatableGameplayAbilitySpecsByAllMatchingTags(InInputActionTag.GetSingleTagContainer(), GameplayAbilitySpecs, false);
		for (FGameplayAbilitySpec* GameplayAbilitySpecPtr : GameplayAbilitySpecs)
		{
			// Tell ASC about ability input pressed
			const bool bAllowAbilityActivation = GameplayAbilitySpecPtr->Ability->AbilityTags.HasTag(ASSNativeGameplayTags::Ability_Type_DisableAutoActivationFromInput) == false;
			UASSAbilitySystemBlueprintLibrary::AbilityLocalInputPressedForSpec(AbilitySystemComponent.Get(), *GameplayAbilitySpecPtr, bAllowAbilityActivation);
		}

		if (InInputActionTag == ASSNativeGameplayTags::InputAction_ConfirmTarget)
		{
			// Tell ASC about Confirm pressed
			AbilitySystemComponent->LocalInputConfirm();
		}
		if (InInputActionTag == ASSNativeGameplayTags::InputAction_CancelTarget)
		{
			// Tell ASC about Cancel pressed
			AbilitySystemComponent->LocalInputCancel();
		}
	}
}
void UASSActorComponent_PawnAvatarActorExtension::OnReleasedInputAction(const FGameplayTag InInputActionTag) const
{
	if (AbilitySystemComponent.IsValid())
	{
		TArray<FGameplayAbilitySpec*> GameplayAbilitySpecs;
		AbilitySystemComponent->GetActivatableGameplayAbilitySpecsByAllMatchingTags(InInputActionTag.GetSingleTagContainer(), GameplayAbilitySpecs, false);
		for (FGameplayAbilitySpec* GameplayAbilitySpecPtr : GameplayAbilitySpecs)
		{
			// Tell ASC about ability input released
			UASSAbilitySystemBlueprintLibrary::AbilityLocalInputReleasedForSpec(AbilitySystemComponent.Get(), *GameplayAbilitySpecPtr);
		}
	}
}
//  END Input setup
