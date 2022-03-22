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

	void GiveAbilities(const TArray<FGameplayAbilitySpec>& Abilities);

	/** Gives abilities that an other given ASC has */
	void RecieveAbilitiesFrom(const UAbilitySystemComponent* Other);

	/**
	 * This code is taken from UAbilitySystemComponent::TargetConfirm() and modified to only confirm targeting on
	 * the Target Actors associated with the given AbilityToConfirmTargetOn.
	 */
	virtual void TargetConfirmByAbility(UGameplayAbility* AbilityToConfirmTargetOn);
	/**
	 * This code is taken from UAbilitySystemComponent::TargetCancel() and modified to re-add the Target Actors that
	 * are not associated with the given AbilityToCancelTargetOn.
	 */
	virtual void TargetCancelByAbility(UGameplayAbility* AbilityToCancelTargetOn);

	// This override adds a check to see if we should confirm/cancel target actors associated with the InputID on keypress
	virtual void BindAbilityActivationToInputComponent(UInputComponent* InputComponent, FGameplayAbilityInputBinds BindInfo) override;

	/** Returns a list of currently active ability instances that match the tags */
	void GetActiveAbilitiesWithTags(const FGameplayTagContainer& GameplayTagContainer, TArray<UGameplayAbility*>& ActiveAbilities);

	/** Returns an ability spec handle from a class. If modifying call MarkAbilitySpecDirty */
	FGameplayAbilitySpecHandle FindAbilitySpecHandleFromClass(TSubclassOf<UGameplayAbility> AbilityClass, UObject* OptionalSourceObject = nullptr);

	// Gameplay cue helpers for running them locally
	void ExecuteGameplayCueLocal(const FGameplayTag& GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);
	void AddGameplayCueLocal(const FGameplayTag& GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);
	void RemoveGameplayCueLocal(const FGameplayTag& GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);

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
