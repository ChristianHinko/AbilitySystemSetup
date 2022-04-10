// Fill out your copyright notice in the Description page of Project Settings.


#include "Subobjects/AbilitySystemSetupComponent.h"

#include "Net/UnrealNetwork.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbility.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"
#include "AbilitySystem/ASSAbilitySystemComponent.h"
#include "AbilitySystemSetup/Private/Utilities/ASSLogCategories.h"
#include "DS_AbilitySystemSetup.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/ASSAbilitySystemBlueprintLibrary.h"

#include "Kismet/KismetSystemLibrary.h"



UAbilitySystemSetupComponent::UAbilitySystemSetupComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}
void UAbilitySystemSetupComponent::InitializeComponent()
{
	Super::InitializeComponent();


#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	TArray<UActorComponent*> AbilitySystemSetupComponents;
	GetOwner()->GetComponents(ThisClass::StaticClass(), AbilitySystemSetupComponents);
	ensureAlwaysMsgf((AbilitySystemSetupComponents.Num() == 1), TEXT("Only one AbilitySystemSetupComponent should exist on [%s]."), *GetNameSafe(GetOwner()));
#endif


	// Get casted owners
	OwningPawn = Cast<APawn>(GetOwner()); // NOTE: maybe do a GetTypedOuter() instead?
}


