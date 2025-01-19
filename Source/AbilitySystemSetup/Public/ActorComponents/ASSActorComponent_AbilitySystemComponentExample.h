// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"

#include "ASSAbilitySystemComponentExtention.h"

#include "ASSActorComponent_AbilitySystemComponentExample.generated.h"

/**
 * @brief Example base class for implementing our extended ASC functionality. Feel
 *        free to subclass if lazy.
 */
UCLASS()
class ABILITYSYSTEMSETUP_API UASSActorComponent_AbilitySystemComponentExample : public UAbilitySystemComponent
{
    GENERATED_BODY()

public:

    UASSActorComponent_AbilitySystemComponentExample(const FObjectInitializer& inObjectInitializer);

public:

    // NOTE: We abandon the ability system's ability input binding due to its limitations. Therefore we check
    //       no entry on their input setup functions since we use our setup instead (InputSetup module).
#if DO_CHECK
    void BindToInputComponent(UInputComponent* inputComponent) override;
    void BindAbilityActivationToInputComponent(UInputComponent* inputComponent, FGameplayAbilityInputBinds bindInfo) override;
    void AbilityLocalInputPressed(int32 inputID) override;
    void AbilityLocalInputReleased(int32 inputID) override;
#endif // DO_CHECK

#if DO_CHECK || !NO_LOGGING
    void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;
#endif // #if DO_CHECK || !NO_LOGGING

private:

    UE_NO_UNIQUE_ADDRESS FASSAbilitySystemComponentExtention AbilitySystemComponentExtention;
};
