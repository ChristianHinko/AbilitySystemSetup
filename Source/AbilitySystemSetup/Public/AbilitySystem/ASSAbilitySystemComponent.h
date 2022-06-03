// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"

#include "ASSAbilitySystemComponent.generated.h"


class UASSGameplayAbility;



/**
 * Base Ability System Component
 */
UCLASS()
class ABILITYSYSTEMSETUP_API UASSAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UASSAbilitySystemComponent(const FObjectInitializer& ObjectInitializer);


	virtual bool ShouldDoServerAbilityRPCBatch() const override { return true; }

	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;

	// This override adds a check to see if we should confirm/cancel target actors associated with the InputID on keypress
	virtual void BindAbilityActivationToInputComponent(UInputComponent* InputComponent, FGameplayAbilityInputBinds BindInfo) override;

	/** Not tested yet!!!!!!! Beware. Could also be better optimized I'm sure. Anyways this function resets the ASC as if it were new again. */
	void FullReset();

protected:
	virtual void InitializeComponent() override;

	/** This override adds a check to see if we should activate the ability associated with the InputID on keypress, according to bDoNotAutoActivateFromGASBindings*/
	virtual void AbilityLocalInputPressed(int32 InputID) override;


	/**
	 * If false, abilities will activate when a bound input is pressed. Keep this enabled if you don't want this but still want input binded
	 * (because you may want to manually control when it gets called but still want to use the tasks that binded input gives you)
	 */
	uint8 bDoNotAutoActivateFromGASBindings : 1;
	/**
	 * If false, target actors will confirm/cancel when a bound input is pressed. Keep enabled if you don't want this but still want input binded
	 * (because you may want to manually control when confirm/cancel get called but still want to use the tasks that binded input gives you)
	 */
	uint8 bDoNotAutoConfirmAndCancelFromGASBindings : 1;
};
