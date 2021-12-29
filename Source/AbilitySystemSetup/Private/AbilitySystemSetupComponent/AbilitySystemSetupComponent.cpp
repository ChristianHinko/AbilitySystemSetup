// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystemSetupComponent/AbilitySystemSetupComponent.h"

#include "Net/UnrealNetwork.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/ASSGameplayAbility.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Components/InputComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemSetup/Private/Utilities/ASSLogCategories.h"
#include "DS_AbilitySystemSetup.h"
#include "AbilitySystemSetupComponent/AbilitySystemSetupInterface.h"

#include "Kismet/KismetSystemLibrary.h"



//void UAbilitySystemSetupComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//
//	//DOREPLIFETIME(UAbilitySystemSetupComponent, PlayerAbilitySystemComponent);			// can be helpful for debugging
//}

UAbilitySystemSetupComponent::UAbilitySystemSetupComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;



	// We make the AI always automatically posses us because the AI ASC will be in use before the Player possesses us so we sould have the SetupWithAbilitySystemAIControlled() run so the ASC can be used.
	// But we're not doing this because its hard to transfer ASC state to another. We don't need this feature right now
	//AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;


	bUnregisterAttributeSetsOnUnpossessed = true; // TODO: make these transfer to next ASC
	bRemoveAbilitiesOnUnpossessed = true;
	bRemoveCharacterTagsOnUnpossessed = true;


	// Minimal Mode means that no GameplayEffects will replicate. They will only live on the Server. Attributes, GameplayTags, and GameplayCues will still replicate to us.
	AIAbilitySystemComponentReplicationMode = EGameplayEffectReplicationMode::Minimal;

	AIAbilitySystemComponent = CreateOptionalDefaultSubobject<UASSAbilitySystemComponent>(TEXT("AIAbilitySystemComponent"));
	if (AIAbilitySystemComponent)
	{
		bShouldHandleAIAbilitySystemSetup = true;
		AIAbilitySystemComponent->SetReplicationMode(AIAbilitySystemComponentReplicationMode);
		AIAbilitySystemComponent->SetIsReplicated(true);
	}

	// So we can get our casted Owners
	bWantsInitializeComponent = true;
}
void UAbilitySystemSetupComponent::InitializeComponent()
{
	Super::InitializeComponent();

	OwningPawn = Cast<APawn>(GetOwner());
	OwningAbilitySystemInterface = Cast<IAbilitySystemInterface>(GetOwner());
	OwningAbilitySystemSetupInterface = Cast<IAbilitySystemSetupInterface>(GetOwner());
	if (!OwningAbilitySystemSetupInterface)
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() MAKE SURE YOU IMPLEMENT THE IAbilitySystemSetupInterface INTERFACE WHEN USING THIS COMPONENT"), *FString(__FUNCTION__));
	}
}




#pragma region Ability System Possess

