// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/TargetActor/ASSGameplayAbilityWorldReticle.h"



AASSGameplayAbilityWorldReticle::AASSGameplayAbilityWorldReticle(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}


void AASSGameplayAbilityWorldReticle::ASSInitializeReticle(AGameplayAbilityTargetActor* InTargetingActor, APlayerController* InPlayerController, FASSWorldReticleParameters InASSParameters)
{
	check(InTargetingActor);

	ASSParameters = InASSParameters;

	InitializeReticle(InTargetingActor, InPlayerController, InASSParameters); // call at the end instead of beginning because of their call to OnParametersInitialized()
}
