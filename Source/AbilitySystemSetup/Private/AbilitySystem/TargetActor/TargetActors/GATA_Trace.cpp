// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/TargetActor/TargetActors/GATA_Trace.h"

#include "GameFramework/PlayerController.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystem/TargetActor/ASSGameplayAbilityTargetDataFilter.h"
#include "Actor/AbilitySystemActor.h"
#include "AbilitySystemSetup/Private/Utilities/ASSLogCategories.h"
#include "Kismet/KismetMathLibrary.h"

#include "DrawDebugHelpers.h"

#include "Kismet/KismetSystemLibrary.h"



AGATA_Trace::AGATA_Trace(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if 0
	RicochetableSurfaces.Add(SURFACETYPE_METAL);
#endif

	bDebug = true;
}


void AGATA_Trace::ConfirmTargetingAndContinue()
{
	Super::ConfirmTargetingAndContinue();

	if (SourceActor)
	{
		// The Target Data that we will broadcast - kind of like our return data
		FGameplayAbilityTargetDataHandle TargetDataHandle;
		

		// Perform this GATA's traces
		TArray<TArray<FHitResult>> TraceResults;
		PerformTraces(TraceResults, SourceActor);

		// Loop through each of our traces
		for (TArray<FHitResult>& HitResults : TraceResults)
		{
			// Filter this trace's Hit Results before we make Target Data from them
			FilterHitResults(HitResults, Filter, bAllowMultipleHitsPerActor);


			// The actors that this trace hit
			TArray<TWeakObjectPtr<AActor>> ThisTraceHitActors;
			ThisTraceHitActors.Reserve(HitResults.Num());

			// Loop through this trace's hits (that have been filtered)
			for (const FHitResult& HitResult : HitResults)
			{
				if (AActor* HitActor = HitResult.GetActor())
				{
					ThisTraceHitActors.Emplace(TWeakObjectPtr<AActor>(HitActor));
				}

			}

			// Add these actors the our Target Data Handle
			FGameplayAbilityTargetDataHandle ThisTraceTargetData = StartLocation.MakeTargetDataHandleFromActors(ThisTraceHitActors);
			TargetDataHandle.Append(ThisTraceTargetData);
		}


		// Broadcast this Target Data
		TargetDataReadyDelegate.Broadcast(TargetDataHandle);
	}
}

void AGATA_Trace::PerformTrace(TArray<FHitResult>& OutHitResults, AActor* InSourceActor)
{
	OutHitResults.Empty();


	FCollisionQueryParams Params = FCollisionQueryParams(SCENE_QUERY_STAT(AGATA_Trace));
	Params.AddIgnoredActor(InSourceActor);

	FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation();
	FVector TraceEnd;
	AimWithPlayerController(InSourceActor, Params, TraceStart, TraceEnd);		//Effective on server and launching client only

	// ------------------------------------------------------


	// Perform line trace
	LineTraceMulti(OutHitResults, InSourceActor->GetWorld(), TraceStart, TraceEnd, Params, bDebug);
}

void AGATA_Trace::PerformTraces(TArray<TArray<FHitResult>>& OutTraceResults, AActor* InSourceActor)
{
	OutTraceResults.Empty();

	// Cached NumberOfTraces
	const float NumberOfTraces = GetNumberOfTraces();

	// Reserve this predetermined space
	OutTraceResults.Reserve(NumberOfTraces);


	// Call event
	OnPrePerformTraces(OutTraceResults, InSourceActor);

	// Perform traces
	CurrentTraceIndex = 0;
	for (CurrentTraceIndex; CurrentTraceIndex < NumberOfTraces; ++CurrentTraceIndex)
	{
		TArray<FHitResult> ThisTraceHitResults;
		PerformTrace(ThisTraceHitResults, InSourceActor);

		OutTraceResults.Add(ThisTraceHitResults);
	}
	CurrentTraceIndex = 0;


}
void AGATA_Trace::OnPrePerformTraces(TArray<TArray<FHitResult>>& OutTraceResults, AActor* InSourceActor)
{

}

int32 AGATA_Trace::GetNumberOfTraces() const
{
	return 1;
}
int32 AGATA_Trace::GetPenetrations() const
{
	return 0;
}
int32 AGATA_Trace::GetRicochets() const
{
	return 0;
}

