// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ASSAbilityTask.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"

#include "AT_WaitInputPress.generated.h"



//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInputPressDelegate, float, TimeWaited);


/**
 * 
 */
UCLASS()
class ABILITYSYSTEMSETUP_API UAT_WaitInputPress : public UASSAbilityTask
{
	GENERATED_BODY()

public:
	UAT_WaitInputPress(const FObjectInitializer& ObjectInitializer);


	UPROPERTY(BlueprintAssignable)
		FInputPressDelegate	OnPress;

	UFUNCTION()
		void OnPressCallback();

	virtual void Activate() override;

	/** Wait until the user presses the input button for this ability's activation. Returns time this node spent waiting for the press. Will return 0 if input was already down. */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
		static UAT_WaitInputPress* WaitInputPress(UGameplayAbility* OwningAbility, bool bTestAlreadyPressed = false, bool bCanBroadcastMultibleTimes = false);

	virtual void OnDestroy(bool bInOwnerFinished) override;

public:
	int32 callBackNumber;

protected:

	float StartTime;
	uint8 bTestInitialState;
	uint8 bCanBroadcastMultibleTimes;
	FDelegateHandle DelegateHandle;
};
