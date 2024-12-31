// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/TargetActor/ASSGameplayAbilityTargetActor.h"



AASSGameplayAbilityTargetActor::AASSGameplayAbilityTargetActor(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // We don't need ticking by default
    PrimaryActorTick.bCanEverTick = false;
    // Start with Tick disabled. We'll enable it in StartTargeting() and disable it again in DisableTargetActor().
    // For instant confirmations, tick will never happen because we StartTargeting(), ConfirmTargeting(), and immediately DisableTargetActor().
    SetActorTickEnabled(false);

    ShouldProduceTargetDataOnServer = false;
    ReticleClass = AASSGameplayAbilityWorldReticle::StaticClass();
}

void AASSGameplayAbilityTargetActor::StartTargeting(UGameplayAbility* Ability) // when the Ability Task starts using us
{
    Super::StartTargeting(Ability);

    // Ensure we are re-enabled in case we were re-used
    SetActorTickEnabled(true);
}
void AASSGameplayAbilityTargetActor::DisableTargetActor() // when we are being disabled
{
    SetActorTickEnabled(false); // disable tick while we aren't being used
    DestroyWorldReticles(); // we should have a Reticle pooling system for this in the future
}


AASSGameplayAbilityWorldReticle* AASSGameplayAbilityTargetActor::SpawnWorldReticle(const FVector& InLocation, const FRotator& InRotation)
{
    if (ReticleClass)
    {
        FActorSpawnParameters SpawnParameters;
        SpawnParameters.ObjectFlags |= RF_Transient;
        SpawnParameters.Owner = this;

        FTransform SpawnTransform = FTransform(InRotation, InLocation, FVector::OneVector);

        AASSGameplayAbilityWorldReticle* SpawnedWorldReticle = GetWorld()->SpawnActor<AASSGameplayAbilityWorldReticle>(ReticleClass, SpawnTransform, SpawnParameters);
        if (IsValid(SpawnedWorldReticle))
        {
            SpawnedWorldReticle->ASSInitializeReticle(this, PrimaryPC, ASSReticleParams);
            SpawnedWorldReticle->SetActorHiddenInGame(true);
            SpawnedWorldReticles.Add(SpawnedWorldReticle);

            // This is to catch cases of playing on a listen server where we are using a replicated reticle actor.
            // (In a client controlled player, this would only run on the client and therefor never replicate. If it runs
            // on a listen server, the reticle actor may replicate. We want consistancy between client/listen server players.
            // Just saying 'make the reticle actor non replicated' isnt a good answer, since we want to mix and match reticle
            // actors and there may be other targeting types that want to replicate the same reticle actor class).
            if (!ShouldProduceTargetDataOnServer)
            {
                SpawnedWorldReticle->SetReplicates(false);
            }

            return SpawnedWorldReticle;
        }
    }

    return nullptr;
}

void AASSGameplayAbilityTargetActor::DestroyWorldReticles()
{
    for (int32 i = SpawnedWorldReticles.Num() - 1; i >= 0; --i)
    {
        if (IsValid(SpawnedWorldReticles[i]))
        {
            SpawnedWorldReticles[i]->Destroy(); // we should have a Reticle pooling system instead of creating and destroying these all of the time
        }
    }

    SpawnedWorldReticles.Empty();
}


void AASSGameplayAbilityTargetActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DestroyWorldReticles();


    Super::EndPlay(EndPlayReason);
}
