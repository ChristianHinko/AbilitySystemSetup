// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"

#include "ASSAbilityTask_WaitTargetData.generated.h"

/**
 * Base that supports re-using of Target Actors that have bDestroyOnConfirmation disabled
 */
UCLASS()
class ABILITYSYSTEMSETUP_API UASSAbilityTask_WaitTargetData : public UAbilityTask_WaitTargetData
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", HideSpawnParms = "Instigator"), Category = "Ability|Tasks")
        static UASSAbilityTask_WaitTargetData* ASSWaitTargetDataUsingActor(UGameplayAbility* OwningAbility, FName TaskInstanceName, TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType, AGameplayAbilityTargetActor* InTargetActor);

protected:
    virtual void Activate() override;

    virtual void OnDestroy(bool AbilityEnded) override;
};
