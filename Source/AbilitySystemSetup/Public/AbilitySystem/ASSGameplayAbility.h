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


	// Every gameplay ability must have this set to something in their constructor so we know what input id to give the ability when we grant it. If the ability doesn't have an input associated with it, it can be left as None.
	UPROPERTY(EditDefaultsOnly, Category = "Input")
		uint8 AbilityInputID;

	bool ActivateAbilityOnGrant = false;

	// Epic's comment: Projects may want to initiate passives or do other "BeginPlay" type of logic here.
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** An exposed EndAbility() that isn't a cancellation. Used for ability batching. */
	virtual void ExternalEndAbility();


	//virtual void OnCurrentAbilityPredictionKeyRejected(); // Not implemented or hooked up yet

	virtual void OnActivationPredictionKeyRejected(); // Not implemented or hooked up yet

};

/** stupid */
#define TryCallOnAvatarSetOnPrimaryInstance																						\
if (HasAnyFlags(RF_ClassDefaultObject) && InstancingPolicy != EGameplayAbilityInstancingPolicy::NonInstanced)					\
{																																\
	if (UGameplayAbility* PrimaryInstance = Spec.GetPrimaryInstance())															\
	{																															\
		PrimaryInstance->OnAvatarSet(ActorInfo, Spec);																			\
		return;																													\
	}																															\
}																																
