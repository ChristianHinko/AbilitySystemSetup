// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "AbilitySystem/TargetActor/ASSGameplayAbilityTargetDataFilter.h"
#include "AbilitySystem/TargetActor/ASSGameplayAbilityWorldReticle.h"

#include "ASSGameplayAbilityTargetActor.generated.h"


class AASSGameplayAbilityWorldReticle;



/**
 * Base target actor class
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

	/** Our custom Gameplay Target Data Filter */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Target Data")
		FGTDF_MultiFilter MultiFilter;
	/**
	 * If true, when a trace overlaps an actor's multiple collisions, those multiple collision hits will add
	 * that actor to the hitresults multiple times.
	 * 
	 * TODO: add option for picking the hit with highest damage
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Target Data")
		bool bAllowMultipleHitsPerActor;


	/** Filter out hit results that do not pass filter and removes multiple hits per actor if needed */
	void FilterHitResults(TArray<FHitResult>& OutHitResults, const FGameplayTargetDataFilterHandle& FilterHandle, const bool inAllowMultipleHitsPerActor) const;
	/**
	 * Filters out one hit result out of a given array. Is meant to be use in FHitResult loops.
	 * Returns true if hit was filtered.
	 */
	bool FilterHitResult(TArray<FHitResult>& OutHitResults, const int32 IndexToTryFilter, const FGameplayTargetDataFilterHandle& FilterHandle, const bool inAllowMultipleHitsPerActor) const;
	/**
	 * Returns true if hit does not pass the filter.
	 * Does NOT remove the hit from the given HitResults.
	 */	
	bool HitResultFailsFilter(const TArray<FHitResult>& InHitResults, const int32 IndexToTryFilter, const FGameplayTargetDataFilterHandle& FilterHandle, const bool inAllowMultipleHitsPerActor) const;


	/**
	 * Called if this Target Actor is going to be reused at the end of Wait Target Data to disable this actor (good for reusing Target Actors across these task activations)
	 */
	virtual void StopTargeting();


	/** Max range of this target actor (not required for all target actors)		(We made this virual so we can just return GunAttributeSet->GetMaxRange() that way we don't have to bind to that attribute's delegate)*/
	virtual float GetMaxRange() const;

	/** Trace channel for this target actor (not required for all target actors) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Trace")
		TEnumAsByte<ECollisionChannel> TraceChannel;


	/** If true, sets StartLocation to the AimPoint determined in CalculateAimDirection() */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Trace")
		bool bUseAimPointAsStartLocation;

	/** Does the trace affect the aiming pitch */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Trace")
		bool bTraceAffectsAimPitch;

	/** Outputs a point that the player controller is looking at (within the MaxRange) (also this uses TraceChannel) */
	void AimWithPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params, const FVector& TraceStart, FVector& OutTraceEnd) const;
	/** Outputs a direction to use rather than a trace endpoint */
	void DirWithPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params, const FVector& TraceStart, FVector& OutTraceDir) const;

	static bool ClipCameraRayToAbilityRange(const FVector& CameraLocation, const FVector& CameraDirection, const FVector& AbilityCenter, const float AbilityRange, FVector& OutClippedPosition);


protected:
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;

	/** This is when the Wait Target Data Task starts using us */
	virtual void StartTargeting(UGameplayAbility* Ability) override;
	/** Where we perform our logic for collecting Target Data */
	virtual void ConfirmTargetingAndContinue() override;

	/**
	 * Calculates AimDir which is used in DirWithPlayerController().
	 * This can be overriden to add bullet spread for guns and stuff.
	 * 
	 * You can also determine AimStart if needed
	 */
	virtual void CalculateAimDirection(FVector& OutAimStart, FVector& OutAimDir) const;

	

	TArray<TWeakObjectPtr<AASSGameplayAbilityWorldReticle>> ReticleActors;

	AASSGameplayAbilityWorldReticle* SpawnReticleActor(const FVector& Location, const FRotator& Rotation);
	virtual void DestroyReticleActors();


	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

};

/*
* Todo:
*	1) Pooling system for reticles (assuming were using a reusable target actor)
*		Resetting the reticles array in StartTargeting shouldn't just destroy all reticles for resetting. Reticles should be recycled (deactivated and activated)
*	2) 
*/

