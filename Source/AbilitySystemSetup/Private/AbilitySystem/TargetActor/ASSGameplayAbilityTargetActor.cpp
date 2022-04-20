// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/TargetActor/ASSGameplayAbilityTargetActor.h"

#include "BlueprintFunctionLibraries/BFL_HitResultHelpers.h"
#include "BlueprintFunctionLibraries/BFL_CollisionQueryHelpers.h"



AASSGameplayAbilityTargetActor::AASSGameplayAbilityTargetActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false; // we don't need ticking by default
	// Start with Tick disabled. We'll enable it in StartTargeting() and disable it again in DisableTargetActor().
	// For instant confirmations, tick will never happen because we StartTargeting(), ConfirmTargeting(), and immediately DisableTargetActor().
	SetActorTickEnabled(false);

	ShouldProduceTargetDataOnServer = false;

	ReticleClass = AASSGameplayAbilityWorldReticle::StaticClass();

	bAllowMultipleHitsPerActor = false;

	MaxRange = 100000.f;
	TraceChannel = ECollisionChannel::ECC_Visibility;

	bUseAimPointAsStartLocation = true;
}

// This is when the Ability Task starts using us
void AASSGameplayAbilityTargetActor::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);

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
// Where we perform our logic for collecting Target Data
void AASSGameplayAbilityTargetActor::ConfirmTargetingAndContinue()
{
	check(ShouldProduceTargetData());
	if (!IsConfirmTargetingAllowed())
	{
		return;
	}
}
void AASSGameplayAbilityTargetActor::DisableTargetActor()
{
	SetActorTickEnabled(false); // disable tick while we aren't being used
	DestroyReticleActors(); // we should have a Reticle pooling system for this in the future
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
bool AASSGameplayAbilityTargetActor::FilterHitResult(TArray<FHitResult>& OutHitResults, const int32 IndexToTryToFilter, const FGameplayTargetDataFilterHandle& FilterHandle, const bool inAllowMultipleHitsPerActor) const
{
	if (HitResultFailsFilter(OutHitResults, IndexToTryToFilter, FilterHandle, inAllowMultipleHitsPerActor))
	{
		OutHitResults.RemoveAt(IndexToTryToFilter);
		return true;
	}

	// This index was not filtered
	return false;
}
bool AASSGameplayAbilityTargetActor::HitResultFailsFilter(const TArray<FHitResult>& InHitResults, const int32 IndexToTryToFilter, const FGameplayTargetDataFilterHandle& FilterHandle, const bool inAllowMultipleHitsPerActor) const
{
	const FHitResult& HitToTryFilter = InHitResults[IndexToTryToFilter];


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
		for (int32 i = 0; i < IndexToTryToFilter; ++i)
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

FVector AASSGameplayAbilityTargetActor::GetAimDirectionOfStartLocation() const
{
	FVector AimStart;
	FVector AimDir;
	CalculateAimDirection(AimStart, AimDir);

	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(SourceActor);
	return UBFL_CollisionQueryHelpers::GetLocationAimDirection(GetWorld(), CollisionQueryParams, AimStart, AimDir, MaxRange, StartLocation.GetTargetingTransform().GetLocation());
}

void AASSGameplayAbilityTargetActor::CalculateAimDirection(FVector& OutAimStart, FVector& OutAimDir) const
{
	if (!IsValid(OwningAbility)) // server and launching client only
	{
		return;
	}

	const APlayerController* PC = OwningAbility->GetCurrentActorInfo()->PlayerController.Get();
	if (IsValid(PC))
	{
		FVector ViewStart;
		FRotator ViewRot;
		PC->GetPlayerViewPoint(ViewStart, ViewRot);
		OutAimStart = ViewStart;
		OutAimDir = ViewRot.Vector();
	}
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
	for (int32 i = ReticleActors.Num() - 1; i >= 0; --i)
	{
		if (ReticleActors[i].IsValid())
		{
			ReticleActors[i]->Destroy(); // we should have a reticle pooling system instead of creating and destroying these all of the time
		}
	}

	ReticleActors.Empty();
}


void AASSGameplayAbilityTargetActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyReticleActors();


	Super::EndPlay(EndPlayReason);
}
