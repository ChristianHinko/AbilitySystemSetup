// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/TargetActor/ASSGameplayAbilityTargetActor.h"

#include "Abilities/GameplayAbility.h"
#include "AbilitySystem/ASSAbilitySystemBlueprintLibrary.h"
#include "BlueprintFunctionLibraries/BFL_HitResultHelpers.h"



AASSGameplayAbilityTargetActor::AASSGameplayAbilityTargetActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	ShouldProduceTargetDataOnServer = false;


	ReticleClass = AASSGameplayAbilityWorldReticle::StaticClass();

	//MultiFilter = Filter;
	MultiFilter.bOnlyAcceptAbilitySystemInterfaces = true;
	bAllowMultipleHitsPerActor = false;

	TraceChannel = ECollisionChannel::ECC_Visibility;

	bUseAimPointAsStartLocation = true;
	bTraceAffectsAimPitch = true;

}
void AASSGameplayAbilityTargetActor::PreInitializeComponents()
{
	Super::PreInitializeComponents();


	Filter = UASSAbilitySystemBlueprintLibrary::MakeMultiFilterHandle(MultiFilter, SourceActor);
}
void AASSGameplayAbilityTargetActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();


	// Start with Tick disabled. We'll enable it in StartTargeting() and disable it again in StopTargeting().
	// For instant confirmations, tick will never happen because we StartTargeting(), ConfirmTargeting(), and immediately StopTargeting().
	SetActorTickEnabled(false);
}


void AASSGameplayAbilityTargetActor::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);
	SourceActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();

	// Ensure we are re-enabled in case we were re-used
	SetActorTickEnabled(true);


	// TODO: This only updates on StartTargeting(). If you had a non-"EGameplayTargetingConfirmation::Instant" confirmation, this would result in the
	// start location of where you began targeting. Add more configuration for this (maybe one that updates in StartTargeting(), one that updates on Tick(), and one
	// that updates in ConfirmTargetingAndContinue())
	if (bUseAimPointAsStartLocation)
	{
		StartLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;

		FVector AimStart;
		FVector AimDir;
		CalculateAimDirection(AimStart, AimDir);

		StartLocation.LiteralTransform.SetLocation(AimStart);
	}

}
void AASSGameplayAbilityTargetActor::ConfirmTargetingAndContinue()
{
	check(ShouldProduceTargetData());
	if (!IsConfirmTargetingAllowed())
	{
		return;
	}
}
void AASSGameplayAbilityTargetActor::StopTargeting()
{
	SetActorTickEnabled(false); // disable tick while we aren't being used
	DestroyReticleActors(); // we should have a Reticle pooling system for this eventually
}

float AASSGameplayAbilityTargetActor::GetMaxRange() const
{
	return 100000.f;
}

void AASSGameplayAbilityTargetActor::FilterHitResults(TArray<FHitResult>& OutHitResults, const FGameplayTargetDataFilterHandle& FilterHandle, const bool inAllowMultipleHitsPerActor) const
{
	for (int32 i = 0; i < OutHitResults.Num(); ++i)
	{
		if (FilterHitResult(OutHitResults, i, FilterHandle, inAllowMultipleHitsPerActor))
		{
			// This index was filtered
		}
	}
}
bool AASSGameplayAbilityTargetActor::FilterHitResult(TArray<FHitResult>& OutHitResults, const int32 IndexToTryFilter, const FGameplayTargetDataFilterHandle& FilterHandle, const bool inAllowMultipleHitsPerActor) const
{
	if (HitResultFailsFilter(OutHitResults, IndexToTryFilter, FilterHandle, inAllowMultipleHitsPerActor))
	{
		OutHitResults.RemoveAt(IndexToTryFilter);
		return true;
	}

	// This index was not filtered
	return false;
}
bool AASSGameplayAbilityTargetActor::HitResultFailsFilter(const TArray<FHitResult>& InHitResults, const int32 IndexToTryFilter, const FGameplayTargetDataFilterHandle& FilterHandle, const bool inAllowMultipleHitsPerActor) const
{
	const FHitResult& HitToTryFilter = InHitResults[IndexToTryFilter];


	if (FilterHandle.Filter.IsValid()) // if valid filter
	{
		const bool bPassesFilter = FilterHandle.FilterPassesForActor(HitToTryFilter.GetActor());
		if (!bPassesFilter)
		{
			return true;
		}
	}

	if (!inAllowMultipleHitsPerActor) // if we should remove multiple hits
	{
		// Loop through each Hit Result and check if the hits infront of it (the Hit Results less than the indexToTryFilter) already have its Actor.
		// If so, remove the indexToTryFilter Hit Result because it has the Actor that was already hit and is considered a duplicate hit.


		// Check if any Hit Results before this hit contains a Hit Result with this Actor already
		for (int32 i = 0; i < IndexToTryFilter; ++i)
		{
			const FHitResult& Hit = InHitResults[i];

			if (HitToTryFilter.GetActor() == Hit.GetActor()) // if we already hit this actor
			{
				if (UBFL_HitResultHelpers::AreHitsFromSameTrace(HitToTryFilter, Hit)) // only remove if they were in the same trace (if they were from separate traces, they aren't considered a duplicate hit)
				{
					return true;
					break;
				}
			}
		}
	}

	// This index was not filtered
	return false;
}

