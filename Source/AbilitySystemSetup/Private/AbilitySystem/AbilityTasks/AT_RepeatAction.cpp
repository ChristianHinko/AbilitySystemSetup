// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AbilityTasks/AT_RepeatAction.h"

UAT_RepeatAction::UAT_RepeatAction(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


UAT_RepeatAction* UAT_RepeatAction::RepeatAction(UGameplayAbility* OwningAbility, float DelayBetweenRepeats, bool ShouldLoop, float FirstDelay)
{
	UAT_RepeatAction* MyObj = NewAbilityTask<UAT_RepeatAction>(OwningAbility);
	MyObj->DelayBetweenRepeats = DelayBetweenRepeats;
	MyObj->shouldLoop = ShouldLoop;
	MyObj->firstDelay = FirstDelay;

	return MyObj;
}

void UAT_RepeatAction::Activate()
{
	if (!Ability || !AbilitySystemComponent)
	{
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(TimerHandle_PerformAction, this, &UAT_RepeatAction::PerformAction, DelayBetweenRepeats, shouldLoop, firstDelay);
}

void UAT_RepeatAction::PerformAction() const
{
	if (!Ability || !AbilitySystemComponent)
	{
		return;
	}

	//	Would be nice to have built in prediction window here but last time I tried it ended up firing lots of server broadcasts at once if you waited long enough

	if (!IsPendingKill())
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnPerformAction.Broadcast();
		}
	}
}



void UAT_RepeatAction::OnDestroy(bool AbilityIsEnding)
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_PerformAction);

	Super::OnDestroy(AbilityIsEnding);
}

FString UAT_RepeatAction::GetDebugString() const
{
	return FString::Printf(TEXT("RepeatAction"));
}