// Fill out your copyright notice in the Description page of Project Settings.

#include "Examples/ASSPawn_Example.h"

#include "ASSPawnAvatarActorExtensionStruct.h"
#include "GameFramework/PlayerState.h"

void AASSPawn_Example::PreInitializeComponents()
{
    Super::PreInitializeComponents();

    PawnAvatarActorExtensionComponent.RemoveLooseAvatarRelatedTagsDelegate.AddUObject(this, &ThisClass::OnRemoveLooseAvatarRelatedTags);
}

void AASSPawn_Example::EndPlay(const EEndPlayReason::Type endPlayReason)
{
    Super::EndPlay(endPlayReason);

    PawnAvatarActorExtensionComponent.OnAvatarActorBeginDestroy();
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

    PawnAvatarActorExtensionComponent.OnAvatarActorControllerChanged(*this); // This doesn't do anything in this case because our ASC is not initialized yet but we are calling it anyways for consistency. However, if your ASC were on the Character, then this is actually necessary in order for actor info to be updated correctly.

    UAbilitySystemComponent* asc = GetAbilitySystemComponent();
    check(asc);
    PawnAvatarActorExtensionComponent.InitializeAbilitySystemComponent(*asc, *this);
}

void AASSPawn_Example::UnPossessed()
{
    PawnAvatarActorExtensionComponent.UninitializeAbilitySystemComponent(*this);

    Super::UnPossessed();

    PawnAvatarActorExtensionComponent.OnAvatarActorControllerChanged(*this); // This doesn't do anything in this case because our ASC is uninitialized but we are calling it anyways for consistency. However, if your ASC were on the Character, then this is actually necessary in order for actor info to be updated correctly.
}

void AASSPawn_Example::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    if (IsPlayerControlled()) // We don't setup for AIs on the client.
    {
        if (GetPlayerState())
        {
            // We have a player state so initialize its ASC.
            UAbilitySystemComponent* asc = GetAbilitySystemComponent();
            check(asc);
            PawnAvatarActorExtensionComponent.InitializeAbilitySystemComponent(*asc, *this);
        }
        else
        {
            // No player state so uninitialize the ASC.
            PawnAvatarActorExtensionComponent.UninitializeAbilitySystemComponent(*this);
        }
    }
}

void AASSPawn_Example::OnRep_Controller()
{
    Super::OnRep_Controller();

    PawnAvatarActorExtensionComponent.OnAvatarActorControllerChanged(*this);
}

void AASSPawn_Example::OnRemoveLooseAvatarRelatedTags(UAbilitySystemComponent& asc)
{
#if 0
    ASC.SetLooseGameplayTagCount(Tag_MovementModeSwimming, 0); // Sets swimming tag count to 0.
    ASC.RemoveLooseGameplayTag(Tag_MovementModeSwimming); // Decrements swimming tag count.
    ASC.RemoveLooseGameplayTag(Tag_MovementModeSwimming, 2); // Decrements swimming tag count by 2.
#endif // #if 0
}

void AASSPawn_Example::SetupPlayerInputComponent(UInputComponent* playerInputComponent)
{
    Super::SetupPlayerInputComponent(playerInputComponent);

    check(playerInputComponent);
    PawnAvatarActorExtensionComponent.OnAvatarActorSetupPlayerInputComponent(*playerInputComponent, *this);
}

void AASSPawn_Example::DestroyPlayerInputComponent()
{
    Super::DestroyPlayerInputComponent();

    PawnAvatarActorExtensionComponent.OnAvatarActorDestroyPlayerInputComponent();
}
