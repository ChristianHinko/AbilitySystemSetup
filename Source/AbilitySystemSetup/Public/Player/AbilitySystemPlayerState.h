// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/ASSAbilitySystemComponent.h"

#include "AbilitySystemPlayerState.generated.h"



/**
 * GAS Player State
 */
UCLASS()
class ABILITYSYSTEMSETUP_API AAbilitySystemPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, Category = "AbilitySystem")
		UASSAbilitySystemComponent* ASSAbilitySystemComponent;
	static FName AbilitySystemComponentName;

public:
	AAbilitySystemPlayerState(const FObjectInitializer& ObjectInitializer);


	UASSAbilitySystemComponent* GetAbilitySystemComponent() const override { return ASSAbilitySystemComponent; }

};