bool AGATA_Trace::ShouldRicochetOffOf(const FHitResult& Hit) const
{
	if (const UPhysicalMaterial* HitPhysMaterial = Hit.PhysMaterial.Get())
	{
		const EPhysicalSurface HitSurfaceType = HitPhysMaterial->SurfaceType;
		if (RicochetableSurfaces.Contains(HitSurfaceType))
		{
			return true;
		}
	}

	return false;
}
void AGATA_Trace::CalculateRicochetDirection(FVector& OutRicoDir, const FHitResult& FromHit) const
{
	const FVector FromDir = UKismetMathLibrary::GetDirectionUnitVector(FromHit.TraceStart, FromHit.Location);
	const FVector MirroredDir = FromDir.MirrorByVector(FromHit.ImpactNormal);

	OutRicoDir = MirroredDir;
}


void AGATA_Trace::LineTraceMulti(TArray<FHitResult>& OutHitResults, const UWorld* World, const FVector& Start, const FVector& End, const FCollisionQueryParams& Params, const bool inDebug)
{
	OutHitResults.Empty();
	check(World);


	float currentMaxRange = GetMaxRange();

	// Ensure we return Physical Material for the ricochet determination
	FCollisionQueryParams TraceParams = Params;
	TraceParams.bReturnPhysicalMaterial = true;

	// Perform initial trace
	TArray<FHitResult> InitialHitResults;
	const bool bShouldExtraTraces = OnInitialTrace(InitialHitResults, World, Start, End, TraceParams);
	if (InitialHitResults.Num() > 0)
	{
		currentMaxRange -= InitialHitResults.Last().Distance;
		OutHitResults.Append(InitialHitResults);
	}
	else
	{
		// Our caller may depend on us returning at least one Hit Result so it can get TraceStart and TraceEnd.
		// Make an empty Hit Result containing this info (this will just end up getting filtered)
		FHitResult TraceInfo;
		TraceInfo.TraceStart = Start;
		TraceInfo.TraceEnd = End;

		OutHitResults.Emplace(TraceInfo);
	}

	// Perform extra traces
	if (bShouldExtraTraces)
	{
		// Extra traces loop
		int32 maxRicochets = GetRicochets();
		int32 timesRicocheted = 0;
		int32 maxPenetrations = GetPenetrations();
		int32 timesPenetrated = 0;
		while (true)
		{
			if (OutHitResults.Num() <= 0)
			{
				// Nothing to work with, break here
				break;
			}
			FHitResult LastHit = OutHitResults.Last();
			if (LastHit.bBlockingHit == false)
			{
				// We only ricochet and penetrate for blocking hits
				break;
			}


			// Ricochet
			bool bRicochetHit = false;
			if (ShouldRicochetOffOf(LastHit))
			{
				if (timesRicocheted < maxRicochets || maxRicochets == -1)
				{
					// Calculate ricochet direction
					FVector RicoDir;
					CalculateRicochetDirection(RicoDir, LastHit);

					// Use direction to get the trace end
					FVector RicoStart = LastHit.Location + ((KINDA_SMALL_NUMBER * 100) * RicoDir);			// bump it outwards a bit for reassurance (sometimes works without this if using Chaos)
					FVector RicoEnd = RicoStart + (currentMaxRange * RicoDir);




					TArray<FHitResult> RicoHitResults;
					const bool bBreakOutEarly = !OnRicochet(OutHitResults, RicoHitResults, World, RicoStart, RicoEnd, TraceParams);




					if (RicoHitResults.Num() > 0)
					{
						currentMaxRange -= RicoHitResults.Last().Distance;
						OutHitResults.Append(RicoHitResults); // add these results to the end of OutHitResults
						LastHit = OutHitResults.Last(); // update LastHit
						++timesRicocheted;
						bRicochetHit = true;
					}
					else if (!bBreakOutEarly) // this ricochet didn't hit anything
					{
						// Our caller may depend on us returning at least one Hit Result so it can get TraceStart and TraceEnd.
						// Make an empty Hit Result containing this info (this will just end up getting filtered)
						FHitResult TraceInfo;
						TraceInfo.TraceStart = RicoStart;
						TraceInfo.TraceEnd = RicoEnd;

						OutHitResults.Emplace(TraceInfo);
					}

					if (bBreakOutEarly)
					{
						break;
					}
				}



				if (bRicochetHit == false)
				{
					// We should've ricocheted but didn't so stop here. (we don't want to proceed to penetration logic on ricochetable surfaces).
					// Possible ways of getting to this point:
					//		1. This ricochet didn't hit anything
					//		2. We ran out of ricochets / speed
					break;
				}
			}

			if (bRicochetHit)
			{
				// Continue here because we always want to check whether we should ricochet first before we try to penetrate.
				// For example. If we ricocheted, and then hit another surface that can be ricocheted off of, we want to run ricochet logic on that. If we don't continue here, the penetration logic below would run on that ricochetable surface instead of ricochet logic
				continue;
			}



			// Penetrate
			bool bPenetrateHit = false;
			if (timesPenetrated < maxPenetrations || maxPenetrations == -1)
			{
				// The direction we traced from
				const FVector FromDir = UKismetMathLibrary::GetDirectionUnitVector(LastHit.TraceStart, LastHit.Location);

				// Use direction to get the trace end
				FVector PenetrateStart = LastHit.Location + ((KINDA_SMALL_NUMBER * 100) * FromDir);			// bump it into itself a bit for reassurance (sometimes works without this if using Chaos)
				FVector PenetrateEnd = PenetrateStart + (currentMaxRange * FromDir);




				TArray<FHitResult> PenetrateHitResults;
				const bool bBreakOutEarly = !OnPenetrate(OutHitResults, PenetrateHitResults, World, PenetrateStart, PenetrateEnd, TraceParams);




				if (PenetrateHitResults.Num() > 0)
				{
					currentMaxRange -= PenetrateHitResults.Last().Distance;
					OutHitResults.Append(PenetrateHitResults); // add these results to the end of OutHitResults
					LastHit = OutHitResults.Last(); // update LastHit
					++timesPenetrated;
					bPenetrateHit = true;
				}
				else if (!bBreakOutEarly) // this penetration didn't hit anything
				{
					// Our caller may depend on us returning at least one Hit Result so it can get TraceStart and TraceEnd.
					// Make an empty Hit Result containing this info (this will just end up getting filtered)
					FHitResult TraceInfo;
					TraceInfo.TraceStart = PenetrateStart;
					TraceInfo.TraceEnd = PenetrateEnd;

					OutHitResults.Emplace(TraceInfo);
				}

				if (bBreakOutEarly)
				{
					break;
				}

			}



			if (bRicochetHit == false && bPenetrateHit == false)
			{
				// We weren't able to ricochet nor penetrate, stop here
				break;
			}

		}

		// If we didn't hit anything (ei. we traced into thin air)
		if (OutHitResults.Num() <= 0)
		{
			// Our caller may depend on us returning at least one Hit Result so it can get TraceStart and TraceEnd.
			// Make an empty Hit Result containing this info (this will just end up getting filtered)
			FHitResult TraceInfo;
			TraceInfo.TraceStart = Start;
			TraceInfo.TraceEnd = End;

			OutHitResults.Emplace(TraceInfo);
		}
	}

	OnPostTraces(OutHitResults, World, TraceParams);





#if ENABLE_DRAW_DEBUG
	if (inDebug)
	{
		DebugHitResults(OutHitResults, World);
	}
#endif
}

