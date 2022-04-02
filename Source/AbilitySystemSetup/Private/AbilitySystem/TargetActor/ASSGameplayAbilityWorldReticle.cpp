// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/TargetActor/ASSGameplayAbilityWorldReticle.h"



AASSGameplayAbilityWorldReticle::AASSGameplayAbilityWorldReticle(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}


void AASSGameplayAbilityWorldReticle::ASSInitializeReticle(AActor* InTargetingActor, APlayerController* PlayerController, FASSWorldReticleParameters InASSParameters)
{
	check(InTargetingActor);

	ASSParameters = InASSParameters;

	InitializeReticle(InTargetingActor, PlayerController, InASSParameters); // call at the end instead of beginning because of their call to OnParametersInitialized()
}
