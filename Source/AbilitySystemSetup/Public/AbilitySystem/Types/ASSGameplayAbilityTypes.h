// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"

#include "ASSGameplayAbilityTypes.generated.h"


class UASSAbilitySystemComponent;



/**
 * Our base GameplayAbilityActorInfo.
 * Put non-game-specific data in here - like base classes and stuff
 */
USTRUCT()
struct ABILITYSYSTEMSETUP_API FASSGameplayAbilityActorInfo : public FGameplayAbilityActorInfo
{
	GENERATED_BODY()

	/** The Controller associated with the owning actor. Shouldn't be null */
	UPROPERTY(BlueprintReadOnly, Category = "ASSActorInfo")
		TWeakObjectPtr<AController> Controller;

	/** Our ASC. Shouldn't be null. */
	UPROPERTY(BlueprintReadOnly, Category = "ASSActorInfo")
		TWeakObjectPtr<UASSAbilitySystemComponent> ASSAbilitySystemComponent;


	virtual void InitFromActor(AActor* OwnerActor, AActor* AvatarActor, UAbilitySystemComponent* InAbilitySystemComponent) override;
	virtual void ClearActorInfo() override;
};
