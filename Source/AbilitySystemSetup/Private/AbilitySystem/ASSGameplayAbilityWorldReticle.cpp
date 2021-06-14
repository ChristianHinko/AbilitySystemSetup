// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ASSGameplayAbilityWorldReticle.h"



AASSGameplayAbilityWorldReticle::AASSGameplayAbilityWorldReticle()
{

}


void AASSGameplayAbilityWorldReticle::ASSInitializeReticle(AActor* InTargetingActor, APlayerController* PlayerController, FASSWorldReticleParameters InASSParameters)
{
	check(InTargetingActor);

	ASSParameters = InASSParameters;

	InitializeReticle(InTargetingActor, PlayerController, InASSParameters); // call at the end instead of beginning because of their call to OnParametersInitialized()
}
