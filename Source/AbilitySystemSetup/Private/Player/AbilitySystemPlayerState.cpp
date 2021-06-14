// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AbilitySystemPlayerState.h"



FName AAbilitySystemPlayerState::AbilitySystemComponentName(TEXT("AbilitySystemComponent"));

AAbilitySystemPlayerState::AAbilitySystemPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UASSAbilitySystemComponent>(AbilitySystemComponentName))
{
	// Create ability system component, and set it to be explicitly replicated
	ASSAbilitySystemComponent = CreateDefaultSubobject<UASSAbilitySystemComponent>(AbilitySystemComponentName);
	ASSAbilitySystemComponent->SetIsReplicated(true);

	// Mixed mode means we only are replicated the GEs to ourself, not the GEs to simulated proxies. If another GDPlayerState (Hero) receives a GE,
	// we won't be told about it by the Server. Attributes, GameplayTags, and GameplayCues will still replicate to us.
	ASSAbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);


	// Set PlayerState's NetUpdateFrequency to the same as the Pawn.
	// Default is very low for PlayerStates and introduces perceived lag in the ability system.
	// 100 is probably way too high for a shipping game, you can adjust to fit your needs.
	NetUpdateFrequency = 100.0f;
	// we're not sure if adaptive net update frequency is safe for PS or what a safe min update frequency would be for PS so we just set it to the max
	MinNetUpdateFrequency = NetUpdateFrequency;

}