void UAbilitySystemSetupComponent::SetupWithAbilitySystemPlayerControlled(APlayerState* PlayerState)
{
	IAbilitySystemInterface* AbilitySystem = Cast<IAbilitySystemInterface>(PlayerState);
	if (!AbilitySystem)
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Failed to setup with GAS on (failed to InitAbilityActorInfo, AddExistingAttributeSets, InitializeAttributes, ApplyStartupEffects, and GrantStartingAbilities). The Player State does not implement IAbilitySystemInterface (Cast failed)"), *FString(__FUNCTION__));
		return;
	}
	PlayerAbilitySystemComponent = Cast<UASSAbilitySystemComponent>(AbilitySystem->GetAbilitySystemComponent());
	if (!PlayerAbilitySystemComponent)
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Failed to setup with GAS on (failed to InitAbilityActorInfo, AddExistingAttributeSets, InitializeAttributes, ApplyStartupEffects, and GrantStartingAbilities). PlayerAbilitySystemComponent was NULL! Ensure you are using UASSAbilitySystemComponent"), *FString(__FUNCTION__));
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
			// Moved these Attribute Set stuff into this check because seems to make more sence - move outside if problems arise
			OwningAbilitySystemSetupInterface->CreateAttributeSets();
			OwningAbilitySystemSetupInterface->RegisterAttributeSets();
			// Must call ForceReplication() after registering an attribute set(s)
			PlayerAbilitySystemComponent->ForceReplication();
		}

		PreApplyStartupEffects.Broadcast();	// good place to bind to Attribute/Tag events, but currently the GE replicates to client faster than it can broadcast, so we need to fix this

		if (GetOwnerRole() == ROLE_Authority)
		{
			InitializeAttributes();
			ApplyStartupEffects();

			// This is the first time our setup is being run. So no matter what (even if bAIToPlayerSyncAbilities), grant our starting abilities.
			GrantStartingAbilities();
		}


		bInitialized = true;
	}
	else    // If something is posessing this us a second time
	{
		// Just register this our already-created attribute sets with the Player's ASC
		OwningAbilitySystemSetupInterface->RegisterAttributeSets();
		if (GetOwnerRole() == ROLE_Authority)
		{
			// Must call ForceReplication() after registering an Attribute Set(s)
			PlayerAbilitySystemComponent->ForceReplication();
		}

		if (GetOwnerRole() == ROLE_Authority) // Sync abilities between ASCs
		{
			// TODO: This is temporary - in UE5, APawn has its own PreviousController variable that we can use rather than making our own
			const bool wasPlayer = (/*OwningPawn->*/PreviousController && /*OwningPawn->*/PreviousController->IsPlayerController());
			const bool isPlayer = OwningPawn->GetController()->IsPlayerController();

			// If we went from AI -> Player
			if (wasPlayer == false && isPlayer == true)
			{
				//PlayerAbilitySystemComponent->RecieveAbilitiesFrom(AIAbilitySystemComponent);
				PlayerAbilitySystemComponent->GrantAbilities(PendingAbilitiesToSync);
				PendingAbilitiesToSync.Empty();
			}
			else // we went from Player -> Player // THIS IS NOT COMPLETELY WORKING YET
			{
				PlayerAbilitySystemComponent->GrantAbilities(PendingAbilitiesToSync);
				PendingAbilitiesToSync.Empty();
			}

			// TODO: we should have a way to sync Tags and active Effects and Abilities to across ACSs but this sounds really hard
		}
	}

	// Refresh ASC Actor Info for clients. Server will be refreshed by its AIController/PlayerController when it possesses a new Actor.
	if (OwningPawn->IsLocallyControlled()) // CLIENT
	{
		PlayerAbilitySystemComponent->RefreshAbilityActorInfo();
	}


	SetupWithAbilitySystemCompleted.Broadcast();
}
void UAbilitySystemSetupComponent::SetupWithAbilitySystemAIControlled()
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		return;
	}
	if (!bShouldHandleAIAbilitySystemSetup)
	{
		return;
	}
	if (!AIAbilitySystemComponent)
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Failed to setup with AI GAS setup on (failed to InitAbilityActorInfo, AddExistingAttributeSets, InitializeAttributes, ApplyStartupEffects, and GrantStartingAbilities). AIAbilitySystemComponent was NULL"), *FString(__FUNCTION__));
		return;
	}



	// From my understanding, only needs to be done on server since no Player is controlling it
	AIAbilitySystemComponent->InitAbilityActorInfo(GetOwner(), GetOwner());

	if (!bInitialized)
	{
		// Must run these on Server but we run them on client too so that we don't have to wait.. It's how Dan does it so seams legit
		OwningAbilitySystemSetupInterface->CreateAttributeSets();
		OwningAbilitySystemSetupInterface->RegisterAttributeSets();
		// Must call ForceReplication() after registering an Attribute Set(s)
		AIAbilitySystemComponent->ForceReplication();
		PreApplyStartupEffects.Broadcast();					// at this point the ASC is safe to use
		InitializeAttributes();
		ApplyStartupEffects();

		// This is the first time our setup is being run. So no matter what (even if bPlayerToAISyncAbilities), grant our starting Abilities
		GrantStartingAbilities();


		bInitialized = true;
	}
	else    // If something is posessing this us a second time
	{
		// Just register this our already-created Attribute Sets with the AI's ASC
		OwningAbilitySystemSetupInterface->RegisterAttributeSets();
		// Must call ForceReplication() after registering an Attribute Set(s)
		AIAbilitySystemComponent->ForceReplication();


		// Sync abilities between ASCs
		{
			// TODO: This is temporary - in UE5, APawn has its own PreviousController variable that we can use rather than making our own
			const bool wasPlayer = (/*OwningPawn->*/PreviousController && /*OwningPawn->*/PreviousController->IsPlayerController());
			const bool isPlayer = OwningPawn->GetController()->IsPlayerController();

			// If we went from Player -> AI
			if (wasPlayer == true && isPlayer == false)
			{
				//AIAbilitySystemComponent->RecieveAbilitiesFrom(PlayerAbilitySystemComponent);
				AIAbilitySystemComponent->GrantAbilities(PendingAbilitiesToSync);
				PendingAbilitiesToSync.Empty();
			}

			// TODO: we should have a way to sync Tags and active Effects and Abilities to across ACSs but this sounds really hard
		}
	}


	SetupWithAbilitySystemCompleted.Broadcast();
}

#pragma endregion

#pragma region ASC Setup Helpers

