// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityWorldReticle.h"

#include "ASSGameplayAbilityWorldReticle.generated.h"



/**
 * Custom World Reticle Parameters
 */
USTRUCT(BlueprintType)
struct FASSWorldReticleParameters : public FWorldReticleParameters
{
	GENERATED_BODY()


};

/**
 * Base World Reticle class
 */
UCLASS(Blueprintable, notplaceable)
class ABILITYSYSTEMSETUP_API AASSGameplayAbilityWorldReticle : public AGameplayAbilityWorldReticle
{
	GENERATED_BODY()
	
public:
	AASSGameplayAbilityWorldReticle(const FObjectInitializer& ObjectInitializer);


	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"), Category = "Reticle")
		FASSWorldReticleParameters ASSParameters;

	virtual void ASSInitializeReticle(AActor* InTargetingActor, APlayerController* InPlayerController, FASSWorldReticleParameters InASSParameters);

protected:

};
