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
	if (IsValid(AIAbilitySystemComponent))
	{
		AIAbilitySystemComponent->SetReplicationMode(AIAbilitySystemComponentReplicationMode);
		AIAbilitySystemComponent->SetIsReplicated(true);
	}

	// So we can get our casted Owners
	bWantsInitializeComponent = true;
}
void UAbilitySystemSetupComponent::InitializeComponent()
{
	Super::InitializeComponent();

	OwningPawn = Cast<APawn>(GetOwner()); // TODO: maybe instead of Cast<APawn>() we can do a GetTypedOuter<APawn>() so this can work on plain Actors too
	OwningAbilitySystemSetupInterface = Cast<IAbilitySystemSetupInterface>(GetOwner()); // TODO: maybe do a UBFL_InterfaceHelpers::GetInterfaceTypedOuter()?
	if (!OwningAbilitySystemSetupInterface)
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() MAKE SURE YOU IMPLEMENT THE IAbilitySystemSetupInterface INTERFACE WHEN USING THIS COMPONENT"), ANSI_TO_TCHAR(__FUNCTION__));
	}
}



UASSAbilitySystemComponent* UAbilitySystemSetupComponent::GetAbilitySystemComponent() const
{
	if (OwningPawn->IsPlayerControlled())
	{
		return PlayerAbilitySystemComponent;
	}
	else // AI controlled
	{
		return AIAbilitySystemComponent;
	}
}

#pragma region Ability System Possess

void UAbilitySystemSetupComponent::SetupWithAbilitySystemPlayerControlled(APlayerState* PlayerState)
{
	IAbilitySystemInterface* AbilitySystem = Cast<IAbilitySystemInterface>(PlayerState);
	if (!AbilitySystem)
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Failed to setup with GAS on (failed to InitAbilityActorInfo, AddExistingAttributeSets, InitializeAttributes, ApplyStartupEffects, and GiveStartingAbilities). The Player State does not implement IAbilitySystemInterface (Cast failed)"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	PlayerAbilitySystemComponent = Cast<UASSAbilitySystemComponent>(AbilitySystem->GetAbilitySystemComponent());
	if (!IsValid(PlayerAbilitySystemComponent))
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
			// Moved these Attribute Set stuff into this check because seems to make more sence - move outside if problems arise
			OwningAbilitySystemSetupInterface->CreateAttributeSets();
			OwningAbilitySystemSetupInterface->RegisterAttributeSets();
			// Must call ForceReplication() after registering an Attribute Set(s)
			PlayerAbilitySystemComponent->ForceReplication();
		}

		OnAbilitySystemSetUpPreInitialized.Broadcast(PreviousASC, PlayerAbilitySystemComponent); // good place to bind to Attribute/Tag events, but currently the GE replicates to client faster than it can broadcast, so we need to fix this

		if (GetOwnerRole() == ROLE_Authority)
		{
			InitializeAttributes();
			ApplyStartupEffects();

			// This is the first time our setup is being run. So no matter what (even if bAIToPlayerSyncAbilities), give our starting Abilities.
			GiveStartingAbilities();
		}


		bInitialized = true;
	}
	else // if something is posessing this us a second time
	{
		// Just register this our already-created Attribute Sets with the Player's ASC
		OwningAbilitySystemSetupInterface->RegisterAttributeSets();
		if (GetOwnerRole() == ROLE_Authority)
		{
			// Must call ForceReplication() after registering an Attribute Set(s)
			PlayerAbilitySystemComponent->ForceReplication();
		}

		// Sync Abilities between ASCs
		if (GetOwnerRole() == ROLE_Authority)
		{
			//PlayerAbilitySystemComponent->RecieveAbilitiesFrom(PreviousASC);
			PlayerAbilitySystemComponent->GiveAbilities(PendingAbilitiesToSync);
			PendingAbilitiesToSync.Empty();

			// TODO: we should have a way to sync Tags and active Effects and Abilities to across ACSs but this sounds really hard
		}
	}

	// Refresh ASC Actor Info for clients. Server will be refreshed by its AIController/PlayerController when it possesses a new Actor.
	if (OwningPawn->IsLocallyControlled()) // CLIENT
	{
		PlayerAbilitySystemComponent->RefreshAbilityActorInfo();
	}


	OnAbilitySystemSetUp.Broadcast(PreviousASC, PlayerAbilitySystemComponent);
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
		// Must run these on Server but we run them on client too so that we don't have to wait.. It's how Dan does it so seams legit
		OwningAbilitySystemSetupInterface->CreateAttributeSets();
		OwningAbilitySystemSetupInterface->RegisterAttributeSets();
		// Must call ForceReplication() after registering an Attribute Set(s)
		AIAbilitySystemComponent->ForceReplication();

		OnAbilitySystemSetUpPreInitialized.Broadcast(PreviousASC, AIAbilitySystemComponent); // at this point the ASC is safe to use

		InitializeAttributes();
		ApplyStartupEffects();

		// This is the first time our setup is being run. So no matter what (even if bPlayerToAISyncAbilities), give our starting Abilities
		GiveStartingAbilities();


		bInitialized = true;
	}
	else // if something is posessing this us a second time
	{
		// Just register this our already-created Attribute Sets with the AI's ASC
		OwningAbilitySystemSetupInterface->RegisterAttributeSets();
		// Must call ForceReplication() after registering an Attribute Set(s)
		AIAbilitySystemComponent->ForceReplication();


		// Sync Abilities between ASCs
		{
			//PlayerAbilitySystemComponent->RecieveAbilitiesFrom(PreviousASC);
			PlayerAbilitySystemComponent->GiveAbilities(PendingAbilitiesToSync);
			PendingAbilitiesToSync.Empty();

			// TODO: We should have a way to sync Tags and active Effects and Abilities to across ACSs but this sounds really hard
		}
	}


	OnAbilitySystemSetUp.Broadcast(PreviousASC, AIAbilitySystemComponent);
}

