// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "ASSGameplayAbilityWorldReticle.h"

#include "ASSGameplayAbilityTargetActor.generated.h"

class AASSGameplayAbilityWorldReticle;

/**
 * Base Target Actor class.
 *
 * Can be disabled and re-enabled across multiple Ability Task activations.
 * Has array of spawned World Reticles.
 */
UCLASS(Abstract, notplaceable)
class ABILITYSYSTEMSETUP_API AASSGameplayAbilityTargetActor : public AGameplayAbilityTargetActor
{
    GENERATED_BODY()

public:
    AASSGameplayAbilityTargetActor(const FObjectInitializer& ObjectInitializer);


    /** Our custom World Reticle Parameters */
    UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = "Targeting")
        FASSWorldReticleParameters ASSReticleParams;


    virtual void StartTargeting(UGameplayAbility* Ability) override;

    /**
     * Called when we are done being used by the Ability Task.
     * Use StartTargeting() to re-enable. This is helpful for re-using Target Actors across multiple Ability Task activations.
     * Must have bDestroyOnConfirmation = false to use this.
     */
    virtual void DisableTargetActor();

protected:
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


    AASSGameplayAbilityWorldReticle* SpawnWorldReticle(const FVector& InLocation, const FRotator& InRotation);

    /** List of spawned World Reticle actors */
    UPROPERTY(Transient)
        TArray<TObjectPtr<AGameplayAbilityWorldReticle>> SpawnedWorldReticles;

    virtual void DestroyWorldReticles();
};
