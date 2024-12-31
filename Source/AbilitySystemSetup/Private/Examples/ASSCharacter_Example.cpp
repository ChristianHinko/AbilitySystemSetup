// Fill out your copyright notice in the Description page of Project Settings.

#include "Examples/ASSCharacter_Example.h"

#include "ActorComponents/ASSActorComponent_PawnAvatarActorExtension.h"
#include "GameFramework/PlayerState.h"

AASSCharacter_Example::AASSCharacter_Example(const FObjectInitializer& objectInitializer)
    : Super(objectInitializer)
{
    // Create the pawn avatar actor extension component to assist in setting us up with the ASC.
    PawnAvatarActorExtensionComponent = CreateDefaultSubobject<UASSActorComponent_PawnAvatarActorExtension>(TEXT("PawnAvatarActorExtensionComponent"));
}

void AASSCharacter_Example::PreInitializeComponents()
{
    Super::PreInitializeComponents();

    PawnAvatarActorExtensionComponent->RemoveLooseAvatarRelatedTagsDelegate.AddUObject(this, &ThisClass::OnRemoveLooseAvatarRelatedTags);
}

UAbilitySystemComponent* AASSCharacter_Example::GetAbilitySystemComponent() const
{
    IAbilitySystemInterface* abilitySystemInterface = Cast<IAbilitySystemInterface>(GetPlayerState());
    if (!abilitySystemInterface)
    {
        return nullptr;
    }

    return abilitySystemInterface->GetAbilitySystemComponent();
}

void AASSCharacter_Example::PossessedBy(AController* newController)
{
    Super::PossessedBy(newController);

    PawnAvatarActorExtensionComponent->OnOwnerControllerChanged(); // This doesn't do anything in this case because our ASC is not initialized yet but we are calling it anyways for consistency. However, if your ASC were on the Character, then this is actually necessary in order for actor info to be updated correctly.

    UAbilitySystemComponent* asc = GetAbilitySystemComponent();
    check(asc);
    PawnAvatarActorExtensionComponent->InitializeAbilitySystemComponent(*asc);
}

void AASSCharacter_Example::UnPossessed()
{
    PawnAvatarActorExtensionComponent->UninitializeAbilitySystemComponent();

    Super::UnPossessed();

    PawnAvatarActorExtensionComponent->OnOwnerControllerChanged(); // This doesn't do anything in this case because our ASC is uninitialized but we are calling it anyways for consistency. However, if your ASC were on the Character, then this is actually necessary in order for actor info to be updated correctly.
}

void AASSCharacter_Example::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    if (IsPlayerControlled()) // We don't setup for AIs on the client.
    {
        if (GetPlayerState())
        {
            // We have a player state so initialize its ASC.
            UAbilitySystemComponent* asc = GetAbilitySystemComponent();
            check(asc);
            PawnAvatarActorExtensionComponent->InitializeAbilitySystemComponent(*asc);
        }
        else
        {
            // No player state so uninitialize the ASC.
            PawnAvatarActorExtensionComponent->UninitializeAbilitySystemComponent();
        }
    }
}

void AASSCharacter_Example::OnRep_Controller()
{
    Super::OnRep_Controller();

    PawnAvatarActorExtensionComponent->OnOwnerControllerChanged();
}

void AASSCharacter_Example::OnRemoveLooseAvatarRelatedTags(UAbilitySystemComponent& asc)
{
#if 0
    ASC.SetLooseGameplayTagCount(Tag_MovementModeSwimming, 0); // Sets swimming tag count to 0.
    ASC.RemoveLooseGameplayTag(Tag_MovementModeSwimming); // Decrements swimming tag count.
    ASC.RemoveLooseGameplayTag(Tag_MovementModeSwimming, 2); // Decrements swimming tag count by 2.
#endif // #if 0
}

void AASSCharacter_Example::SetupPlayerInputComponent(UInputComponent* playerInputComponent)
{
    Super::SetupPlayerInputComponent(playerInputComponent);

    check(playerInputComponent);
    PawnAvatarActorExtensionComponent->OnOwnerSetupPlayerInputComponent(*playerInputComponent);
}

void AASSCharacter_Example::DestroyPlayerInputComponent()
{
    Super::DestroyPlayerInputComponent();

    PawnAvatarActorExtensionComponent->OnOwnerDestroyPlayerInputComponent();
}
