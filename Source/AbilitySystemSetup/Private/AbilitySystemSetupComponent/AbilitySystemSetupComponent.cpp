// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystemSetupComponent/AbilitySystemSetupComponent.h"

#include "Net/UnrealNetwork.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbility.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Components/InputComponent.h"
#include "AbilitySystem/ASSAbilitySystemComponent.h"
#include "AbilitySystemSetup/Private/Utilities/ASSLogCategories.h"
#include "DS_AbilitySystemSetup.h"
#include "AbilitySystemSetupComponent/AbilitySystemSetupInterface.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/ASSAbilitySystemBlueprintLibrary.h"

#include "Kismet/KismetSystemLibrary.h"



UAbilitySystemSetupComponent::UAbilitySystemSetupComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Create AIAbilitySystemComponent
	AIAbilitySystemComponent = CreateOptionalDefaultSubobject<UASSAbilitySystemComponent>(TEXT("AIAbilitySystemComponent"));
	if (IsValid(AIAbilitySystemComponent))
	{
		AIAbilitySystemComponent->SetIsReplicated(true);
		AIAbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal); // Minimal mode means that no Gameplay Effects will replicate. They will only live on the Server. Attributes, Gameplay Tags, and Gameplay Cues will still replicate to us. NOTE: Owner Actors can access and change this if needed
	}

	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;

	// We make the AI always automatically posses us because the AI ASC will be in use before the Player possesses us so we sould have the SetupWithAbilitySystemAIControlled() run so the ASC can be used.
	// But we're not doing this because its hard to transfer ASC state to another. We don't need this feature right now
	//AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	bRemoveAttributeSetsOnUnPossessed = true; // TODO: make these transfer to next ASC
	bClearAbilitiesOnUnPossessed = true;
	bRemoveCharacterTagsOnUnpossessed = true;
}
void UAbilitySystemSetupComponent::InitializeComponent()
{
	Super::InitializeComponent();


	// Get casted owners
	OwningPawn = Cast<APawn>(GetOwner()); // NOTE: maybe do a GetTypedOuter() instead?
	OwningAbilitySystemSetupInterface = Cast<IAbilitySystemSetupInterface>(GetOwner()); // NOTE: maybe do a UBFL_InterfaceHelpers::GetInterfaceTypedOuter() instead?
	if (!OwningAbilitySystemSetupInterface)
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() MAKE SURE YOU IMPLEMENT THE IAbilitySystemSetupInterface INTERFACE WHEN USING THIS COMPONENT"), ANSI_TO_TCHAR(__FUNCTION__));
	}

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


UASSAbilitySystemComponent* UAbilitySystemSetupComponent::GetAbilitySystemComponent() const
{
	if (OwningPawn->IsPlayerControlled())
	{
		return PlayerAbilitySystemComponent.Get();
	}
	else // AI controlled
	{
		return AIAbilitySystemComponent;
	}
}

