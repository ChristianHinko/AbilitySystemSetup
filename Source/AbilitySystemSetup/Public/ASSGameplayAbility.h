// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"

#include "ASSGameplayAbility.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogASSGameplayAbility, Log, All)

/**
 * Base Gameplay Ability
 */
UCLASS()
class ABILITYSYSTEMSETUP_API UASSGameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UASSGameplayAbility(const FObjectInitializer& ObjectInitializer);


    /** Passive abilities are auto activated on given */
    uint8 bPassiveAbility : 1;


    //  BEGIN UGameplayAbility interface
    virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override final; // final since we use our own event to fix the engine's
    virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override final;
    //  END UGameplayAbility interface


    /** An exposed EndAbility() that isn't a cancellation. Used for ability batching. */
    virtual void ExternalEndAbility();

protected:
    /**
     * The engine's OnAvatarSet() is not called properly for instanced abilities! When the avatar actor changes, the
     * event is mistakingly called on the CDO.
     * This version correctly gets called on instanced abilities when the avatar actor changes.
     */
    virtual void ASSOnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec);

    /**
     * The engine's EndAbility() is not safe to override out of the box. There are several checks that must be done before
     * using it as an event.
     * This version is called at a safe point for ability subclass logic to use as an event.
     */
    virtual void ASSEndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled);


private:
    void TryActivatePassiveAbility(const FGameplayAbilityActorInfo* InActorInfo, const FGameplayAbilitySpec& InSpec) const;
};
