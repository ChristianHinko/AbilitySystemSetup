// Fill out your copyright notice in the Description page of Project Settings.


#include "Examples/ASSCharacter_Example.h"

#include "Subobjects/ASSActorComponent_PawnAvatarActorExtension.h"
#include "GameFramework/PlayerState.h"



AASSCharacter_Example::AASSCharacter_Example(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Create the pawn avatar actor extension component to assist in setting us up with the ASC
	PawnAvatarActorExtensionComponent = CreateDefaultSubobject<UASSActorComponent_PawnAvatarActorExtension>(TEXT("PawnAvatarActorExtensionComponent"));
}

void AASSCharacter_Example::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	PawnAvatarActorExtensionComponent->RemoveLooseAvatarRelatedTagsDelegate.AddUObject(this, &AASSCharacter_Example::OnRemoveLooseAvatarRelatedTags);
}

UAbilitySystemComponent* AASSCharacter_Example::GetAbilitySystemComponent() const
{
	IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(GetPlayerState());
	if (!AbilitySystemInterface)
	{
		return nullptr;
	}

	return AbilitySystemInterface->GetAbilitySystemComponent();
}

void AASSCharacter_Example::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	PawnAvatarActorExtensionComponent->OnOwnerControllerChanged(); // this doesn't do anything in this case because our ASC is not initialized yet but we are calling it anyways for consistency. However, if your ASC were on the Character, then this is actually necessary in order for actor info to be updated correctly

	PawnAvatarActorExtensionComponent->InitializeAbilitySystemComponent(GetAbilitySystemComponent());
}
void AASSCharacter_Example::UnPossessed()
{
	PawnAvatarActorExtensionComponent->UninitializeAbilitySystemComponent();


	Super::UnPossessed();
	PawnAvatarActorExtensionComponent->OnOwnerControllerChanged(); // this doesn't do anything in this case because our ASC is uninitialized but we are calling it anyways for consistency. However, if your ASC were on the Character, then this is actually necessary in order for actor info to be updated correctly
}

void AASSCharacter_Example::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();


	if (IsPlayerControlled())	// we don't setup for AIs on the client
	{
		if (IsValid(GetPlayerState()))
		{
			// We have a Player State so initialize its ASC
			PawnAvatarActorExtensionComponent->InitializeAbilitySystemComponent(GetAbilitySystemComponent());
		}
		else
		{
			// No Player State so uninitialize the ASC
			PawnAvatarActorExtensionComponent->UninitializeAbilitySystemComponent();
		}
	}
}

void AASSCharacter_Example::OnRep_Controller()
{
	Super::OnRep_Controller();
	PawnAvatarActorExtensionComponent->OnOwnerControllerChanged();
}


void AASSCharacter_Example::OnRemoveLooseAvatarRelatedTags(UAbilitySystemComponent* ASC)
{
#if 0
	ASC->SetLooseGameplayTagCount(Tag_MovementModeSwimming, 0);	// sets swimming tag count to 0
	ASC->RemoveLooseGameplayTag(Tag_MovementModeSwimming);		// decrements swimming tag count
	ASC->RemoveLooseGameplayTag(Tag_MovementModeSwimming, 2);	// decrements swimming tag count by 2
#endif
}


void AASSCharacter_Example::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PawnAvatarActorExtensionComponent->OnOwnerSetupPlayerInputComponent(PlayerInputComponent);
}
void AASSCharacter_Example::DestroyPlayerInputComponent()
{
	Super::DestroyPlayerInputComponent();

	PawnAvatarActorExtensionComponent->OnOwnerDestroyPlayerInputComponent();
}
