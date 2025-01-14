// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"

#include "ASSAbilitySystemComponent.generated.h"

class UInputComponent;
struct FGameplayAbilityInputBinds;

/**
 * Base Ability System Component
 */
UCLASS()
class ABILITYSYSTEMSETUP_API UASSAbilitySystemComponent : public UAbilitySystemComponent
{
    GENERATED_BODY()

public:

    UASSAbilitySystemComponent(const FObjectInitializer& objectInitializer);

protected:

    virtual void OnGiveAbility(FGameplayAbilitySpec& abilitySpec) override;

protected:

    // NOTE: We abandon the ability system's ability input binding due to its limitations. Therefore we check
    //       no entry on their input setup functions since we use our setup instead (InputSetup module).
#if DO_CHECK
    virtual void BindToInputComponent(UInputComponent* inputComponent) override;
    virtual void BindAbilityActivationToInputComponent(UInputComponent* inputComponent, FGameplayAbilityInputBinds bindInfo) override;
    virtual void AbilityLocalInputPressed(int32 inputID) override;
    virtual void AbilityLocalInputReleased(int32 inputID) override;
#endif // DO_CHECK

    FORCEINLINE virtual bool ShouldDoServerAbilityRPCBatch() const override { return true; }

    // Not tested yet!!!!!!! Beware. Could also be better optimized I'm sure. Anyways this function resets the ASC as if it were new again.
    void FullReset();
};
