// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AbilityTasks/AT_Ticker.h"

#include "Kismet/KismetSystemLibrary.h"



UAT_Ticker::UAT_Ticker(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = true;
	
}


UAT_Ticker* UAT_Ticker::Ticker(UGameplayAbility* OwningAbility, bool SkipFirstTick, float Duration, float Interval)
{
	UAT_Ticker* MyObj = NewAbilityTask<UAT_Ticker>(OwningAbility);
	MyObj->duration = Duration;
	MyObj->tickInterval = Interval;
	MyObj->skipFirstTick = SkipFirstTick;

	return MyObj;
}

void UAT_Ticker::Activate()
{

	currentTime = 0;
	timeRemaining = duration;
	continueTimestamp = 0;
}

void UAT_Ticker::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);


	if (duration != -1)	// If user set a duration
	{
		if (currentTime >= duration)
		{
			OnDurationEnded();
			RemoveAllDelegates();
			return;
		}
	}

	if ((continueTimestamp == 0) && skipFirstTick)
	{
		skipFirstTick = false;

		////
		currentTime = currentTime + DeltaTime;
		if (duration != -1)	// If user set a duration
		{
			timeRemaining = timeRemaining - DeltaTime;
		}
		continueTimestamp = continueTimestamp + tickInterval;
		////
		return;
	}
	if (currentTime < continueTimestamp)
	{
		currentTime = currentTime + DeltaTime;
		if (duration != -1)	// If user set a duration
		{
			timeRemaining = timeRemaining - DeltaTime;
		}
		return;
	}
	
	if (!IsPendingKill())
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnTick.Broadcast(DeltaTime, currentTime, timeRemaining);
		}
	}




	currentTime = currentTime + DeltaTime;
	if (duration != -1)	// If user set a duration
	{
		timeRemaining = timeRemaining - DeltaTime;
	}
	continueTimestamp = continueTimestamp + tickInterval;
}

void UAT_Ticker::OnDurationEnded()
{
	if (!IsPendingKill())
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnDurationFinish.Broadcast();
		}
	}
	EndTask();
}

void UAT_Ticker::RemoveAllDelegates()
{
	OnTick.Clear();
	OnDurationFinish.Clear();
}

FString UAT_Ticker::GetDebugString() const
{
	return FString::Printf(TEXT("Ticker"));
}