void UAbilitySystemSetupComponent::InitializeAbilitySystemComponent(UAbilitySystemComponent* ASC)
{
	if (!IsValid(ASC))
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() failed to setup with GAS because ASC passed in was NULL"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}
	if (CurrentASC == ASC)
	{
		UE_LOG(LogAbilitySystemSetup, Verbose, TEXT("%s() called again after already being initialized - no need to proceed"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}
	// Resolve edge case: You forgot to uninitialize the ASC before initializing a new one
	if (CurrentASC.IsValid())
	{
		UE_LOG(LogAbilitySystemSetup, Warning, TEXT("%s() - Looks like you forgot to uninitialize the ASC before initializing a new one. Maybe you forgot to uninitialize on UnPossessed() - this will probably cause unwanted side-effects such as Gameplay Effects lingering after UnPossessed(). We are uninitializing the ASC for you before initializing the new one BUT you should manually do this instead to prevent lingering stuff."), ANSI_TO_TCHAR(__FUNCTION__));
		UninitializeAbilitySystemComponent();
	}

	AActor* CurrentAvatar = ASC->GetAvatarActor();	// the passed in ASC's old avatar
	AActor* NewAvatarToUse = GetOwner();				// new avatar for the passed in ASC
	UE_LOG(LogAbilitySystemSetup, Verbose, TEXT("%s() setting up ASC [%s] on actor [%s] owner [%s], current [%s] "), ANSI_TO_TCHAR(__FUNCTION__), *GetNameSafe(ASC), *GetNameSafe(NewAvatarToUse), *GetNameSafe(ASC->GetOwnerActor()), *GetNameSafe(CurrentAvatar));
	
	// Resolve edge cases: You forgot to uninitialize the ASC before initializing a new one    OR    destruction of previous avatar hasn't been replicated
	if ((CurrentAvatar != nullptr) && (CurrentAvatar != NewAvatarToUse))	// if we are switching avatars (there was previously one in use)
	{
		if (ThisClass* PreviousAbilitySystemSetupComponent = CurrentAvatar->FindComponentByClass<ThisClass>())		// get the previous AbilitySystemSetupComponent (the setup component of the old avatar actor)
		{
			if (PreviousAbilitySystemSetupComponent->CurrentASC == ASC)
			{
				// Our old avatar actor forgot to uninitialize the ASC    OR    our old avatar actor hasn't been destroyed by replication yet during respawn
				// We will uninitialize the ASC from the old avatar before initializing it with this new avatar
				UE_CLOG(GetOwnerRole() == ROLE_Authority, LogAbilitySystemSetup, Warning, TEXT("%s() - Looks like you forgot to uninitialize the ASC before initializing a new one. Maybe you forgot to uninitialize on UnPossessed() - this will probably cause unwanted side-effects such as Gameplay Effects lingering after UnPossessed(). We are uninitializing the ASC for you before initializing the new one BUT you should manually do this instead to prevent lingering stuff."), ANSI_TO_TCHAR(__FUNCTION__));	// only on the Authority because we can be certain there is something wrong if the server gets here (regardless this log should catch our attention and get us to fix it)
				PreviousAbilitySystemSetupComponent->UninitializeAbilitySystemComponent();		// kick out the old avatar from the ASC
			}
		}
	}
	
	
	
	CurrentASC = ASC;
	CurrentASC->InitAbilityActorInfo(ASC->GetOwnerActor(), NewAvatarToUse);


	if (IsValid(OwningPawn) && OwningPawn->IsPlayerControlled())
	{ 
		// Bind Player input to the AbilitySystemComponent.
		// Called from both SetupPlayerInputComponent() and SetUpAbilitySystemComponent() because of a potential race condition where the Player Controller might
		// call ClientRestart() which calls SetupPlayerInputComponent() before the Player State is repped to the client so the Player State would be null in SetupPlayerInputComponent().
		// Conversely, the Player State might be repped before the Player Controller calls ClientRestart() so the Actor's Input Component would be null in OnRep_PlayerState().
		BindAbilitySystemInput(OwningPawn->InputComponent);
	}

	// Grant Abilities, Active Effects, and Attribute Sets
	if (GetOwnerRole() == ROLE_Authority)
	{
		if (!bGrantedAbilitySets)
		{
			for (TSubclassOf<UAbilitySet> AbilitySet : AbilitySets)
			{
				if (IsValid(AbilitySet))
				{
					AbilitySet.GetDefaultObject()->GrantToAbilitySystemComponent(ASC, GetOwner(), GrantHandles.AddDefaulted_GetRef());
				}
			}
			bGrantedAbilitySets = true;
		}
	}

	OnInitializeAbilitySystemComponentDelegate.Broadcast(PreviousASC.Get(), CurrentASC.Get());
}
void UAbilitySystemSetupComponent::UninitializeAbilitySystemComponent()
{
	if (CurrentASC.IsValid())
	{
		if (CurrentASC->GetAvatarActor() == GetOwner())
		{
			CurrentASC->CancelAbilities(nullptr, nullptr);
			CurrentASC->RemoveAllGameplayCues();

			if (IsValid(CurrentASC->GetOwnerActor()))
			{
				// Clear our avatar actor from it (this will re-init other actor info as well)
				CurrentASC->SetAvatarActor(nullptr);
			}
			else
			{
				// Clear ALL actor info because don't even have an owner actor for some reason
				CurrentASC->ClearActorInfo();
			}


			// Remove Abilities, Active Effects, and Attribute Sets
			if (GetOwnerRole() == ROLE_Authority)
			{
				for (FAbilitySetGrantedHandles GrantHandle : GrantHandles)
				{
					GrantHandle.RemoveFromAbilitySystemComponent();
				}
			}

			// Remove Loose Gameplay Tags
			RemoveLooseAvatarRelatedTags();
		}
		else
		{
			UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Tried uninitializing the ASC when the actor with this component was not the avatar actor"), ANSI_TO_TCHAR(__FUNCTION__));
		}
	}

	PreviousASC = CurrentASC.Get();
	CurrentASC = nullptr;

	// TODO: This is temporary - in UE5, APawn has its own PreviousController variable that we can use rather than making our own
	PreviousController = OwningPawn->GetController();	// we make sure we set our Previous Controller right before we UnPossessed so this is the most reliable Previous Controller
}

void UAbilitySystemSetupComponent::HandleControllerChanged()
{
	if (CurrentASC.IsValid() == false)
	{
		// In the case of ASC being on the PlayerState, this is expected to hit on the client for initial possessions (Controller gets replicated before PlayerState)
		return;
	}
	if (CurrentASC->GetAvatarActor() != GetOwner())
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Tried RefreshAbilityActorInfo(), but the actor with this component was not the avatar actor"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}
	ensure(CurrentASC->AbilityActorInfo->OwnerActor == CurrentASC->GetOwnerActor());	// ensure that the owner of the AbilitySystemComponent matches the OwnerActor from the ActorInfo


	CurrentASC->RefreshAbilityActorInfo();		// update ActorInfo's Controller
}

void UAbilitySystemSetupComponent::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Called in SetupPlayerInputComponent() because of a potential race condition.
	BindAbilitySystemInput(PlayerInputComponent);
}
void UAbilitySystemSetupComponent::BindAbilitySystemInput(UInputComponent* InputComponent)
{
	if (!IsValid(InputComponent))
	{
		return;
	}

	UAbilitySystemComponent* ASC = CurrentASC.Get();
	if (!IsValid(ASC))
	{
		return;
	}

	if (!bAbilitySystemInputBinded)
	{
		const UDS_AbilitySystemSetup* AbilitySystemSetupDeveloperSettings = GetDefault<const UDS_AbilitySystemSetup>();
		if (!IsValid(AbilitySystemSetupDeveloperSettings))
		{
			UE_LOG(LogAbilitySystemSetup, Fatal, TEXT("%s() No valid pointer to UDS_AbilitySystemSetup when trying to get the name of the confirm and cancel input action names and the Ability Input Id Enum Name."), ANSI_TO_TCHAR(__FUNCTION__), *GetName());
		}

		ASC->BindAbilityActivationToInputComponent(InputComponent, FGameplayAbilityInputBinds(
			AbilitySystemSetupDeveloperSettings->ConfirmTargetInputActionName,			// name of our confirm input from the project settings
			AbilitySystemSetupDeveloperSettings->CancelTargetInputActionName,			// name of our cancel input from the project settings
			AbilitySystemSetupDeveloperSettings->AbilityInputIDEnumName					// name of our GAS input enum that gives the names of the rest of our inputs in the project settings
		)
		);

		bAbilitySystemInputBinded = true; // only run this function only once
	}

}

void UAbilitySystemSetupComponent::RemoveLooseAvatarRelatedTags()
{
	// Give external sources an opportunity to remove any Loose Gameplay Tags
	if (CurrentASC.IsValid())
	{
		RemoveLooseAvatarRelatedTagsDelegate.Broadcast(CurrentASC.Get());
	}
}