void UAbilitySystemSetupComponent::InitializeAttributes()
{
	UAbilitySystemComponent* ASC = OwningAbilitySystemInterface->GetAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("GetAbilitySystemComponent() returned null on %s"), *FString(__FUNCTION__));
		return;
	}
	if (!DefaultAttributeValuesEffectTSub)
	{
		UE_LOG(LogAbilitySystemSetup, Warning, TEXT("%s() Missing DefaultAttributeValuesEffect for %s. Please fill DefaultAttributeValuesEffect in Blueprint."), *FString(__FUNCTION__), *GetName());
		return;
	}

	// Can run on Server and Client
	FGameplayEffectContextHandle EffectContextHandle = ASC->MakeEffectContext();
	EffectContextHandle.AddInstigator(GetOwner(), GetOwner());
	EffectContextHandle.AddSourceObject(GetOwner());

	FGameplayEffectSpecHandle NewEffectSpecHandle = ASC->MakeOutgoingSpec(DefaultAttributeValuesEffectTSub, 1/*GetLevel()*/, EffectContextHandle);
	if (NewEffectSpecHandle.IsValid())
	{
		ASC->ApplyGameplayEffectSpecToSelf(*NewEffectSpecHandle.Data.Get());
	}
	else
	{
		UE_LOG(LogAbilitySystemSetup, Warning, TEXT("%s() Tried to apply the default attributes effect on %s but failed. Maybe check if you filled out your DefaultAttributeValuesEffect correctly in Blueprint"), *FString(__FUNCTION__), *GetName());
	}
}

void UAbilitySystemSetupComponent::ApplyStartupEffects()
{
	UAbilitySystemComponent* ASC = OwningAbilitySystemInterface->GetAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Tried to apply startup Effects on %s but GetAbilitySystemComponent() returned NULL"), *FString(__FUNCTION__), *GetName());
		return;
	}

	FGameplayEffectContextHandle EffectContextHandle = ASC->MakeEffectContext();
	EffectContextHandle.AddInstigator(GetOwner(), GetOwner());
	EffectContextHandle.AddSourceObject(GetOwner());
	for (int32 i = 0; i < EffectsToApplyOnStartup.Num(); ++i)
	{
		FGameplayEffectSpecHandle NewEffectSpecHandle = ASC->MakeOutgoingSpec(EffectsToApplyOnStartup[i], 1/*GetLevel()*/, EffectContextHandle);
		if (NewEffectSpecHandle.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*NewEffectSpecHandle.Data.Get());
		}
	}
}

bool UAbilitySystemSetupComponent::GrantStartingAbilities()
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		return false;
	}
	UASSAbilitySystemComponent* CastedASC = Cast<UASSAbilitySystemComponent>(OwningAbilitySystemInterface->GetAbilitySystemComponent());
	if (!CastedASC)
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Tried to grant startup abilities on %s but GetAbilitySystemComponent() returned NULL"), *FString(__FUNCTION__), *GetName());
		return false;
	}

	OwningAbilitySystemSetupInterface->GrantStartingAbilities();

	// ---------Grant non handle starting abilities---------
	for (int32 i = 0; i < NonHandleStartingAbilities.Num(); ++i)
	{
		CastedASC->GrantAbility(NonHandleStartingAbilities[i], GetOwner()/*, GetLevel()*/); // GetLevel() doesn't exist in this template. Will need to implement one if you want a level system
	}

	return true;
}
#pragma endregion

#pragma region Input

void UAbilitySystemSetupComponent::BindASCInput(UInputComponent* InputComponent)
{
	UAbilitySystemComponent* ASC = OwningAbilitySystemInterface->GetAbilitySystemComponent();
	if (!bASCInputBound && IsValid(ASC) && IsValid(InputComponent))
	{
		const UDS_AbilitySystemSetup* AbilitySystemSetupDeveloperSettings = GetDefault<UDS_AbilitySystemSetup>();
		if (IsValid(AbilitySystemSetupDeveloperSettings) == false)
		{
			UE_LOG(LogAbilitySystemSetup, Fatal, TEXT("%s() No valid pointer to UDS_AbilitySystemSetup when trying to get the name of the confirm and cancel input action names and the Ability Input Id Enum Name."), *FString(__FUNCTION__), *GetName());
		}
		ASC->BindAbilityActivationToInputComponent
		(
			InputComponent,
			FGameplayAbilityInputBinds
			(
				AbilitySystemSetupDeveloperSettings->ConfirmTargetInputActionName,			// Name of our confirm input from the project settings
				AbilitySystemSetupDeveloperSettings->CancelTargetInputActionName,			// Name of our cancel input from the project settings
				AbilitySystemSetupDeveloperSettings->AbilityInputIDEnumName					// Name of our GAS input enum that gives the names of the rest of our inputs in the project settings
			)
		);

		bASCInputBound = true;	// only run this function only once
	}
}

