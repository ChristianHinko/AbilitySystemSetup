// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"

#include "ASSGameplayAbilityTypes.generated.h"


class UASSAbilitySystemComponent;



DECLARE_MULTICAST_DELEGATE(FActorInfoStatus)


/**
 * Our base GameplayAbilityActorInfo.
 * Put non-game-specific data in here - like base classes and stuff
 */
USTRUCT()
struct ABILITYSYSTEMSETUP_API FASSGameplayAbilityActorInfo : public FGameplayAbilityActorInfo
{
	GENERATED_BODY()


    FASSGameplayAbilityActorInfo();
    virtual ~FASSGameplayAbilityActorInfo() override
    {

    }

    /** Fired when this Actor Info gets initted */
    FActorInfoStatus& GetOnInittedDelegate() { return OnInitted; }



    // Our ASC. Should NEVER be null.
    UPROPERTY(BlueprintReadOnly, Category = "ASSActorInfo")
        TWeakObjectPtr<UASSAbilitySystemComponent> ASSAbilitySystemComponent;


    /** Marked as final. Use ASSInitFromActor() instead. */
    virtual void InitFromActor(AActor* OwnerActor, AActor* AvatarActor, UAbilitySystemComponent* InAbilitySystemComponent) override final;
    /**
     * Override this instead of InitFromActor(). The reason InitFromActor() is marked as final is that this base class needs to know when we are
     * fully initted so that we can broadcast OnInitted. InitFromActor() just calls ASSInitFromActor() and OnInitted after it.
     */
    virtual void ASSInitFromActor(AActor* OwnerActor, AActor* AvatarActor, UAbilitySystemComponent* InAbilitySystemComponent);

    virtual void SetAvatarActor(AActor* AvatarActor) override;
    virtual void ClearActorInfo() override;


private:
    /** Fired at the end of InitFromActor() */
    FActorInfoStatus OnInitted;

};
