// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"

#include "ASSActor_Example.generated.h"

class UAbilitySystemComponent;
class UASSActorComponent_AvatarActorExtension;

/**
 * @brief Example implementation of an Actor initializing with an ASC by using
 *        the `UASSActorComponent_AvatarActorExtension`. Feel free to subclass if lazy.
 */
UCLASS()
class ABILITYSYSTEMSETUP_API AASSActor_Example : public AActor, public IAbilitySystemInterface
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

protected:

    UPROPERTY(EditAnywhere, Category = "AbilitySystemSetup")
    TObjectPtr<UASSActorComponent_AvatarActorExtension> AvatarActorExtensionComponent = nullptr;

    UPROPERTY(EditAnywhere, Category = "AbilitySystemSetup")
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;
};