void AASSGameplayAbilityTargetActor::AimWithPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params, const FVector& TraceStart, FVector& OutTraceEnd) const
{
	FVector TraceDir;
	DirWithPlayerController(InSourceActor, Params, TraceStart, TraceDir);

	OutTraceEnd = TraceStart + (TraceDir * GetMaxRange());
}
void AASSGameplayAbilityTargetActor::DirWithPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params, const FVector& TraceStart, FVector& OutTraceDir) const
{
	FVector AimStart;
	FVector AimDir;
	CalculateAimDirection(AimStart, AimDir);
	FVector AimEnd = AimStart + (AimDir * GetMaxRange());

	ClipCameraRayToAbilityRange(AimStart, AimDir, TraceStart, GetMaxRange(), AimEnd);

	// If the TraceStart is nearly equal to the AimStart, skip the useless camera trace and just return the aim direction
	if (TraceStart.Equals(AimStart))
	{
		// As an optimization, skip the extra trace and return here
		OutTraceDir = (AimEnd - TraceStart).GetSafeNormal();
		return;
	}

	// Line trace from the TraceStart to the the point that player is looking at so we can calculate the direction
	TArray<FHitResult> HitResults;
	InSourceActor->GetWorld()->LineTraceMultiByChannel(HitResults, AimStart, AimEnd, TraceChannel, Params);
	FHitResult HitResult = HitResults.Num() ? HitResults[0] : FHitResult();

	const bool bUseTraceResult = /*HitResult.bBlockingHit && */(FVector::DistSquared(TraceStart, HitResult.Location) <= (GetMaxRange() * GetMaxRange()));

	const FVector AdjustedEnd = (bUseTraceResult) ? HitResult.Location : AimEnd;

	FVector AdjustedAimDir = (AdjustedEnd - TraceStart).GetSafeNormal();
	if (AdjustedAimDir.IsZero())
	{
		AdjustedAimDir = AimDir;
	}

	if (!bTraceAffectsAimPitch && bUseTraceResult)
	{
		FVector OriginalAimDir = (AimEnd - TraceStart).GetSafeNormal();

		if (!OriginalAimDir.IsZero())
		{
			// Convert to angles and use original pitch
			const FRotator OriginalAimRot = OriginalAimDir.Rotation();

			FRotator AdjustedAimRot = AdjustedAimDir.Rotation();
			AdjustedAimRot.Pitch = OriginalAimRot.Pitch;

			AdjustedAimDir = AdjustedAimRot.Vector();
		}
	}

	OutTraceDir = AdjustedAimDir;
}

void AASSGameplayAbilityTargetActor::CalculateAimDirection(FVector& OutAimStart, FVector& OutAimDir) const
{
	if (!OwningAbility) // Server and launching client only
	{
		return;
	}


	const APlayerController* PC = OwningAbility->GetCurrentActorInfo()->PlayerController.Get();
	check(PC);

	FRotator ViewRot;
	PC->GetPlayerViewPoint(OutAimStart, ViewRot);

	OutAimDir = ViewRot.Vector();
}

bool AASSGameplayAbilityTargetActor::ClipCameraRayToAbilityRange(const FVector& CameraLocation, const FVector& CameraDirection, const FVector& AbilityCenter, const float AbilityRange, FVector& OutClippedPosition)
{
	const FVector CameraToCenter = AbilityCenter - CameraLocation;
	const float DotToCenter = FVector::DotProduct(CameraToCenter, CameraDirection);
	if (DotToCenter >= 0)		//If this fails, we're pointed away from the center, but we might be inside the sphere and able to find a good exit point.
	{
		const float DistanceSquared = CameraToCenter.SizeSquared() - (DotToCenter * DotToCenter);
		const float RadiusSquared = (AbilityRange * AbilityRange);
		if (DistanceSquared <= RadiusSquared)
		{
			const float DistanceFromCamera = FMath::Sqrt(RadiusSquared - DistanceSquared);
			const float DistanceAlongRay = DotToCenter + DistanceFromCamera;					//Subtracting instead of adding will get the other intersection point
			OutClippedPosition = CameraLocation + (DistanceAlongRay * CameraDirection);			//Cam aim point clipped to range sphere
			return true;
		}
	}

	return false;
}


AASSGameplayAbilityWorldReticle* AASSGameplayAbilityTargetActor::SpawnReticleActor(const FVector& Location, const FRotator& Rotation)
{
	if (!ReticleClass)
	{
		return nullptr;
	}

	AASSGameplayAbilityWorldReticle* SpawnedReticleActor = GetWorld()->SpawnActor<AASSGameplayAbilityWorldReticle>(ReticleClass, Location, Rotation);
	if (IsValid(SpawnedReticleActor))
	{
		SpawnedReticleActor->ASSInitializeReticle(this, MasterPC, ASSReticleParams);
		SpawnedReticleActor->SetActorHiddenInGame(true);
		ReticleActors.Add(SpawnedReticleActor);

		// This is to catch cases of playing on a listen server where we are using a replicated reticle actor.
		// (In a client controlled player, this would only run on the client and therefor never replicate. If it runs
		// on a listen server, the reticle actor may replicate. We want consistancy between client/listen server players.
		// Just saying 'make the reticle actor non replicated' isnt a good answer, since we want to mix and match reticle
		// actors and there may be other targeting types that want to replicate the same reticle actor class).
		if (!ShouldProduceTargetDataOnServer)
		{
			SpawnedReticleActor->SetReplicates(false);
		}

	}

	return SpawnedReticleActor;
}

void AASSGameplayAbilityTargetActor::DestroyReticleActors()
{
	for (int32 i = ReticleActors.Num() - 1; i >= 0; i--)
	{
		if (ReticleActors[i].IsValid())
		{
			ReticleActors[i].Get()->Destroy();
		}
	}

	ReticleActors.Empty();
}


void AASSGameplayAbilityTargetActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyReticleActors();


	Super::EndPlay(EndPlayReason);
}
