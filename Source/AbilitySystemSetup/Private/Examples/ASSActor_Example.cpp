// Fill out your copyright notice in the Description page of Project Settings.


#include "Examples\ASSActor_Example.h"

#include "AbilitySystem/ASSAbilitySystemComponent.h"
#include "Subobjects/ASSActorComponent_AbilitySystemSetup.h"



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


	// Create setup component for the ASC
	AbilitySystemSetupComponent = CreateDefaultSubobject<UASSActorComponent_AbilitySystemSetup>(TEXT("AbilitySystemSetupComponent"));
}


void AASSActor_Example::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	AbilitySystemSetupComponent->InitializeAbilitySystemComponent(GetAbilitySystemComponent());
}

void AASSActor_Example::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AbilitySystemSetupComponent->UninitializeAbilitySystemComponent();

	Super::EndPlay(EndPlayReason);
}
