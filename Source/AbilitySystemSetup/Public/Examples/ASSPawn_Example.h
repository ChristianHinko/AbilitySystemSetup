// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemInterface.h"

#include "ASSPawn_Example.generated.h"


class UASSActorComponent_PawnAvatarActorExtension;
class UAbilitySystemComponent;



/**
 * Example implementation of a pawn initializing with an ASC by using the UASSActorComponent_PawnAvatarActorExtension.
 * Feel free to subclass if lazy.
 */
UCLASS()
class ABILITYSYSTEMSETUP_API AASSPawn_Example : public APawn, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    AASSPawn_Example(const FObjectInitializer& objectInitializer);

protected:

    virtual void PreInitializeComponents();

    //  BEGIN APawn Interface
    virtual void PossessedBy(AController* newController) override;
    virtual void UnPossessed() override;
    virtual void OnRep_PlayerState() override;
    virtual void OnRep_Controller() override;
    virtual void SetupPlayerInputComponent(UInputComponent* playerInputComponent) override;
    virtual void DestroyPlayerInputComponent() override;
    //  END APawn Interface

    //  BEGIN AvatarExtensionDelegate
    virtual void OnRemoveLooseAvatarRelatedTags(UAbilitySystemComponent& asc);
    //  END AvatarExtensionDelegate

public:

    UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:

    UPROPERTY(VisibleAnywhere, Category = "AbilitySystem")
    TObjectPtr<UASSActorComponent_PawnAvatarActorExtension> PawnAvatarActorExtensionComponent;

};
