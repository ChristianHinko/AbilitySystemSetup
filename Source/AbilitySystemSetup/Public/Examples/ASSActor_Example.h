// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/ASSAbilitySystemComponent.h"
#include "ASSActor_Example.generated.h"

class UASSActorComponent_AbilitySystemSetup;

/**
 * Example implementation of an Actor initializing with an ASC by using the UASSActor_Example.
 * Feel free to subclass if lazy.
 */
UCLASS()
class ABILITYSYSTEMSETUP_API AASSActor_Example : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(VisibleAnywhere, Category = "AbilitySystem")
		TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	UPROPERTY(VisibleAnywhere, Category = "AbilitySystem")
		TObjectPtr<UASSActorComponent_AbilitySystemSetup> AbilitySystemSetupComponent;

public:
	AASSActor_Example(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure)
		UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

protected:
	// BEGIN AActor Interface
	virtual void PostInitializeComponents() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// END AActor Interface
};
