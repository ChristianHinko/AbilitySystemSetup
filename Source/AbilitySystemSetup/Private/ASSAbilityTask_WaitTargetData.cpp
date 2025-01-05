// Fill out your copyright notice in the Description page of Project Settings.

#include "ASSAbilityTask_WaitTargetData.h"

#include "GCUtils_Log.h"
#include "AbilitySystemComponent.h"
#include "ASSGameplayAbilityTargetActor.h"

DEFINE_LOG_CATEGORY(LogASSAbilityTask_WaitTargetData)

UASSAbilityTask_WaitTargetData* UASSAbilityTask_WaitTargetData::ASSWaitTargetDataUsingActor(UGameplayAbility* OwningAbility, FName TaskInstanceName, TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType, AGameplayAbilityTargetActor* InTargetActor)
{
    UASSAbilityTask_WaitTargetData* MyObj = NewAbilityTask<UASSAbilityTask_WaitTargetData>(OwningAbility, TaskInstanceName);
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


            if (!IsValid(this))
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
                if (TargetActor->bDestroyOnConfirmation)    // If this is true, the developer obiously is handling the GATA lifetime on his own. So we won't destroy it
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
    if (IsValid(TargetActor))
    {
        if (TargetActor->bDestroyOnConfirmation)
        {
            TargetActor->Destroy();
        }
        else
        {
            // Instead of destroying it, just deactivate it

            AASSGameplayAbilityTargetActor* ASSTargetActor = Cast<AASSGameplayAbilityTargetActor>(TargetActor);
            if (IsValid(ASSTargetActor))
            {
                ASSTargetActor->DisableTargetActor();
            }
            else
            {
                GC_LOG_STR_UOBJECT(
                    this,
                    LogASSAbilityTask_WaitTargetData,
                    Warning,
                    TEXT("Your not using our custom base target actor. Tried to call DisableTargetActor() but we couldn't because of this")
                );
            }

            // Clear added callbacks
            TargetActor->TargetDataReadyDelegate.RemoveAll(this);
            TargetActor->CanceledDelegate.RemoveAll(this);

            AbilitySystemComponent->GenericLocalConfirmCallbacks.RemoveDynamic(TargetActor, &AGameplayAbilityTargetActor::ConfirmTargeting);
            AbilitySystemComponent->GenericLocalCancelCallbacks.RemoveDynamic(TargetActor, &AGameplayAbilityTargetActor::CancelTargeting);
            TargetActor->GenericDelegateBoundASC = nullptr;
        }
    }

    // Skip UAbilityTask_WaitTargetData's OnDestroy() implementation
    Super::Super::OnDestroy(AbilityEnded);
}
