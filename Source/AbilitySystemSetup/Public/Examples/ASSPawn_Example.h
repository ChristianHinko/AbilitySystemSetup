// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemInterface.h"

#include "ASSPawn_Example.generated.h"

class UASSActorComponent_PawnAvatarActorExtension;
class UAbilitySystemComponent;

/**
 * @brief An example implementation of a pawn initializing with an ASC by using
 *        the `UASSActorComponent_PawnAvatarActorExtension`. Feel free to subclass if lazy.
 */
UCLASS()
class ABILITYSYSTEMSETUP_API AASSPawn_Example : public APawn, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:

    AASSPawn_Example(const FObjectInitializer& objectInitializer);

public:

    // ~ IAbilitySystemInterface overrides.
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    // ~ IAbilitySystemInterface overrides.

protected:

    // ~ AActor overrides.
    virtual void PreInitializeComponents();
    // ~ AActor overrides.

    // ~ APawn overrides.
    virtual void PossessedBy(AController* newController) override;
    virtual void UnPossessed() override;
    virtual void OnRep_PlayerState() override;
    virtual void OnRep_Controller() override;
    virtual void SetupPlayerInputComponent(UInputComponent* playerInputComponent) override;
    virtual void DestroyPlayerInputComponent() override;
    // ~ APawn overrides.

    // ~ AvatarActorExtension delegate callbacks.
    virtual void OnRemoveLooseAvatarRelatedTags(UAbilitySystemComponent& asc);
    // ~ AvatarActorExtension delegate callbacks.

protected:

    UPROPERTY(EditAnywhere, Category = "AbilitySystemSetup")
    TObjectPtr<UASSActorComponent_PawnAvatarActorExtension> PawnAvatarActorExtensionComponent = nullptr;
};
