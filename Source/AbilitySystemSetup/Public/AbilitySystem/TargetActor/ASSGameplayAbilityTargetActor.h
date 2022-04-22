// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "AbilitySystem/TargetActor/ASSGameplayAbilityTargetDataFilter.h"
#include "AbilitySystem/TargetActor/ASSGameplayAbilityWorldReticle.h"

#include "ASSGameplayAbilityTargetActor.generated.h"


class AASSGameplayAbilityWorldReticle;



/**
 * Base Target Actor class.
 * 
 * Can be disabled and re-enabled across mutliple Ability Task activations.
 * Provides helpful aiming direction functions using the StartLocation and the Player Controller.
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
	 * If true, when a trace overlaps an Actor's multiple collisions, those multiple collision hits can add
	 * that Actor to the Hit Results multiple times.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Target Data")
		bool bAllowMultipleHitsPerActor;


	/**
	 * Filter out hit results that do not pass filter and removes multiple hits per actor if needed
	 */
	void FilterHitResults(TArray<FHitResult>& OutHitResults, const FGameplayTargetDataFilterHandle& FilterHandle, const bool inAllowMultipleHitsPerActor) const;
	/**
	 * Filters out one hit result out of a given array. Is meant to be use in FHitResult loops.
	 * Returns true if hit was filtered.
	 */
	bool FilterHitResult(TArray<FHitResult>& OutHitResults, const int32 IndexToTryToFilter, const FGameplayTargetDataFilterHandle& FilterHandle, const bool inAllowMultipleHitsPerActor) const;
	/**
	 * Returns true if hit does not pass the filter.
	 * Does NOT remove the hit from the given HitResults.
	 */	
	bool WouldHitResultGetFiltered(const TArray<FHitResult>& InHitResults, const int32 IndexToTryToFilter, const FGameplayTargetDataFilterHandle& FilterHandle, const bool inAllowMultipleHitsPerActor) const;


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

	/** If true, sets StartLocation to the AimPoint determined in CalculateAimDirection() */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Trace")
		bool bUseAimPointAsStartLocation;

	/** Outputs the direction which our StartLocation is aiming - towards our Player's aiming endpoint */
	FVector GetAimDirectionOfStartLocation() const;

protected:
	virtual void StartTargeting(UGameplayAbility* Ability) override;
	virtual void ConfirmTargetingAndContinue() override;

	/**
	 * Calculates AimDir which is used in GetAimDirectionOfStartLocation().
	 * This can be overriden to add bullet spread for guns and stuff.
	 */
	virtual void CalculateAimDirection(FVector& OutAimStart, FVector& OutAimDir) const;


	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	/** List of custom Reticle actors */
	TArray<TWeakObjectPtr<AASSGameplayAbilityWorldReticle>> ReticleActors;

	AASSGameplayAbilityWorldReticle* SpawnReticleActor(const FVector& Location, const FRotator& Rotation);
	virtual void DestroyReticleActors();

};
