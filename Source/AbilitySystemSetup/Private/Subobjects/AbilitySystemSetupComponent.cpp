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

	bRemoveAttributeSetsOnUnPossessed = true;
	bClearAbilitiesOnUnPossessed = true;
	bRemoveCharacterTagsOnUnpossessed = true;
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


//BEGIN On Possess setup
void UAbilitySystemSetupComponent::SetUpAbilitySystemComponent(UAbilitySystemComponent* ASC)
{
	CurrentASC = ASC;
	if (!IsValid(ASC))
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Failed to setup with GAS because ASC passed in was NULL"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}


	// This must be done on both client and server
	ASC->InitAbilityActorInfo(ASC->GetOwnerActor(), GetOwner());


	if (IsValid(OwningPawn) && OwningPawn->IsPlayerControlled())
	{ 
		// Bind Player input to the AbilitySystemComponent.
		// Called from both SetupPlayerInputComponent() and SetUpAbilitySystemComponent() because of a potential race condition where the Player Controller might
		// call ClientRestart() which calls SetupPlayerInputComponent() before the Player State is repped to the client so the Player State would be null in SetupPlayerInputComponent().
		// Conversely, the Player State might be repped before the Player Controller calls ClientRestart() so the Actor's Input Component would be null in OnRep_PlayerState().
		BindASCInput(OwningPawn->InputComponent);
	}

	if (!bInitialized)
	{
		if (GetOwnerRole() == ROLE_Authority)
		{
			AddStartingAttributeSets();
		}

		OnAbilitySystemSetUpPreInitialized.Broadcast(PreviousASC.Get(), ASC); // good place to bind to Attribute/Tag events, but currently the GE replicates to client faster than it can broadcast, so we need to fix this

		if (GetOwnerRole() == ROLE_Authority)
		{
			ApplyStartingEffects();

			// This is the first time our setup is being run so give our starting Abilities
			GiveStartingAbilities();
		}


		bInitialized = true;
	}
	else // we aren't a first time possession
	{
		// Just add our already-created Attribute Sets with the ASC
		AddStartingAttributeSets();

		// Transfer Abilities between ASCs
		if (GetOwnerRole() == ROLE_Authority)
		{
			UASSAbilitySystemBlueprintLibrary::GiveAbilities(ASC, PendingAbilitiesToTransfer);
			PendingAbilitiesToTransfer.Empty();

			// TODO: we should have a way to transfer Tags and active Effects and Abilities to across ACSs but this sounds really hard
		}
	}


	OnAbilitySystemSetUp.Broadcast(PreviousASC.Get(), ASC);
}
//END On Possess setup

//BEGIN On Possess helper functions
void UAbilitySystemSetupComponent::AddStartingAttributeSets()
{
	UAbilitySystemComponent* ASC = CurrentASC.Get();
	if (!IsValid(ASC))
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() ASC was invalid"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}


	// Allow a chance for owner to give starting Attribute Sets through C++
	OnAddStartingAttributeSets.Broadcast(ASC);

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
	OnGiveStartingAbilities.Broadcast(ASC);

	// Give non-handle starting Abilities
	for (int32 i = 0; i < StartingAbilities.Num(); ++i)
	{
		FGameplayAbilitySpec Spec(StartingAbilities[i], /*, GetLevel()*/1, -1, GetOwner()); // GetLevel() doesn't exist in this template. Will need to implement one if you want a level system
		ASC->GiveAbility(Spec);
	}

	return true;
}
//END On Possess helper functions

//BEGIN Input setup
void UAbilitySystemSetupComponent::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Called in SetupPlayerInputComponent() because of a potential race condition.
	BindASCInput(PlayerInputComponent);
}
void UAbilitySystemSetupComponent::BindASCInput(UInputComponent* InputComponent)
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

	if (!bASCInputBound)
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

		bASCInputBound = true; // only run this function only once
	}

}
//END Input setup


//BEGIN On UnPossess setup
void UAbilitySystemSetupComponent::UnPossessed()
{
	if (CurrentASC.IsValid())
	{
		PendingAbilitiesToTransfer = CurrentASC->GetActivatableAbilities();


		if (bRemoveAttributeSetsOnUnPossessed)
		{
			RemoveOwnedAttributeSets();
		}

		if (bClearAbilitiesOnUnPossessed)
		{
			ClearGivenAbilities();
			for (int32 i = 0; i < PendingAbilitiesToTransfer.Num(); ++i)
			{
				FGameplayAbilitySpec& Spec = PendingAbilitiesToTransfer[i];
				Spec.NonReplicatedInstances.Empty();
				Spec.ReplicatedInstances.Empty();
				Spec.ActiveCount = 0;
				//Spec.PendingRemove = false; // maybe not actually?

				//AbilitySystemComponent->ClearAbility(Spec->Handle);
			}
		}

		if (bRemoveCharacterTagsOnUnpossessed)
		{
			RemoveAllCharacterTags();
		}
	}

	// Make sure we set previous ASC right before UnPossessed
	PreviousASC = CurrentASC.Get();

	// TODO: This is temporary - in UE5, APawn has its own PreviousController variable that we can use rather than making our own
	PreviousController = OwningPawn->GetController();	// we make sure we set our Previous Controller right before we UnPossessed so this is the most reliable Previous Controller
}
//END On UnPossess setup

//BEGIN On UnPossess helper functions
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

int32 UAbilitySystemSetupComponent::RemoveAllCharacterTags() // only called on Authority
{
	// Needs implementation. Below I was trying to find a way to get all Tags containing a parent of Character.
	//int32 AmountFound = AbilitySystemComponent->GetTagCount(FGameplayTag::RequestGameplayTag("Character"));

	return -1;
}
//END On UnPossess helper functions
