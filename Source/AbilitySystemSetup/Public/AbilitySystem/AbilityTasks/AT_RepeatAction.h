// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ASSAbilityTask.h"

#include "AT_RepeatAction.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRepeatActionDelegate);


/**
 * 
 */
UCLASS()
class ABILITYSYSTEMSETUP_API UAT_RepeatAction : public UASSAbilityTask
{
	GENERATED_BODY()

public:
	UAT_RepeatAction(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable)
		FRepeatActionDelegate OnPerformAction;

	FString GetDebugString() const override;

	void PerformAction() const;

	/** Start a task that repeats an action or set of actions. */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (DisplayName = "Repeat Action (Timer)", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
		static UAT_RepeatAction* RepeatAction(UGameplayAbility* OwningAbility, float DelayBetweenRepeats, bool ShouldLoop, float FirstDelay = 0);

	void Activate() override;

protected:
	float DelayBetweenRepeats;
	bool shouldLoop;
	float firstDelay;

	FTimerHandle TimerHandle_PerformAction;

	void OnDestroy(bool AbilityIsEnding) override;
};