//BEGIN On Possess setup
void UAbilitySystemSetupComponent::SetupWithAbilitySystemPlayerControlled(APlayerState* PlayerState)
{
	const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(PlayerState);
	if (!AbilitySystemInterface)
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Failed to setup with GAS on (failed to InitAbilityActorInfo, AddExistingAttributeSets, InitializeAttributes, ApplyStartupEffects, and GiveStartingAbilities). The Player State does not implement IAbilitySystemInterface (Cast failed)"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	PlayerAbilitySystemComponent = Cast<UASSAbilitySystemComponent>(AbilitySystemInterface->GetAbilitySystemComponent());
	if (!PlayerAbilitySystemComponent.IsValid())
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Failed to setup with GAS on (failed to InitAbilityActorInfo, AddExistingAttributeSets, InitializeAttributes, ApplyStartupEffects, and GiveStartingAbilities). PlayerAbilitySystemComponent was NULL! Ensure you are using UASSAbilitySystemComponent"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}



	// This must be done on both client and server
	PlayerAbilitySystemComponent->InitAbilityActorInfo(PlayerState, GetOwner());

	// Bind Player input to the AbilitySystemComponent. Also called in SetupPlayerInputComponent() because of a potential race condition
	BindASCInput(OwningPawn->InputComponent);

	if (!bInitialized)
	{
		if (GetOwnerRole() == ROLE_Authority)
		{
			AddAttributeSets();
		}

		OnAbilitySystemSetUpPreInitialized.Broadcast(PreviousASC.Get(), PlayerAbilitySystemComponent.Get()); // good place to bind to Attribute/Tag events, but currently the GE replicates to client faster than it can broadcast, so we need to fix this

		if (GetOwnerRole() == ROLE_Authority)
		{
			InitializeAttributes();
			ApplyStartupEffects();

			// This is the first time our setup is being run so give our starting Abilities
			GiveStartingAbilities();
		}


		bInitialized = true;
	}
	else // if something is posessing this us a second time
	{
		// Just add our already-created Attribute Sets with the ASC
		AddAttributeSets();

		// Transfer Abilities between ASCs
		if (GetOwnerRole() == ROLE_Authority)
		{
			//PlayerAbilitySystemComponent->RecieveAbilitiesFrom(PreviousASC);
			PlayerAbilitySystemComponent->GiveAbilities(PendingAbilitiesToTransfer);
			PendingAbilitiesToTransfer.Empty();

			// TODO: we should have a way to transfer Tags and active Effects and Abilities to across ACSs but this sounds really hard
		}
	}

	// Refresh ASC Actor Info for clients. Server will be refreshed by its AIController/PlayerController when it possesses a new Actor.
	if (OwningPawn->IsLocallyControlled()) // CLIENT
	{
		PlayerAbilitySystemComponent->RefreshAbilityActorInfo();
	}


	OnAbilitySystemSetUp.Broadcast(PreviousASC.Get(), PlayerAbilitySystemComponent.Get());
}
void UAbilitySystemSetupComponent::SetupWithAbilitySystemAIControlled()
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		return;
	}
	if (!IsValid(AIAbilitySystemComponent))
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Failed to setup with AI GAS setup on (failed to InitAbilityActorInfo, AddExistingAttributeSets, InitializeAttributes, ApplyStartupEffects, and GiveStartingAbilities). AIAbilitySystemComponent was NULL"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}



	// From my understanding, only needs to be done on server since no Player is controlling it
	AIAbilitySystemComponent->InitAbilityActorInfo(GetOwner(), GetOwner());

	if (!bInitialized)
	{
		AddAttributeSets();

		OnAbilitySystemSetUpPreInitialized.Broadcast(PreviousASC.Get(), AIAbilitySystemComponent); // at this point the ASC is safe to use

		InitializeAttributes();
		ApplyStartupEffects();

		// This is the first time our setup is being run so give our starting Abilities
		GiveStartingAbilities();


		bInitialized = true;
	}
	else // if something is posessing this us a second time
	{
		// Just add this our already-created Attribute Sets with the ASC
		AddAttributeSets();


		// Transfer Abilities between ASCs
		{
			//PlayerAbilitySystemComponent->RecieveAbilitiesFrom(PreviousASC);
			PlayerAbilitySystemComponent->GiveAbilities(PendingAbilitiesToTransfer);
			PendingAbilitiesToTransfer.Empty();

			// TODO: We should have a way to transfer Tags and active Effects and Abilities to across ACSs but this sounds really hard
		}
	}


	OnAbilitySystemSetUp.Broadcast(PreviousASC.Get(), AIAbilitySystemComponent);
}
//END On Possess setup

//BEGIN On Possess helper functions
void UAbilitySystemSetupComponent::AddAttributeSets()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!IsValid(ASC))
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() ASC was invalid"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}


	// Notify our owner
	OwningAbilitySystemSetupInterface->AddAttributeSets();

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