#pragma endregion


#pragma region Ability System Unpossess
int32 UAbilitySystemSetupComponent::UnregisterOwnedAttributeSets()
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		return 0;
	}
	UAbilitySystemComponent* ASC = OwningAbilitySystemInterface->GetAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Tried to remove owned Attribute Sets from ASC before UnPossessed but GetAbilitySystemComponent() returned NULL. ASC probably now has unneeded Attribute Sets and possibly Attribute Set duplicates now (very bad). Owner: %s"), *FString(__FUNCTION__), *GetName());
		return 0;
	}

	int32 retVal = 0;
	for (int32 i = ASC->GetSpawnedAttributes().Num() - 1; i >= 0; --i)
	{
		if (UAttributeSet* AS = ASC->GetSpawnedAttributes()[i])
		{
			if (AS->GetOwningActor() == GetOwner()) // for Attribute Set we check the OwningActor since thats what they use. It's also automatically set by the engine so were good
			{
				ASC->GetSpawnedAttributes_Mutable().RemoveAt(i);
				++retVal;
			}
		}
	}
	ASC->ForceReplication();

	return retVal;
}

int32 UAbilitySystemSetupComponent::RemoveOwnedAbilities()
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		return 0;
	}
	UAbilitySystemComponent* ASC = OwningAbilitySystemInterface->GetAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Tried to remove owned Abilities from ASC before UnPossessed but GetAbilitySystemComponent() returned NULL. ASC probably now has unneeded Abilitie(s) and possibly duplicates. Owner: %s"), *FString(__FUNCTION__), *GetName());
		return 0;
	}

	int32 retVal = 0;
	for (int32 i = ASC->GetActivatableAbilities().Num() - 1; i >= 0; --i)
	{
		FGameplayAbilitySpec Spec = ASC->GetActivatableAbilities()[i];
		if (Spec.SourceObject == GetOwner()) // for Abilities we check the SourceObject since thats what they use. SourceObjects are expected to be correct when set on GrantAbility()
		{
			ASC->ClearAbility(Spec.Handle);
			++retVal;
		}
	}

	return retVal;
}

int32 UAbilitySystemSetupComponent::RemoveAllCharacterTags()	// Only called on Authority
{
	// Needs implementation. Below I was trying to find a way to get all tags containing a parent of Character.
	//int32 amountFound = OwningAbilitySystemInterface()->GetAbilitySystemComponent()->GetTagCount(FGameplayTag::RequestGameplayTag("Character"));

	return -1;
}

void UAbilitySystemSetupComponent::UnPossessed()
{
	PendingAbilitiesToSync = OwningAbilitySystemInterface->GetAbilitySystemComponent()->GetActivatableAbilities();

	// This goes before Super so we can get the Controller before it unpossess and the Pawn's reference becomes null. If it was
	// null we can't do IsPlayerControlled() and GetAbilitySystemComponent() would return the wrong ASC so the functions that we are calling would
	// probably act really weird and try doing stuff on the wrong ASC

	if (OwningPawn->IsPlayerControlled() || (!OwningPawn->IsPlayerControlled() && bShouldHandleAIAbilitySystemSetup)) // if you were a Player or were an AI with the AIAbilitySystemComponent subobject (if we are supposed to be doing ASC logic)
	{
		if (bUnregisterAttributeSetsOnUnpossessed)
		{
			UnregisterOwnedAttributeSets();
		}
		if (bRemoveAbilitiesOnUnpossessed)
		{
			RemoveOwnedAbilities();
			for (int32 i = 0; i < PendingAbilitiesToSync.Num(); ++i)
			{
				FGameplayAbilitySpec* Spec = &PendingAbilitiesToSync[i];
				Spec->NonReplicatedInstances.Empty();
				Spec->ReplicatedInstances.Empty();
				Spec->ActiveCount = 0;
				//Spec->PendingRemove = false; // maybe not actually?

				//OwningAbilitySystemInterface->GetAbilitySystemComponent()->ClearAbility(Spec->Handle);
			}
		}
		if (bRemoveCharacterTagsOnUnpossessed)
		{
			RemoveAllCharacterTags();
		}
	}


	if (OwningPawn->IsPlayerControlled()/* || OwningPawn->GetPlayerState()->IsABot()*/) // should we be checking if we have a Player State bot whenever we want to use the Player ASC
	{
		PreviousPlayerASC = PlayerAbilitySystemComponent; // make sure we set previous ASC right before UnPossessed
	}

	// TODO: This is temporary - in UE5, APawn has its own PreviousController variable that we can use rather than making our own
	PreviousController = OwningPawn->GetController();	// we make sure we set our Previous Controller right before we UnPossessed so this is the most reliable Previous Controller
}
#pragma endregion
