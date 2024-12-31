// Fill out your copyright notice in the Description page of Project Settings.

#include "ASSGameplayAbilityWorldReticle.h"

AASSGameplayAbilityWorldReticle::AASSGameplayAbilityWorldReticle(const FObjectInitializer& inObjectInitializer)
    : Super(inObjectInitializer)
{
}

void AASSGameplayAbilityWorldReticle::ASSInitializeReticle(AGameplayAbilityTargetActor* inTargetingActor, APlayerController* inPlayerController, FASSWorldReticleParameters inASSParameters)
{
    check(inTargetingActor);

    ASSParameters = MoveTemp(inASSParameters);

    InitializeReticle(inTargetingActor, inPlayerController, ASSParameters); // Call at the end instead of beginning because of their call to `OnParametersInitialized()`.
}
