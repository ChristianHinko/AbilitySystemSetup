// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AbilitySystemSetupCharacter.h"

#include "AbilitySystemSetupComponent/AbilitySystemSetupComponent.h"
#include "Player/AbilitySystemPlayerState.h"
#include "DS_AbilitySystemSetup.h"
#include "Utilities/ASSLogCategories.h"



//void AAbilitySystemSetupCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//
//	//DOREPLIFETIME(AAbilitySystemSetupCharacter, PlayerAbilitySystemComponent);			// can be helpful for debugging
//}

AAbilitySystemSetupCharacter::AAbilitySystemSetupCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemSetup = CreateDefaultSubobject<UAbilitySystemSetupComponent>(TEXT("AbilitySystemSetup"));
}


UASSAbilitySystemComponent* AAbilitySystemSetupCharacter::GetAbilitySystemComponent() const
{
	if (IsPlayerControlled())
	{
		return PlayerAbilitySystemComponent;
	}
	else // AI controlled
	{
		return AbilitySystemSetup->GetAIAbilitySystemComponent();
	}
}

#pragma region Ability System Possess
void AAbilitySystemSetupCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);


	if (IsPlayerControlled())
	{
		AbilitySystemPlayerState = GetPlayerState<AAbilitySystemPlayerState>();
		PlayerAbilitySystemComponent = AbilitySystemPlayerState->GetAbilitySystemComponent();

		AbilitySystemSetup->SetupWithAbilitySystemPlayerControlled(AbilitySystemPlayerState);
	}
	else // AI Controlled
	{
		AbilitySystemSetup->SetupWithAbilitySystemAIControlled();
	}
}

void AAbilitySystemSetupCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Make sure its not when unpossessing (when unpossessed, Player State is null)
	if (GetPlayerState())
	{
		if (IsPlayerControlled())
		{
			AbilitySystemPlayerState = GetPlayerState<AAbilitySystemPlayerState>();
			PlayerAbilitySystemComponent = AbilitySystemPlayerState->GetAbilitySystemComponent();

			AbilitySystemSetup->SetupWithAbilitySystemPlayerControlled(AbilitySystemPlayerState);
		}
	}
}

void AAbilitySystemSetupCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();


	if (GetAbilitySystemComponent())
	{
		GetAbilitySystemComponent()->RefreshAbilityActorInfo();		// Kaos said to do this here, not really sure what it's for, but might be for AIs? Also noticed ArcInventory example project does it
	}
}
#pragma endregion

#pragma region Input
void AAbilitySystemSetupCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);


	const UDS_AbilitySystemSetup* AbilitySystemSetupDeveloperSettings = GetDefault<UDS_AbilitySystemSetup>();
	if (IsValid(AbilitySystemSetupDeveloperSettings) == false)
	{
		UE_LOG(LogAbilitySystemInputEnumMappingsSafetyChecks, Fatal, TEXT("%s() No valid pointer to UDS_AbilitySystemSetup when trying to get the name of the confirm and cancel input action names."), *FString(__FUNCTION__), *GetName());
	}

	PlayerInputComponent->BindAction(FName(AbilitySystemSetupDeveloperSettings->ConfirmTargetInputActionName), IE_Pressed, this, &AAbilitySystemSetupCharacter::OnConfirmTargetPressed);
	PlayerInputComponent->BindAction(FName(AbilitySystemSetupDeveloperSettings->ConfirmTargetInputActionName), IE_Released, this, &AAbilitySystemSetupCharacter::OnConfirmTargetReleased);

	PlayerInputComponent->BindAction(FName(AbilitySystemSetupDeveloperSettings->CancelTargetInputActionName), IE_Pressed, this, &AAbilitySystemSetupCharacter::OnCancelTargetPressed);
	PlayerInputComponent->BindAction(FName(AbilitySystemSetupDeveloperSettings->CancelTargetInputActionName), IE_Released, this, &AAbilitySystemSetupCharacter::OnCancelTargetReleased);


	// Bind Player input to the AbilitySystemComponent. Also called in OnRep_PlayerState() because of a potential race condition.
	AbilitySystemSetup->BindASCInput(PlayerInputComponent);
}
//----------------------------------------------------------------------------- \/\/\/\/ EVENTS \/\/\/\/ ------------------------