#pragma endregion

#pragma region ASC Setup Helpers

void UAbilitySystemSetupComponent::InitializeAttributes()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!IsValid(ASC))
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("GetAbilitySystemComponent() returned null on %s"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}
	if (!IsValid(DefaultAttributeValuesEffectTSub))
	{
		UE_LOG(LogAbilitySystemSetup, Warning, TEXT("%s() Missing DefaultAttributeValuesEffect for %s. Please fill DefaultAttributeValuesEffect in Blueprint."), ANSI_TO_TCHAR(__FUNCTION__), *GetName());
		return;
	}


	// Init Attribute Set defaults NOTE: we don't use the FAttributeSetInitter system but we are calling this here if we ever wanted to TODO: commented out because	UAbilitySystemGlobals::AllocAttributeSetInitter() never ends up getting called
	//UAbilitySystemGlobals::Get().GetAttributeSetInitter()->InitAttributeSetDefaults(ASC, FName(TEXT("Default"))/*GetCharacterName()*/, 1/*GetLevel()*/, true);


	// Apply default Attribute values Effects
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
		UE_LOG(LogAbilitySystemSetup, Warning, TEXT("%s() Tried to apply the default attributes effect on %s but failed. Maybe check if you filled out your DefaultAttributeValuesEffect correctly in Blueprint"), ANSI_TO_TCHAR(__FUNCTION__), *GetName());
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
	for (int32 i = 0; i < EffectsToApplyOnStartup.Num(); ++i)
	{
		FGameplayEffectSpecHandle NewEffectSpecHandle = ASC->MakeOutgoingSpec(EffectsToApplyOnStartup[i], 1/*GetLevel()*/, EffectContextHandle);
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

	OwningAbilitySystemSetupInterface->GiveStartingAbilities();

	// Give non-handle starting Abilities
	for (int32 i = 0; i < NonHandleStartingAbilities.Num(); ++i)
	{
		FGameplayAbilitySpec Spec(NonHandleStartingAbilities[i], /*, GetLevel()*/1, -1, GetOwner()); // GetLevel() doesn't exist in this template. Will need to implement one if you want a level system
		ASC->GiveAbility(Spec);
	}

	return true;
}
#pragma endregion

