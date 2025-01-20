// Fill out your copyright notice in the Description page of Project Settings.

#include "Examples/ASSActor_Example.h"

#include "AbilitySystemComponent.h"

AASSActor_Example::AASSActor_Example(const FObjectInitializer& inObjectInitializer)
    : Super(inObjectInitializer)
{
    bReplicates = true;

    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);

    // No possessing this actor allows for minimal replication mode.
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

    // Haven't messed with testing a good min net update frequency for adaptive net update frequency yet so we will keep it at max for now.
    SetMinNetUpdateFrequency(GetNetUpdateFrequency());
}

void AASSActor_Example::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    check(AbilitySystemComponent);
    AvatarActorExtensionComponent.InitializeAbilitySystemComponent(*AbilitySystemComponent, *this);
}

void AASSActor_Example::EndPlay(const EEndPlayReason::Type inEndPlayReason)
{
    AvatarActorExtensionComponent.UninitializeAbilitySystemComponent(*this);

    Super::EndPlay(inEndPlayReason);
}