void AAbilitySystemSetupCharacter::OnConfirmTargetPressed()
{
	GetAbilitySystemComponent()->LocalInputConfirm();
}
void AAbilitySystemSetupCharacter::OnConfirmTargetReleased()
{
}

void AAbilitySystemSetupCharacter::OnCancelTargetPressed()
{
	GetAbilitySystemComponent()->LocalInputCancel();
}
void AAbilitySystemSetupCharacter::OnCancelTargetReleased()
{
}
#pragma endregion

#pragma region ASC Setup Helpers
void AAbilitySystemSetupCharacter::CreateAttributeSets()
{
	//Super::CreateAttributeSets();

#if 0
	if (!MyAttributeSet)
	{
		MyAttributeSet = NewObject<UAS_MyAttributeSet>(this, UAS_MyAttributeSet::StaticClass(), TEXT("MyAttributeSet"));
	}
	else
	{
		UE_CLOG((GetLocalRole() == ROLE_Authority), LogTemp, Warning, TEXT("%s() %s was already valid when trying to create the attribute set; did nothing"), *FString(__FUNCTION__), *(MyAttributeSet->GetName()));
	}

	if (!MyOtherAttributeSet)
	{
		MyOtherAttributeSet = NewObject<UAS_MyOtherAttributeSet>(this, UAS_MyOtherAttributeSet::StaticClass(), TEXT("MyOtherAttributeSet"));
	}
	else
	{
		UE_CLOG((GetLocalRole() == ROLE_Authority), LogTemp, Warning, TEXT("%s() %s was already valid when trying to create the attribute set; did nothing"), *FString(__FUNCTION__), *(MyOtherAttributeSet->GetName()));
	}
#endif
}
void AAbilitySystemSetupCharacter::RegisterAttributeSets()
{
	//Super::RegisterAttributeSets();

#if 0
	if (MyAttributeSet && !GetAbilitySystemComponent()->GetSpawnedAttributes().Contains(MyAttributeSet))
	{
		GetAbilitySystemComponent()->AddAttributeSetSubobject(MyAttributeSet);
	}
	else
	{
		UE_CLOG((GetLocalRole() == ROLE_Authority), LogTemp, Warning, TEXT("%s() MyAttributeSet was either NULL or already added to the character's ASC. Character: %s"), *FString(__FUNCTION__), *GetName());
	}

	if (MyOtherAttributeSet && !GetAbilitySystemComponent()->GetSpawnedAttributes().Contains(MyOtherAttributeSet))
	{
		GetAbilitySystemComponent()->AddAttributeSetSubobject(MyOtherAttributeSet);
	}
	else
	{
		UE_CLOG((GetLocalRole() == ROLE_Authority), LogTemp, Warning, TEXT("%s() MyOtherAttributeSet was either NULL or already added to the character's ASC. Character: %s"), *FString(__FUNCTION__), *GetName());
	}
#endif
}

void AAbilitySystemSetupCharacter::GrantStartingAbilities()
{
	//Super::GrantStartingAbilities();

#if 0
	MyAbilitySpecHandle = GetAbilitySystemComponent()->GrantAbility(MyAbilityTSub, this/*, GetLevel()*/);
	MyOtherAbilitySpecHandle = GetAbilitySystemComponent()->GrantAbility(MyOtherAbilityTSub, this/*, GetLevel()*/);
#endif
}
#pragma endregion


void AAbilitySystemSetupCharacter::UnPossessed()
{
	AbilitySystemSetup->UnPossessed(); // called BEFORE we call the Super


	// Actual unpossession happens here
	Super::UnPossessed();
}
