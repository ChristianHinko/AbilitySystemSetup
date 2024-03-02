// Fill out your copyright notice in the Description page of Project Settings.


#include "Examples\ASSActor_Example.h"

#include "AbilitySystem/ASSAbilitySystemComponent.h"
#include "Subobjects/ASSActorComponent_AvatarActorExtension.h"



AASSActor_Example::AASSActor_Example(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bReplicates = true;

    AbilitySystemComponent = CreateDefaultSubobject<UASSAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);

    // No possessing this actor allows for Minimal replication mode
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
    // Havn't messed with testing a good MinNetUpdateFrequency for adaptive net update frequency yet so we will keep it at max for now
    MinNetUpdateFrequency = NetUpdateFrequency;


    // Create the avatar actor extension component to assist in setting us up with the ASC
    AvatarActorExtensionComponent = CreateDefaultSubobject<UASSActorComponent_AvatarActorExtension>(TEXT("AvatarActorExtensionComponent"));
}


void AASSActor_Example::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    check(AbilitySystemComponent);
    AvatarActorExtensionComponent->InitializeAbilitySystemComponent(*AbilitySystemComponent);
}

void AASSActor_Example::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    AvatarActorExtensionComponent->UninitializeAbilitySystemComponent();

    Super::EndPlay(EndPlayReason);
}
