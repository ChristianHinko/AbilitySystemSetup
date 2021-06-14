// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"

#include "ASSGameplayAbilityTypes.generated.h"


class UASSAbilitySystemComponent;



DECLARE_MULTICAST_DELEGATE(FAbilityActorInfoState)


/**
 * Our base GameplayAbilityActorInfo.
 * Put non-game-specific data in here - like base classes and stuff
 */
USTRUCT()
struct ABILITYSYSTEMSETUP_API FASSGameplayAbilityActorInfo : public FGameplayAbilityActorInfo
{
	GENERATED_BODY()


    FASSGameplayAbilityActorInfo();
    virtual ~FASSGameplayAbilityActorInfo()
    {

    }

    // Our ASC. Should NEVER be null.
    UPROPERTY(BlueprintReadOnly, Category = "ASSActorInfo")
        TWeakObjectPtr<UASSAbilitySystemComponent> ASSAbilitySystemComponent;


    /**
     * Broadcast this at the end of your InitFromActor().
     * The reason this base class can't is because it would be done in the Super call and wouldn't be done after the subclass initialization.
     */
    FAbilityActorInfoState OnInited;

    virtual void InitFromActor(AActor* OwnerActor, AActor* AvatarActor, UAbilitySystemComponent* InAbilitySystemComponent) override;
    virtual void SetAvatarActor(AActor* AvatarActor) override;
    virtual void ClearActorInfo() override;

};
