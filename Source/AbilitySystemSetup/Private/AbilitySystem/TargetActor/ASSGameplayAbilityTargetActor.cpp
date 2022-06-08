// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/TargetActor/ASSGameplayAbilityTargetActor.h"

#include "BlueprintFunctionLibraries/HLBlueprintFunctionLibrary_MathHelpers.h"



AASSGameplayAbilityTargetActor::AASSGameplayAbilityTargetActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false; // we don't need ticking by default
	// Start with Tick disabled. We'll enable it in StartTargeting() and disable it again in DisableTargetActor().
	// For instant confirmations, tick will never happen because we StartTargeting(), ConfirmTargeting(), and immediately DisableTargetActor().
	SetActorTickEnabled(false);

	ShouldProduceTargetDataOnServer = false;

	ReticleClass = AASSGameplayAbilityWorldReticle::StaticClass();

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
	DestroyWorldReticles(); // we should have a Reticle pooling system for this in the future
}

FVector AASSGameplayAbilityTargetActor::GetAimDirectionOfStartLocation() const
{
	FVector AimStart;
	FVector AimDir;
	CalculateAimDirection(AimStart, AimDir);

	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(SourceActor);
	return UHLBlueprintFunctionLibrary_MathHelpers::GetLocationAimDirection(GetWorld(), CollisionQueryParams, AimStart, AimDir, MaxRange, StartLocation.GetTargetingTransform().GetLocation());
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
		SpawnedWorldReticles.Add(SpawnedReticleActor);

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

void AASSGameplayAbilityTargetActor::DestroyWorldReticles()
{
	for (int32 i = SpawnedWorldReticles.Num() - 1; i >= 0; --i)
	{
		if (IsValid(SpawnedWorldReticles[i]))
		{
			SpawnedWorldReticles[i]->Destroy(); // we should have a reticle pooling system instead of creating and destroying these all of the time
		}
	}

	SpawnedWorldReticles.Empty();
}


void AASSGameplayAbilityTargetActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyWorldReticles();


	Super::EndPlay(EndPlayReason);
}
