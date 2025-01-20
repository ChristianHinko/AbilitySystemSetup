// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "ASSAvatarActorExtensionStruct.h"
#include "ASSAvatarActorExtentionInterface.h"

#include "ASSActor_Example.generated.h"

class UAbilitySystemComponent;

/**
 * @brief Example implementation of an Actor initializing with an ASC by using
 *        the `FASSAvatarActorExtensionStruct`. Feel free to subclass if lazy.
 */
UCLASS()
class ABILITYSYSTEMSETUP_API AASSActor_Example : public AActor,
                                                 public IAbilitySystemInterface,
                                                 public IASSAvatarActorExtentionInterface
{
    GENERATED_BODY()

public:

    AASSActor_Example(const FObjectInitializer& inObjectInitializer);

public:

    // ~ IAbilitySystemInterface overrides.
    UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
    // ~ IAbilitySystemInterface overrides.

protected:

    // ~ AActor overrides.
    virtual void PostInitializeComponents() override;
    virtual void EndPlay(const EEndPlayReason::Type inEndPlayReason) override;
    // ~ AActor overrides.

    // ~ IASSAvatarActorExtentionInterface overrides.
    FORCEINLINE virtual FASSAvatarActorExtensionStruct& GetASSAvatarActorExtension() override { return AvatarActorExtensionComponent; }
    // ~ IASSAvatarActorExtentionInterface overrides.

protected:

    UPROPERTY(EditAnywhere, Category = "AbilitySystemSetup")
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;
    
    // Create the avatar actor extension component to assist in setting us up with the ASC.
    UPROPERTY(EditAnywhere, Category = "AbilitySystemSetup")
    FASSAvatarActorExtensionStruct AvatarActorExtensionComponent;
};
