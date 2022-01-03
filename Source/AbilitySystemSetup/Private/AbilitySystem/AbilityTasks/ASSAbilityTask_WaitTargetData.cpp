// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AbilityTasks/ASSAbilityTask_WaitTargetData.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/TargetActor/ASSGameplayAbilityTargetActor.h"
#include "AbilitySystemSetup/Private/Utilities/ASSLogCategories.h"



UASSAbilityTask_WaitTargetData* UASSAbilityTask_WaitTargetData::ASSWaitTargetDataUsingActor(UGameplayAbility* OwningAbility, FName TaskInstanceName, TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType, AGameplayAbilityTargetActor* InTargetActor)
{
	UASSAbilityTask_WaitTargetData* MyObj = NewAbilityTask<UASSAbilityTask_WaitTargetData>(OwningAbility, TaskInstanceName);		//Register for task list here, providing a given FName as a key
	MyObj->TargetClass = nullptr;
	MyObj->TargetActor = InTargetActor;
	MyObj->ConfirmationType = ConfirmationType;
	return MyObj;
}

void UASSAbilityTask_WaitTargetData::Activate()
{
	// Need to handle case where target actor was passed into task
	if (Ability && (TargetClass == nullptr))
	{
		if (TargetActor)
		{
			AGameplayAbilityTargetActor* SpawnedActor = TargetActor;
			TargetClass = SpawnedActor->GetClass();

			RegisterTargetDataCallbacks();


			if (IsPendingKill())
			{
				return;
			}

			if (ShouldSpawnTargetActor())
			{
				InitializeTargetActor(SpawnedActor);
				FinalizeTargetActor(SpawnedActor);

				// Note that the call to FinalizeTargetActor, this task could finish and our owning ability may be ended.
			}
			else
			{
				if (TargetActor->bDestroyOnConfirmation)	// If this is true, the developer obiously is handling the GATA lifetime on his own. So we won't destroy it
				{
					TargetActor = nullptr;

					// We may need a better solution here.  We don't know the target actor isn't needed till after it's already been spawned.
					SpawnedActor->Destroy();
					SpawnedActor = nullptr;
				}
			}
		}
		else
		{
			EndTask();
		}
	}
}

void UASSAbilityTask_WaitTargetData::OnDestroy(bool AbilityEnded)
{
	if (TargetActor)
	{
		if (TargetActor->bDestroyOnConfirmation)
		{
			TargetActor->Destroy();
		}
		else
		{
			// Instead of destroying it, just deactivate it:


			if (AASSGameplayAbilityTargetActor* ASSTargetActor = Cast<AASSGameplayAbilityTargetActor>(TargetActor))
			{
				// Tell the Target Actor he is being deactivated
				ASSTargetActor->StopTargeting();
			}
			else
			{
				UE_LOG(LogGameplayAbilityTargetActorSetup, Warning, TEXT("%s() Your not using our custom base target actor. Tried to call stop targeting but we couldn't because of this"), *FString(__FUNCTION__));
			}

			// Clear added callbacks
			TargetActor->TargetDataReadyDelegate.RemoveAll(this);
			TargetActor->CanceledDelegate.RemoveAll(this);

			AbilitySystemComponent->GenericLocalConfirmCallbacks.RemoveDynamic(TargetActor, &AGameplayAbilityTargetActor::ConfirmTargeting);
			AbilitySystemComponent->GenericLocalCancelCallbacks.RemoveDynamic(TargetActor, &AGameplayAbilityTargetActor::CancelTargeting);
			TargetActor->GenericDelegateBoundASC = nullptr;
		}
	}
	
	if (Super::StaticClass() == UAbilityTask_WaitTargetData::StaticClass())
	{
		UAbilityTask_WaitTargetData::Super::OnDestroy(AbilityEnded);	// skip UAbilityTask_WaitTargetData's call on OnDestroy()
	}
	else
	{
		Super::OnDestroy(AbilityEnded);
	}
}
