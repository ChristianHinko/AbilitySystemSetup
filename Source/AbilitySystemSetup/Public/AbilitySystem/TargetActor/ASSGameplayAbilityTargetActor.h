// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "AbilitySystem/TargetActor/ASSGameplayAbilityWorldReticle.h"

#include "ASSGameplayAbilityTargetActor.generated.h"


class AASSGameplayAbilityWorldReticle;



/**
 * Base Target Actor class.
 * 
 * Can be disabled and re-enabled across mutliple Ability Task activations.
 * Has array of spawned Reticle Actors.
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


	/**
	 * Called when we are done being used by the Ability Task.
	 * Use StartTargeting() to re-enable. This is helpful for re-using Target Actors across multiple Ability Task activations.
	 * Must have bDestroyOnConfirmation = false to use this.
	 */
	virtual void DisableTargetActor();


	/** Max range of this target actor (not required for all target actors) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Trace")
		float MaxRange;

	/** Trace channel for this target actor (not required for all target actors) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Trace")
		TEnumAsByte<ECollisionChannel> TraceChannel;

protected:
	virtual void StartTargeting(UGameplayAbility* Ability) override;
	virtual void ConfirmTargetingAndContinue() override;


	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	/** List of spawned World Reticle actors */
	UPROPERTY()
		TArray<TObjectPtr<AGameplayAbilityWorldReticle>> SpawnedWorldReticles;

	AASSGameplayAbilityWorldReticle* SpawnReticleActor(const FVector& Location, const FRotator& Rotation);
	virtual void DestroyWorldReticles();

};
