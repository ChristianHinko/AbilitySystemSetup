// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "ASSPawnAvatarActorExtentionInterface.h"
#include "ActorComponents/ASSActorComponent_PawnAvatarActorExtension.h"

#include "ASSCharacter_Example.generated.h"

class UAbilitySystemComponent;

/**
 * @brief An example implementation of a character initializing with an ASC by using
 *        the `FASSActorComponent_PawnAvatarActorExtension`. Feel free to subclass if lazy.
 */
UCLASS()
class ABILITYSYSTEMSETUP_API AASSCharacter_Example : public ACharacter,
                                                     public IAbilitySystemInterface,
                                                     public IASSPawnAvatarActorExtentionInterface
{
    GENERATED_BODY()

protected:
    
    // ~ AActor overrides.
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;
    // ~ AActor overrides.

public:

    // ~ IAbilitySystemInterface overrides.
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    // ~ IAbilitySystemInterface overrides.

    // ~ IASSPawnAvatarActorExtentionInterface overrides.
    FORCEINLINE virtual FASSActorComponent_PawnAvatarActorExtension& GetASSAvatarActorExtension() override { return PawnAvatarActorExtensionComponent; }
    // ~ IASSPawnAvatarActorExtentionInterface overrides.

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
    
    // Create the avatar actor extension component to assist in setting us up with the ASC.
    UPROPERTY(EditAnywhere, Category = "AbilitySystemSetup")
    FASSActorComponent_PawnAvatarActorExtension PawnAvatarActorExtensionComponent;
};
