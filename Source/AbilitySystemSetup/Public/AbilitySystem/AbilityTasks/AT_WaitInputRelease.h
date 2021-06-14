// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem\ASSAbilityTask.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"

#include "AT_WaitInputRelease.generated.h"



//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInputReleaseDelegate, float, TimeHeld);


/**
 * 
 */
UCLASS()
class ABILITYSYSTEMSETUP_API UAT_WaitInputRelease : public UASSAbilityTask
{
	GENERATED_BODY()

public:
	UAT_WaitInputRelease(const FObjectInitializer& ObjectInitializer);


	UPROPERTY(BlueprintAssignable)
		FInputReleaseDelegate OnRelease;

	UFUNCTION()
		void OnReleaseCallback();

	virtual void Activate() override;

	/** Wait until the user releases the input button for this ability's activation. Returns time from hitting this node, till release. Will return 0 if input was already released. */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
		static UAT_WaitInputRelease* WaitInputRelease(UGameplayAbility* OwningAbility, bool bTestAlreadyReleased = false, bool bCanBroadcastMultibleTimes = false);

	virtual void OnDestroy(bool bInOwnerFinished) override;

public:
	int32 callBackNumber;

protected:

	float StartTime;
	uint8 bTestInitialState;
	uint8 bCanBroadcastMultibleTimes;
	FDelegateHandle DelegateHandle;
};