bool AGATA_Trace::OnInitialTrace(TArray<FHitResult>& OutInitialHitResults, const UWorld* World, const FVector& Start, const FVector& End, const FCollisionQueryParams& TraceParams)
{
	OutInitialHitResults.Empty();


	World->LineTraceMultiByChannel(OutInitialHitResults, Start, End, TraceChannel, TraceParams);

	if (OutInitialHitResults.Num() <= 0)
	{
		return false;
	}
	if (OutInitialHitResults.Last().bBlockingHit == false)
	{
		return false;
	}


	return true;
}
bool AGATA_Trace::OnPenetrate(TArray<FHitResult>& HitResults, TArray<FHitResult>& OutPenetrateHitResults, const UWorld* World, const FVector& PenetrateStart, const FVector& PenetrateEnd, const FCollisionQueryParams& TraceParams)
{
	OutPenetrateHitResults.Empty();


	FCollisionQueryParams PenetrateParams = TraceParams;
	PenetrateParams.bTraceComplex = true; // Use complex collision if the collider has it. We need this because we are starting from inside the geometry and shooting out (this won't work for CTF_UseSimpleAsComplex and Physics Assest colliders so TODO: we will need a case that covers this)

	// Perform penetrate trace
	World->LineTraceMultiByChannel(OutPenetrateHitResults, PenetrateStart, PenetrateEnd, TraceChannel, PenetrateParams);



	// Our fallback if the trace messed up (if we hit ourselves instead of going through to the next hit). This happens if we are a CTF_UseSimpleAsComplex or a Physics Asset collider.
	if (OutPenetrateHitResults.Num() > 0 && OutPenetrateHitResults.Last().Distance == 0)
	{
		// Try again with this component ignored
		PenetrateParams.AddIgnoredComponent(OutPenetrateHitResults.Last().GetComponent());
		OutPenetrateHitResults.Empty();
		World->LineTraceMultiByChannel(OutPenetrateHitResults, PenetrateStart, PenetrateEnd, TraceChannel, PenetrateParams);
	}


	return true;
}
bool AGATA_Trace::OnRicochet(TArray<FHitResult>& HitResults, TArray<FHitResult>& OutRicoHitResults, const UWorld* World, const FVector& RicoStart, const FVector& RicoEnd, const FCollisionQueryParams& TraceParams)
{
	OutRicoHitResults.Empty();



	FCollisionQueryParams RicoParams = TraceParams;
	RicoParams.bTraceComplex = true; // ricochet doesn't need Trace Complex but because we are mixed in with penetration logic (which uses Trace Complex), we have to consider it

	// Perform ricochet trace
	World->LineTraceMultiByChannel(OutRicoHitResults, RicoStart, RicoEnd, TraceChannel, RicoParams);


	return true;
}
void AGATA_Trace::OnPostTraces(TArray<FHitResult>& HitResults, const UWorld* World, const FCollisionQueryParams& TraceParams)
{
	
}

