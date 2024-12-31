// Fill out your copyright notice in the Description page of Project Settings.


#include "Examples/ASSPawn_Example.h"

#include "Subobjects/ASSActorComponent_PawnAvatarActorExtension.h"
#include "GameFramework/PlayerState.h"

AASSPawn_Example::AASSPawn_Example(const FObjectInitializer& objectInitializer)
    : Super(objectInitializer)
{
    // Create the pawn avatar actor extension component to assist in setting us up with the ASC
    PawnAvatarActorExtensionComponent = CreateDefaultSubobject<UASSActorComponent_PawnAvatarActorExtension>(TEXT("PawnAvatarActorExtensionComponent"));
}

void AASSPawn_Example::PreInitializeComponents()
{
    Super::PreInitializeComponents();

    PawnAvatarActorExtensionComponent->RemoveLooseAvatarRelatedTagsDelegate.AddUObject(this, &AASSPawn_Example::OnRemoveLooseAvatarRelatedTags);
}

UAbilitySystemComponent* AASSPawn_Example::GetAbilitySystemComponent() const
{
    IAbilitySystemInterface* abilitySystemInterface = Cast<IAbilitySystemInterface>(GetPlayerState());
    if (!abilitySystemInterface)
    {
        return nullptr;
    }

    return abilitySystemInterface->GetAbilitySystemComponent();
}

void AASSPawn_Example::PossessedBy(AController* newController)
{
    Super::PossessedBy(newController);

    PawnAvatarActorExtensionComponent->OnOwnerControllerChanged(); // this doesn't do anything in this case because our ASC is not initialized yet but we are calling it anyways for consistency. However, if your ASC were on the Character, then this is actually necessary in order for actor info to be updated correctly

    UAbilitySystemComponent* asc = GetAbilitySystemComponent();
    check(asc);
    PawnAvatarActorExtensionComponent->InitializeAbilitySystemComponent(*asc);
}
void AASSPawn_Example::UnPossessed()
{
    PawnAvatarActorExtensionComponent->UninitializeAbilitySystemComponent();


    Super::UnPossessed();
    PawnAvatarActorExtensionComponent->OnOwnerControllerChanged(); // this doesn't do anything in this case because our ASC is uninitialized but we are calling it anyways for consistency. However, if your ASC were on the Character, then this is actually necessary in order for actor info to be updated correctly
}

void AASSPawn_Example::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();


    if (IsPlayerControlled())    // we don't setup for AIs on the client
    {
        if (IsValid(GetPlayerState()))
        {
            // We have a Player State so initialize its ASC
            UAbilitySystemComponent* asc = GetAbilitySystemComponent();
            check(asc);
            PawnAvatarActorExtensionComponent->InitializeAbilitySystemComponent(*asc);
        }
        else
        {
            // No Player State so uninitialize the ASC
            PawnAvatarActorExtensionComponent->UninitializeAbilitySystemComponent();
        }
    }
}

void AASSPawn_Example::OnRep_Controller()
{
    Super::OnRep_Controller();
    PawnAvatarActorExtensionComponent->OnOwnerControllerChanged();
}

void AASSPawn_Example::OnRemoveLooseAvatarRelatedTags(UAbilitySystemComponent& asc)
{
#if 0
    ASC.SetLooseGameplayTagCount(Tag_MovementModeSwimming, 0);    // sets swimming tag count to 0
    ASC.RemoveLooseGameplayTag(Tag_MovementModeSwimming);        // decrements swimming tag count
    ASC.RemoveLooseGameplayTag(Tag_MovementModeSwimming, 2);    // decrements swimming tag count by 2
#endif
}

void AASSPawn_Example::SetupPlayerInputComponent(UInputComponent* playerInputComponent)
{
    Super::SetupPlayerInputComponent(playerInputComponent);

    check(playerInputComponent);
    PawnAvatarActorExtensionComponent->OnOwnerSetupPlayerInputComponent(*playerInputComponent);
}
void AASSPawn_Example::DestroyPlayerInputComponent()
{
    Super::DestroyPlayerInputComponent();

    PawnAvatarActorExtensionComponent->OnOwnerDestroyPlayerInputComponent();
}
