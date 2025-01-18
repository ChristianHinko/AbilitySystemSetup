// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"

class UInputComponent;
struct FGameplayAbilityInputBinds;

struct FASSAbilitySystemComponentExtention
{
    // NOTE: We abandon the ability system's ability input binding due to its limitations. Therefore we check
    //       no entry on their input setup functions since we use our setup instead (InputSetup module).
#if DO_CHECK
    void BindToInputComponent_ReplaceSuper(UInputComponent* inputComponent);
    void BindAbilityActivationToInputComponent_ReplaceSuper(UInputComponent* inputComponent, FGameplayAbilityInputBinds bindInfo);
    void AbilityLocalInputPressed_ReplaceSuper(int32 inputID);
    void AbilityLocalInputReleased_ReplaceSuper(int32 inputID);
#endif // DO_CHECK
    
#if DO_CHECK || !NO_LOGGING
    void OnGiveAbility_PostSuper(UAbilitySystemComponent& asc, FGameplayAbilitySpec& abilitySpec);
#endif // #if DO_CHECK || !NO_LOGGING

};
