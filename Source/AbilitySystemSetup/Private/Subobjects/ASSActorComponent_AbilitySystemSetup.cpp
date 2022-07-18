// Fill out your copyright notice in the Description page of Project Settings.


#include "Subobjects/ASSActorComponent_AbilitySystemSetup.h"

#include "Net/UnrealNetwork.h"
#include "Components/InputComponent.h"
#include "ISDeveloperSettings_InputSetup.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "InputTriggers.h"
#include "AbilitySystem/ASSAbilitySystemBlueprintLibrary.h"



UASSActorComponent_AbilitySystemSetup::UASSActorComponent_AbilitySystemSetup(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;

	bInitialized = false;
}
void UASSActorComponent_AbilitySystemSetup::OnRegister()
{
	Super::OnRegister();


#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	TArray<UActorComponent*> AbilitySystemSetupComponents;
	GetOwner()->GetComponents(ThisClass::StaticClass(), AbilitySystemSetupComponents);
	if (AbilitySystemSetupComponents.Num() > 1)
	{
		UE_LOG(LogASSSetupComponent, Error, TEXT("No more than one UASSActorComponent_AbilitySystemSetup is allowed on actors. Culprit: [%s]"), *GetNameSafe(GetOwner()));
		check(0);
	}
#endif
}


void UASSActorComponent_AbilitySystemSetup::InitializeAbilitySystemComponent(UAbilitySystemComponent* InASC)
{
	if (!IsValid(InASC))
	{
		UE_LOG(LogASSSetupComponent, Error, TEXT("%s() failed to setup with GAS because InASC passed in was NULL"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}
	if (AbilitySystemComponent == InASC)
	{
		UE_LOG(LogASSSetupComponent, Verbose, TEXT("%s() called again after already being initialized - no need to proceed"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}
	// Resolve edge case: You forgot to uninitialize the InASC before initializing a new one
	if (bInitialized || AbilitySystemComponent.IsValid())
	{
		UE_LOG(LogASSSetupComponent, Warning, TEXT("%s() - Looks like you forgot to uninitialize the ASC before initializing a new one. Maybe you forgot to uninitialize on UnPossessed() - this will probably cause unwanted side-effects such as Gameplay Effects lingering after UnPossessed(). We are uninitializing the ASC for you before initializing the new one BUT you should manually do this instead to prevent lingering stuff."), ANSI_TO_TCHAR(__FUNCTION__));
		UninitializeAbilitySystemComponent();
	}

	AActor* CurrentAvatar = InASC->GetAvatarActor(); // the passed in ASC's old avatar
	AActor* NewAvatarToUse = GetOwner();			 // new avatar for the passed in ASC
	UE_LOG(LogASSSetupComponent, Verbose, TEXT("%s() setting up ASC: [%s] on actor: [%s] with owner: [%s] and Avatar Actor: [%s]"), ANSI_TO_TCHAR(__FUNCTION__), *GetNameSafe(InASC), *GetNameSafe(NewAvatarToUse), *GetNameSafe(InASC->GetOwnerActor()), *GetNameSafe(CurrentAvatar));

	// Resolve edge cases: You forgot to uninitialize the ASC before initializing a new one    OR    destruction of previous avatar hasn't been replicated yet (because of lagged client)
	if ((CurrentAvatar != nullptr) && (CurrentAvatar != NewAvatarToUse))	// if we are switching avatars (there was previously one in use)
	{
		if (ThisClass* PreviousAbilitySystemSetupComponent = CurrentAvatar->FindComponentByClass<ThisClass>())		// get the previous ASSActorComponent_AbilitySystemSetup (the setup component of the old avatar actor)
		{
			if (PreviousAbilitySystemSetupComponent->AbilitySystemComponent == InASC)
			{
				// Our old avatar actor forgot to uninitialize the ASC    OR    our old avatar actor hasn't been destroyed by replication yet during respawn
				// We will uninitialize the ASC from the old avatar before initializing it with this new avatar
				UE_CLOG(GetOwnerRole() == ROLE_Authority, LogASSSetupComponent, Warning, TEXT("%s() - Looks like you forgot to uninitialize the ASC before initializing a new one. Maybe you forgot to uninitialize on UnPossessed() - this will probably cause unwanted side-effects such as Gameplay Effects lingering after UnPossessed(). We are uninitializing the ASC for you before initializing the new one BUT you should manually do this instead to prevent lingering stuff."), ANSI_TO_TCHAR(__FUNCTION__));	// only on the Authority because we can be certain there is something wrong if the server gets here (regardless this log should catch our attention and get us to fix it)
				PreviousAbilitySystemSetupComponent->UninitializeAbilitySystemComponent();		// kick out the old avatar from the ASC
			}
		}
	}



	AbilitySystemComponent = InASC;
	AbilitySystemComponent->InitAbilityActorInfo(InASC->GetOwnerActor(), NewAvatarToUse);

	// Grant Abilities, Active Effects, and Attribute Sets
	if (GetOwnerRole() == ROLE_Authority)
	{
		if (!bGrantedAbilitySets)
		{
			for (const TSubclassOf<UASSAbilitySet> AbilitySet : AbilitySets)
			{
				if (IsValid(AbilitySet))
				{
					AbilitySet.GetDefaultObject()->GrantToAbilitySystemComponent(InASC, GetOwner(), GrantedHandles.AddDefaulted_GetRef());
				}
			}
			bGrantedAbilitySets = true;
		}
	}

	bInitialized = true;
	OnInitializeAbilitySystemComponentDelegate.Broadcast(AbilitySystemComponent.Get());
}
void UASSActorComponent_AbilitySystemSetup::UninitializeAbilitySystemComponent()
{
	bInitialized = false;

	if (AbilitySystemComponent.IsValid())
	{
		if (AbilitySystemComponent->GetAvatarActor() == GetOwner())
		{
			// Cancel ongoing stuff
			AbilitySystemComponent->CancelAbilities(nullptr, nullptr);
			AbilitySystemComponent->RemoveAllGameplayCues();


			// Remove granted AbilitySets
			if (GetOwnerRole() == ROLE_Authority)
			{
				for (FASSAbilitySetGrantedHandles GrantHandle : GrantedHandles)
				{
					GrantHandle.RemoveFromAbilitySystemComponent();
				}
			}

			// Remove Loose Gameplay Tags
			RemoveLooseAvatarRelatedTags();


			// Clear the AvatarActor from the ASC
			if (IsValid(AbilitySystemComponent->GetOwnerActor()))
			{
				// Clear our avatar actor from it (this will re-init other actor info as well)
				AbilitySystemComponent->SetAvatarActor(nullptr);
			}
			else
			{
				// Clear ALL actor info because don't even have an owner actor for some reason
				AbilitySystemComponent->ClearActorInfo();
			}
		}
		else
		{
			UE_LOG(LogASSSetupComponent, Error, TEXT("%s() Tried uninitializing the ASC when the actor with this component was not the avatar actor"), ANSI_TO_TCHAR(__FUNCTION__));
		}
	}

	AbilitySystemComponent = nullptr;
}

void UASSActorComponent_AbilitySystemSetup::HandleControllerChanged()
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
			UE_LOG(LogASSSetupComponent, Error, TEXT("%s() Tried RefreshAbilityActorInfo(), but the actor with this component was not the avatar actor"), ANSI_TO_TCHAR(__FUNCTION__));
		}
	}
	else
	{
		// In the case of ASC being on the PlayerState, this is expected to hit on the client for initial possessions (Controller gets replicated before PlayerState)
	}
}

//  BEGIN Input setup
void UASSActorComponent_AbilitySystemSetup::SetupPlayerInputComponent(UInputComponent* InPlayerInputComponent)
{
	// Bind to all Input Actions so we can tell the ability system when ability inputs have been pressed/released
	const UISDeveloperSettings_InputSetup* InputSetupDeveloperSettings = GetDefault<UISDeveloperSettings_InputSetup>();
	if (IsValid(InputSetupDeveloperSettings))
	{
		// Bind to all known Input Actions
		UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(InPlayerInputComponent);
		if (IsValid(PlayerEnhancedInputComponent))
		{
			TMap<FGameplayTag, TSoftObjectPtr<const UInputAction>> InputActionTagMap = InputSetupDeveloperSettings->GetInputActions();
			for (const TPair<FGameplayTag, TSoftObjectPtr<const UInputAction>> TagInputActionPair : InputActionTagMap)
			{
				const UInputAction* InputAction = TagInputActionPair.Value.LoadSynchronous();
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
		InputSetupDeveloperSettings->OnRuntimeInputActionAdded.AddWeakLambda(this,
			[this](const TPair<FGameplayTag, TSoftObjectPtr<const UInputAction>>& InTagInputActionPair)
			{
				UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(GetOwner()->InputComponent);
				if (IsValid(PlayerEnhancedInputComponent))
				{
					const UInputAction* InputAction = InTagInputActionPair.Value.LoadSynchronous();
					if (IsValid(InputAction))
					{
						const uint32 PressedBindingHandle = PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Started, this, &ThisClass::OnPressedInputAction, InTagInputActionPair.Key).GetHandle();
						const uint32 ReleasedBindingHandle = PlayerEnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Completed, this, &ThisClass::OnReleasedInputAction, InTagInputActionPair.Key).GetHandle();

						PressedInputActionBindingHandles.Add(InputAction, PressedBindingHandle);
						ReleasedInputActionBindingHandles.Add(InputAction, ReleasedBindingHandle);

						UE_LOG(LogASSAbilitySystemInputSetup, Log, TEXT("%s() Binding to new runtime-added Input Action [%s] for calling GAS input events."), ANSI_TO_TCHAR(__FUNCTION__), *(InTagInputActionPair.Key.ToString()));
					}
				}
			}
		);
		// When Input Actions are removed during the game, unbind from them.
		InputSetupDeveloperSettings->OnRuntimeInputActionRemoved.AddWeakLambda(this,
			[this](const TPair<FGameplayTag, TSoftObjectPtr<const UInputAction>>& InTagInputActionPair)
			{
				UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(GetOwner()->InputComponent);
				if (IsValid(PlayerEnhancedInputComponent))
				{
					const UInputAction* InputAction = InTagInputActionPair.Value.LoadSynchronous();
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

						UE_LOG(LogASSAbilitySystemInputSetup, Log, TEXT("%s() Input Action [%s] removed at runtime. Unbinding function that triggers GAS input events."), ANSI_TO_TCHAR(__FUNCTION__), *(InTagInputActionPair.Key.ToString()));
					}
				}
			}
		);
	}
}
void UASSActorComponent_AbilitySystemSetup::DestroyPlayerInputComponent()
{
	// The InputComponent is destroyed which means all of its bindings are destroyed too. So update our handle lists.
	PressedInputActionBindingHandles.Empty();
	ReleasedInputActionBindingHandles.Empty();
}

void UASSActorComponent_AbilitySystemSetup::OnPressedInputAction(const FGameplayTag InInputActionTag) const
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
			// Tell ASC about confirm pressed
			AbilitySystemComponent->LocalInputConfirm();
		}
		if (InInputActionTag == ASSNativeGameplayTags::InputAction_CancelTarget)
		{
			// Tell ASC about cancel pressed
			AbilitySystemComponent->LocalInputCancel();
		}
	}
}
void UASSActorComponent_AbilitySystemSetup::OnReleasedInputAction(const FGameplayTag InInputActionTag) const
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

void UASSActorComponent_AbilitySystemSetup::RemoveLooseAvatarRelatedTags()
{
	if (AbilitySystemComponent.IsValid())
	{
		RemoveLooseAvatarRelatedTagsDelegate.Broadcast(AbilitySystemComponent.Get());
	}
}
