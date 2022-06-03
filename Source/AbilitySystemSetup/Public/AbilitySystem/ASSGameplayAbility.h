// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystem/ASSAbilitySystemComponent.h"

#include "ASSGameplayAbility.generated.h"



/**
 * Base Gameplay Ability
 */
UCLASS()
class ABILITYSYSTEMSETUP_API UASSGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UASSGameplayAbility();


	// Every Gameplay Ability must have this assigned to something in their constructor so we know what InputID to give the Ability Spec in ASSAbilitySystemComponent::OnGiveAbility(). If the Ability doesn't have an input associated with it, it can be assigned to NoInput.
	UPROPERTY(EditDefaultsOnly, Category = "Input")
		uint8 AbilityInputID;

	uint8 bPassiveAbility : 1;


	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override final;
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/**
	 * The engine's OnAvatarSet() is not called properly for instanced abilities! When the avatar actor changes, the
	 * event is mistakingly called on the CDO.
	 * This version correctly gets called on instanced abilities when the avatar actor changes.
	 */
	virtual void OnAvatarSetThatWorks(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec);

	/** An exposed EndAbility() that isn't a cancellation. Used for ability batching. */
	virtual void ExternalEndAbility();

private:
	//Called to try to active the passive ability
	void TryActivatePassiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const;
};
