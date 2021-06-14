// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/TargetActor/ASSGameplayAbilityTargetActor.h"

#include "GATA_Trace.generated.h"



/**
 * Our base Trace class.
 * 
 * By default is just a standard trace with nothing special.
 * 
 *		- Has the ability to fire multiple traces at once using GetNumberOfTraces() and PerformTraces().
 *		- Has the ability to penetrate through blocking hits by using GetPenetrations().
 *		- Has the ability to ricochet off of certain hits using GetRicochets().
 *			Fill out RicochetableSurfaces array to determine which surface types to ricochet off of - OR override ShouldRicochetOffOf() to
 *			implement custom ricochet determination.
 *		- For super advanced tracing features override the trace events. We implemented a very advanced Bullet Speed system in GATA_BulletTrace using these.
 *			These events determine how the LineTraceMulti() function behaves. You can return false in these to stop LineTraceMulti() from doing any more traces.
 *			The reference parameters for HitResults are not const so you can directly modify them in these. (For example when we run out of Bullet Speed in GATA_BulletTrace, we
 *			remove the rest of the Hit Results after that point from the HitResults reference parameter).
 *		- TODO: sweeps aren't well-supported yet (for ricochets and penetrations)
 * 
 */
UCLASS(Abstract, notplaceable)
class ABILITYSYSTEMSETUP_API AGATA_Trace : public AASSGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	AGATA_Trace(const FObjectInitializer& ObjectInitializer);


	/** Number of traces to fire at once (a shotgun-like feature) */
	virtual int32 GetNumberOfTraces() const;
	/** Max times to penetrate through blocking hits (-1 for infinite) */
	virtual int32 GetPenetrations() const;
	/** Max times to ricochet (-1 for infinite) */
	virtual int32 GetRicochets() const;

	/**
	 * Any Physical Surfaces added to this array will ricochet any Trace that hits them.
	 * Note: you don't have to use these, you can override ShouldRicochetOffOf() for advanced ricochet logic.
	 */
	TArray<EPhysicalSurface> RicochetableSurfaces;

protected:
	/**
	 * Where we are needed to perform our traces
	 */
	virtual void ConfirmTargetingAndContinue() override;

	/** Traces as normal, but does ricochet and penetration traces aswell if needed */
	void LineTraceMulti(TArray<FHitResult>& OutHitResults, const UWorld* World, const FVector& Start, const FVector& End, const FCollisionQueryParams& Params, const bool inDebug);
	/** Sweeps as normal, but does ricochet traces aswell (unless ricochets is 0) */
	void SweepMultiWithRicochets(TArray<FHitResult>& OutHitResults, const UWorld* World, const FVector& Start, const FVector& End, const FQuat& Rotation, const FCollisionShape& CollisionShape, const FCollisionQueryParams& Params, const bool inDebug);


	/**
	 * Determines whether we should ricochet off of the given Hit or not.
	 * This can be overriden for custom ricochet determination.
	 */
	virtual bool ShouldRicochetOffOf(const FHitResult& Hit) const;
	/**
	 * Calculates the direction to ricochet from the given hit.
	 * This can be overriden for custom ricochet directions
	 */
	virtual void CalculateRicochetDirection(FVector& OutRicoDir, const FHitResult& FromHit) const;


	// LineTraceMulti() events - return false to discontinue traces
	virtual bool OnInitialTrace(TArray<FHitResult>& OutInitialHitResults, const UWorld* World, const FVector& Start, const FVector& End, const FCollisionQueryParams& TraceParams);
	virtual bool OnPenetrate(TArray<FHitResult>& HitResults, TArray<FHitResult>& OutPenetrateHitResults, const UWorld* World, const FVector& PenetrateStart, const FVector& PenetrateEnd, const FCollisionQueryParams& TraceParams);
	virtual bool OnRicochet(TArray<FHitResult>& HitResults, TArray<FHitResult>& OutRicoHitResults, const UWorld* World, const FVector& RicoStart, const FVector& RicoEnd, const FCollisionQueryParams& TraceParams);
	virtual void OnPostTraces(TArray<FHitResult>& HitResults, const UWorld* World, const FCollisionQueryParams& TraceParams);

	/**
	 * HERE FOR OVERRIDING PURPOSES ONLY!! DO NOT DIRECTLY CALL!!
	 * Prefer PerformTraces() instead in case GetNumberOfTraces() is greater than 1!
	 */
	virtual void PerformTrace(TArray<FHitResult>& OutHitResults, AActor* InSourceActor);

	/**
	 * Use this instead of PerformTrace() directly!
	 * Calls PerformTrace() for GetNumberOfTraces() amount of times safely.
	 * Outputs a 2D array of Hits for the sake of clarity for which Hit Results were from which trace.
	 */
	void PerformTraces(TArray<TArray<FHitResult>>& OutTraceResults, AActor* InSourceActor);
	/** Indicates which PerformTrace() call we are. */
	int32 CurrentTraceIndex;

	/** Event for when we are about to PerformTraces(). Useful for initializing data about multiple traces */
	virtual void OnPrePerformTraces(TArray<TArray<FHitResult>>& OutTraceResults, AActor* InSourceActor);


//#if ENABLE_DRAW_DEBUG
	/**
	 * Debugs given hit results.
	 */
	virtual void DebugHitResults(const TArray<FHitResult>& HitResults, const UWorld* World) const;
	virtual void DebugTrace(const TArray<FHitResult>& HitResults, const UWorld* World, const FVector& Start, const FVector& End) const;
	virtual void DebugTrace(const TArray<FHitResult>& HitResults, const UWorld* World, const FVector& Start, const FVector& End, const FVector& ExtraTraceEnd) const;
//#endif // ENABLE_DRAW_DEBUG

};