void AGATA_Trace::SweepMultiWithRicochets(TArray<FHitResult>& OutHitResults, const UWorld* World, const FVector& Start, const FVector& End, const FQuat& Rotation, const FCollisionShape& CollisionShape, const FCollisionQueryParams& Params, const bool inDebug)
{
	check(World);


	FCollisionQueryParams TraceParams = Params;
	TraceParams.bReturnPhysicalMaterial = true;

	TArray<FHitResult> HitResults;
	World->SweepMultiByChannel(OutHitResults, Start, End, Rotation, TraceChannel, CollisionShape, TraceParams);

	OutHitResults.Append(HitResults);

	// ricochets
	uint8 r = 0; // outside for bDebug to use maybe try to change this idk
	for (r; r < GetRicochets(); ++r)
	{
		if (OutHitResults.Num() <= 0)
		{
			break;
		}
		const FHitResult LastHit = OutHitResults.Last();
		if (LastHit.bBlockingHit == false)
		{
			break;
		}
		const EPhysicalSurface LastSurface = LastHit.PhysMaterial.Get()->SurfaceType;
		if (RicochetableSurfaces.Contains(LastSurface) == false)
		{
			// This surface isn't ricochetable
			break;
		}


		// Add the current hit actor on top of the ignored actors
		FCollisionQueryParams RicoParams = TraceParams;

		// Calculate ricochet direction
		FVector RicoDir;
		CalculateRicochetDirection(RicoDir, LastHit);

		// Use direction to get the trace end
		const FVector RicoStart = LastHit.Location + (KINDA_SMALL_NUMBER * RicoDir);
		const FVector RicoEnd = RicoStart + ((GetMaxRange() - LastHit.Distance) * RicoDir);

		// Perform ricochet sweep
		TArray<FHitResult> RicoHitResults; // this ricochet's hit results
		const bool bHitBlockingHit = World->SweepMultiByChannel(RicoHitResults, RicoStart, RicoEnd, Rotation, TraceChannel, CollisionShape, RicoParams);

		OutHitResults.Append(RicoHitResults);

		if (!bHitBlockingHit)
		{
			break;
		}
	}



#if ENABLE_DRAW_DEBUG
	if (inDebug)
	{
		DebugTrace(OutHitResults, World, Start, End/*, r*/);
	}
#endif
}

