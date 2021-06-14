// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ASSAbilityTask.h"

#include "AT_Ticker.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FTickDelegate, float, DeltaTime, float, CurrentTime, float, TimeRemaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDurationEnded);


/**
 * 
 */
UCLASS()
class ABILITYSYSTEMSETUP_API UAT_Ticker : public UASSAbilityTask
{
	GENERATED_BODY()

public:
	UAT_Ticker(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable)
		FTickDelegate OnTick;
	UPROPERTY(BlueprintAssignable)
		FDurationEnded OnDurationFinish;

	virtual void TickTask(float DeltaTime) override;


	/** Start a task that repeats an action or set of actions. */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (DisplayName = "Ticker", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
		static UAT_Ticker* Ticker(UGameplayAbility* OwningAbility, bool skipFirstTick = false, float Duration = -1.f, float Interval = 0.f);

	void Activate() override;

	void RemoveAllDelegates();
	FString GetDebugString() const override;

	float duration;
	bool skipFirstTick;
	float tickInterval;

protected:
	float currentTime;
	float continueTimestamp;
	float timeRemaining;

	void OnDurationEnded();

};