void UAbilitySystemSetupComponent::InitializeAttributes()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!IsValid(ASC))
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("GetAbilitySystemComponent() returned null on %s"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}
	if (!IsValid(InitializationEffectTSub))
	{
		UE_LOG(LogAbilitySystemSetup, Warning, TEXT("%s() Missing InitializationEffectTSub for %s. Please fill InitializationEffectTSub in Blueprint."), ANSI_TO_TCHAR(__FUNCTION__), *GetName());
		return;
	}


	// Init Attribute Set defaults NOTE: we don't use the FAttributeSetInitter system but we are calling this here if we ever wanted to TODO: commented out because	UAbilitySystemGlobals::AllocAttributeSetInitter() never ends up getting called
	//UAbilitySystemGlobals::Get().GetAttributeSetInitter()->InitAttributeSetDefaults(ASC, FName(TEXT("Default"))/*GetCharacterName()*/, 1/*GetLevel()*/, true);


	// Apply default Attribute values Effects
	FGameplayEffectContextHandle EffectContextHandle = ASC->MakeEffectContext();
	EffectContextHandle.AddInstigator(GetOwner(), GetOwner());
	EffectContextHandle.AddSourceObject(GetOwner());

	FGameplayEffectSpecHandle NewEffectSpecHandle = ASC->MakeOutgoingSpec(InitializationEffectTSub, 1/*GetLevel()*/, EffectContextHandle);
	if (NewEffectSpecHandle.IsValid())
	{
		ASC->ApplyGameplayEffectSpecToSelf(*NewEffectSpecHandle.Data.Get());
	}
	else
	{
		UE_LOG(LogAbilitySystemSetup, Warning, TEXT("%s() Tried to apply the default attributes effect on %s but failed. Maybe check if you filled out your InitializationEffectTSub correctly in Blueprint"), ANSI_TO_TCHAR(__FUNCTION__), *GetName());
	}
}

void UAbilitySystemSetupComponent::ApplyStartupEffects()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!IsValid(ASC))
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Tried to apply startup Effects on %s but GetAbilitySystemComponent() returned NULL"), ANSI_TO_TCHAR(__FUNCTION__), *GetName());
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
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!IsValid(ASC))
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Tried to give startup abilities on %s but GetAbilitySystemComponent() returned NULL"), ANSI_TO_TCHAR(__FUNCTION__), *GetName());
		return false;
	}

	// Notify our owner
	OwningAbilitySystemSetupInterface->GiveStartingAbilities();

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
void UAbilitySystemSetupComponent::BindASCInput(UInputComponent* InputComponent)
{
	if (!IsValid(InputComponent))
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
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
	PendingAbilitiesToTransfer = GetAbilitySystemComponent()->GetActivatableAbilities();

	// This goes before Super so we can get the Controller before it unpossess and the Pawn's reference becomes null. If it was
	// null we can't do IsPlayerControlled() and GetAbilitySystemComponent() would return the wrong ASC so the functions that we are calling would
	// probably act really weird and try doing stuff on the wrong ASC

	const bool bAIWithoutASC = (OwningPawn->IsPlayerControlled() == false && !IsValid(AIAbilitySystemComponent));
	if (!bAIWithoutASC) // if we were a Player with an ASC or we were an AI with an ASC
	{
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

				//GetAbilitySystemComponent()->ClearAbility(Spec->Handle);
			}
		}

		if (bRemoveCharacterTagsOnUnpossessed)
		{
			RemoveAllCharacterTags();
		}
	}

	// Make sure we set previous ASC right before UnPossessed
	PreviousASC = GetAbilitySystemComponent();

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
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
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
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
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
	//int32 AmountFound = GetAbilitySystemComponent()->GetTagCount(FGameplayTag::RequestGameplayTag("Character"));

	return -1;
}
//END On UnPossess helper functions