#pragma region Input

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
		const UDS_AbilitySystemSetup* AbilitySystemSetupDeveloperSettings = GetDefault<UDS_AbilitySystemSetup>();
		if (!IsValid(AbilitySystemSetupDeveloperSettings))
		{
			UE_LOG(LogAbilitySystemSetup, Fatal, TEXT("%s() No valid pointer to UDS_AbilitySystemSetup when trying to get the name of the confirm and cancel input action names and the Ability Input Id Enum Name."), ANSI_TO_TCHAR(__FUNCTION__), *GetName());
		}

		ASC->BindAbilityActivationToInputComponent(InputComponent, FGameplayAbilityInputBinds(
				AbilitySystemSetupDeveloperSettings->ConfirmTargetInputActionName,			// Name of our confirm input from the project settings
				AbilitySystemSetupDeveloperSettings->CancelTargetInputActionName,			// Name of our cancel input from the project settings
				AbilitySystemSetupDeveloperSettings->AbilityInputIDEnumName					// Name of our GAS input enum that gives the names of the rest of our inputs in the project settings
			)
		);

		bASCInputBound = true; // only run this function only once
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
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!IsValid(ASC))
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Tried to remove owned Attribute Sets from ASC before UnPossessed but GetAbilitySystemComponent() returned NULL. ASC probably now has unneeded Attribute Sets and possibly Attribute Set duplicates now (very bad). Owner: %s"), ANSI_TO_TCHAR(__FUNCTION__), *GetName());
		return 0;
	}


	int32 RetVal = 0;
	for (int32 i = ASC->GetSpawnedAttributes().Num() - 1; i >= 0; --i)
	{
		if (UAttributeSet* AS = ASC->GetSpawnedAttributes()[i])
		{
			if (AS->GetOwningActor() == GetOwner()) // for Attribute Set we check the OwningActor since thats what they use. It's also automatically set by the engine so were good
			{
				ASC->GetSpawnedAttributes_Mutable().RemoveAt(i);
				++RetVal;
			}
		}
	}

	ASC->ForceReplication();

	return RetVal;
}

int32 UAbilitySystemSetupComponent::RemoveOwnedAbilities()
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		return 0;
	}
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!IsValid(ASC))
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Tried to remove owned Abilities from ASC before UnPossessed but GetAbilitySystemComponent() returned NULL. ASC probably now has unneeded Abilitie(s) and possibly duplicates. Owner: %s"), ANSI_TO_TCHAR(__FUNCTION__), *GetName());
		return 0;
	}


	int32 RetVal = 0;
	for (int32 i = ASC->GetActivatableAbilities().Num() - 1; i >= 0; --i)
	{
		FGameplayAbilitySpec Spec = ASC->GetActivatableAbilities()[i];
		if (Spec.SourceObject == GetOwner()) // for Abilities we check the SourceObject since thats what they use. SourceObjects are expected to be correct when set on GiveAbility()
		{
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

void UAbilitySystemSetupComponent::UnPossessed()
{
	PendingAbilitiesToSync = GetAbilitySystemComponent()->GetActivatableAbilities();

	// This goes before Super so we can get the Controller before it unpossess and the Pawn's reference becomes null. If it was
	// null we can't do IsPlayerControlled() and GetAbilitySystemComponent() would return the wrong ASC so the functions that we are calling would
	// probably act really weird and try doing stuff on the wrong ASC

	const bool bAIWithoutASC = (OwningPawn->IsPlayerControlled() == false && !IsValid(AIAbilitySystemComponent));
	if (bAIWithoutASC == false) // if we were a Player with an ASC or we were an AI with an ASC
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
#pragma endregion