//#if ENABLE_DRAW_DEBUG
void AGATA_Trace::DebugHitResults(const TArray<FHitResult>& HitResults, const UWorld* World) const
{
#if ENABLE_DRAW_DEBUG
	TArray<FHitResult> DebugHitResults = HitResults;
	FilterHitResults(DebugHitResults, FGameplayTargetDataFilterHandle(), bAllowMultipleHitsPerActor); // removes multiple hits if needed (but doesn't filter actors because we have different colors for whether it filters or not)


	const float debugLifeTime = 5.f;
	const FColor TraceColor = FColor::Blue;
	const FColor PassesFilterColor = FColor::Red;

	if (DebugHitResults.Num() > 0)
	{
		for (int32 i = 0; i < DebugHitResults.Num(); ++i)
		{
			const FHitResult& Hit = DebugHitResults[i];

			FVector FromLocation = Hit.TraceStart;
			FVector ToLocation = Hit.Location;
			if (Hit.Location == FVector::ZeroVector) // if invalid Location (this is only to support empty Hit Results that hold TraceStart and TraceEnd locations and this is weird but i don't care about this function enough to fix it)
			{
				ToLocation = Hit.TraceEnd; // to get a line that makes sense, just draw to TraceEnd
			}

			DrawDebugLine(World, FromLocation, ToLocation, TraceColor, false, debugLifeTime);
			
			if (Hit.Location != FVector::ZeroVector) // if Location is valid
			{
				const bool bPassesFilter = Filter.FilterPassesForActor(Hit.GetActor());
				if (bPassesFilter)
				{
					DrawDebugPoint(World, Hit.ImpactPoint, 10, PassesFilterColor, false, debugLifeTime);
				}
				else
				{
					DrawDebugPoint(World, Hit.ImpactPoint, 10, TraceColor, false, debugLifeTime);
				}
			}
		}
	}
#endif // ENABLE_DRAW_DEBUG
}

void AGATA_Trace::DebugTrace(const TArray<FHitResult>& HitResults, const UWorld* World, const FVector& Start, const FVector& End) const
{
#if ENABLE_DRAW_DEBUG
	DebugHitResults(HitResults, World);


	TArray<FHitResult> DebugHitResults = HitResults;
	FilterHitResults(DebugHitResults, FGameplayTargetDataFilterHandle(), bAllowMultipleHitsPerActor); // removes multiple hits if needed (but doesn't filter actors because we have different colors for whether it filters or not)


	const float debugLifeTime = 5.f;
	const FColor TraceColor = FColor::Blue;
	const FColor PassesFilterColor = FColor::Red;

	if (DebugHitResults.Num() > 0)
	{
		if (DebugHitResults.Last().bBlockingHit == false)
		{
			// If our last hit was an overlap, draw out one last Trace End to show that it went through
			DrawDebugLine(World, DebugHitResults.Last().Location, DebugHitResults.Last().TraceEnd, TraceColor, false, debugLifeTime);		// after the we've drawn a line to all hit results, draw from last hit result to the trace end
		}
	}
	else // if we've traced in thin air
	{
		// Draw the line that didn't hit anything
		DrawDebugLine(World, Start, End, TraceColor, false, debugLifeTime);
	}

#endif // ENABLE_DRAW_DEBUG
}

void AGATA_Trace::DebugTrace(const TArray<FHitResult>& HitResults, const UWorld* World, const FVector& Start, const FVector& End, const FVector& ExtraTraceEnd) const
{
#if ENABLE_DRAW_DEBUG
	DebugTrace(HitResults, World, Start, End);


	TArray<FHitResult> DebugHitResults = HitResults;
	FilterHitResults(DebugHitResults, FGameplayTargetDataFilterHandle(), bAllowMultipleHitsPerActor); // removes multiple hits if needed (but doesn't filter actors because we have different colors for whether it filters or not)


	const float debugLifeTime = 5.f;
	const FColor TraceColor = FColor::Blue;
	const FColor PassesFilterColor = FColor::Red;

	if (DebugHitResults.Num() > 0)
	{
		if (DebugHitResults.Last().bBlockingHit)
		{
			if ((ExtraTraceEnd - DebugHitResults.Last().Location).IsNearlyZero() == false) // only draw this extra trace if it is different then the last trace
			{
				DrawDebugLine(World, DebugHitResults.Last().Location, ExtraTraceEnd, TraceColor, false, debugLifeTime);
			}
		}
	}
#endif // ENABLE_DRAW_DEBUG
}
//#endif // ENABLE_DRAW_DEBUG
