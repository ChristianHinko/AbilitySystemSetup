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

	bFirstInitialize = true;

	bRemoveAttributeSetsOnUnPossessed = true;
}
void UAbilitySystemSetupComponent::InitializeComponent()
{
	Super::InitializeComponent();


	// Get casted owners
	OwningPawn = Cast<APawn>(GetOwner()); // NOTE: maybe do a GetTypedOuter() instead?

	// Create Attribute Sets using the StartingAttributeSets array
	if (GetOwnerRole() == ROLE_Authority)
	{
		for (const TSubclassOf<UAttributeSet> AttributeSetClass : StartingAttributeSets)
		{
			// Create this new Attribute Set
			UAttributeSet* NewAttributeSet = NewObject<UAttributeSet>(GetOwner(), AttributeSetClass);
			CreatedAttributeSets.Add(NewAttributeSet);
		}
	}
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

	if (GetOwnerRole() == ROLE_Authority)
	{
		AddStartingAttributeSets();
	}

	if (bFirstInitialize)
	{
		OnAbilitySystemSetUpPreInitializedDelegate.Broadcast(PreviousASC.Get(), CurrentASC.Get()); // good place to bind to Attribute/Tag events, but currently the GE replicates to client faster than it can broadcast, so we need to fix this

		if (GetOwnerRole() == ROLE_Authority)
		{
			ApplyStartingEffects();
		}

		bFirstInitialize = false;
	}

	if (GetOwnerRole() == ROLE_Authority)
	{
		GiveStartingAbilities();
	}

	OnAbilitySystemSetUpDelegate.Broadcast(PreviousASC.Get(), CurrentASC.Get());
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

			if (GetOwnerRole() == ROLE_Authority)
			{
				ClearGivenAbilities();

				if (bRemoveAttributeSetsOnUnPossessed)
				{
					RemoveOwnedAttributeSets();
				}

				// Give the game an opportunity to remove all Character related tags
				RemoveAvatarRelatedTagsDelegate.Broadcast(CurrentASC.Get());
			}
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

//BEGIN InitializeAbilitySystemComponent() helper functions
void UAbilitySystemSetupComponent::AddStartingAttributeSets()
{
	UAbilitySystemComponent* ASC = CurrentASC.Get();
	if (!IsValid(ASC))
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() ASC was invalid"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}


	// Allow a chance for owner to give starting Attribute Sets through C++
	AddStartingAttributeSetsDelegate.Broadcast(ASC);

	// Add our CreatedAttributeSets
	for (UAttributeSet* AttributeSet : CreatedAttributeSets)
	{
		if (UASSAbilitySystemBlueprintLibrary::GetAttributeSet(ASC, AttributeSet->GetClass()))
		{
			// Already add an Attribute Set of this class!
			UE_LOG(LogAbilitySystemSetup, Warning, TEXT("%s() Tried to add a %s when one has already been added! Skipping this Attribute Set."), ANSI_TO_TCHAR(__FUNCTION__), *(AttributeSet->GetClass()->GetName()));
			continue;
		}

		// Add this Attribute Set
		AttributeSet->Rename(nullptr, GetOwner()); // assign the outer so that we know that it is ours so we can remove it when needed - I saw Roy do this too in his ArcInventory
		ASC->AddAttributeSetSubobject(AttributeSet);
	}


	// Mark it Net Dirty after adding any Attribute Sets
	ASC->ForceReplication();
}

void UAbilitySystemSetupComponent::ApplyStartingEffects()
{
	UAbilitySystemComponent* ASC = CurrentASC.Get();
	if (!IsValid(ASC))
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Tried to apply starting Effects on %s but AbilitySystemComponent was NULL"), ANSI_TO_TCHAR(__FUNCTION__), *GetName());
		return;
	}

	FGameplayEffectContextHandle EffectContextHandle = ASC->MakeEffectContext();
	EffectContextHandle.AddInstigator(GetOwner(), GetOwner());
	EffectContextHandle.AddSourceObject(GetOwner());
	for (int32 i = 0; i < StartingEffects.Num(); ++i)
	{
		FGameplayEffectSpecHandle NewEffectSpecHandle = ASC->MakeOutgoingSpec(StartingEffects[i], 1/*GetLevel()*/, EffectContextHandle);
		if (NewEffectSpecHandle.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*NewEffectSpecHandle.Data.Get());
		}
	}
}

bool UAbilitySystemSetupComponent::GiveStartingAbilities()
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		return false;
	}
	UAbilitySystemComponent* ASC = CurrentASC.Get();
	if (!IsValid(ASC))
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Tried to give starting abilities on %s but AbilitySystemComponent was NULL"), ANSI_TO_TCHAR(__FUNCTION__), *GetName());
		return false;
	}

	// Allow a chance for owner to give starting abilities through C++
	GiveStartingAbilitiesDelegate.Broadcast(ASC);

	// Give non-handle starting Abilities
	for (int32 i = 0; i < StartingAbilities.Num(); ++i)
	{
		FGameplayAbilitySpec Spec = FGameplayAbilitySpec(StartingAbilities[i], /*, GetLevel()*/1, INDEX_NONE, GetOwner());
		ASC->GiveAbility(Spec);
	}

	return true;
}
//END InitializeAbilitySystemComponent() helper functions

//BEGIN UninitializeAbilitySystemComponent() helper functions
int32 UAbilitySystemSetupComponent::ClearGivenAbilities()
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		return 0;
	}
	UAbilitySystemComponent* ASC = CurrentASC.Get();
	if (!IsValid(ASC))
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() ASC was invalid. Could not clear given Abilities from ASC so it probably has unneeded Abilities and possibly duplicates now. Owner: %s"), ANSI_TO_TCHAR(__FUNCTION__), *GetName());
		return 0;
	}


	int32 RetVal = 0;

	for (int32 i = ASC->GetActivatableAbilities().Num() - 1; i >= 0; --i)
	{
		const FGameplayAbilitySpec& Spec = ASC->GetActivatableAbilities()[i];
		if (Spec.SourceObject == GetOwner()) // ensure we were the ones who gave this Ability - SourceObjects are expected to correctly assigned when using GiveAbility()
		{
			// Remove it
			ASC->ClearAbility(Spec.Handle);
			++RetVal;
		}
	}

	return RetVal;
}

int32 UAbilitySystemSetupComponent::RemoveOwnedAttributeSets()
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		return 0;
	}
	UAbilitySystemComponent* ASC = CurrentASC.Get();
	if (!IsValid(ASC))
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() ASC was invalid. Could not remove owned Attribute Sets from ASC so it probably has unneeded Attribute Sets and possibly duplicates now (very bad). Owner: %s"), ANSI_TO_TCHAR(__FUNCTION__), *GetName());
		return 0;
	}


	int32 RetVal = 0;

	for (int32 i = ASC->GetSpawnedAttributes().Num() - 1; i >= 0; --i)
	{
		if (const UAttributeSet* AS = ASC->GetSpawnedAttributes()[i])
		{
			if (AS->GetOwningActor() == GetOwner()) // ensure we were the one who added this Attribute Set
			{
				// remove it
				ASC->GetSpawnedAttributes_Mutable().RemoveAt(i);
				++RetVal;
			}
		}
	}

	// Mark it Net Dirty
	ASC->ForceReplication();

	return RetVal;
}
//END UninitializeAbilitySystemComponent() helper functions
