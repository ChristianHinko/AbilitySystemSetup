// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilitySet.h"

#include "AbilitySystemSetupComponent.generated.h"


class UGameplayAbility;
class UAbilitySystemComponent;
class UGameplayEffect;
enum class EGameplayEffectReplicationMode : uint8;
struct FGameplayAbilitySpec;
class UAttributeSet;



DECLARE_MULTICAST_DELEGATE_OneParam(FAbilitySystemSetupDelegate, UAbilitySystemComponent*);


/**
 * NOTE: Actor owning this component must be the avatar actor.
 * 
 * 
 * 
 * Provides the minimal and necessary setup for Ability System Components.
 * Provides a nice setup for ASCs where the Avatar Actor differs from the Owner Actor (this was the primary motivation behind this component).
 * Provides setup functions for unpossession and re-possession the Avatar Actor.
 * 
 * For Player controlled pawns, this currently expects your ASC to be on your PlayerState. We do however plan on also supporting the ASC being on the Pawn for games where that is needed. We have a method for achieving this (just make use of the AIAbilitySystemComponent for that situation).
 * The Player State's ASC is kept track of by the PlayerAbilitySystemComponent pointer.
 * 
 * This component is flexible enough to be used on any Actor that has an Ability System Component.
 * There are 2 different scenerios where you need a setup with the Ability System:
 * 		1) The Actor is being player controlled (a Pawn)
 * 			- PlayerAbilitySystemComponent will be used (exists outside of this component)
 * 			- In this case, we need to make our Pawn the Avatar Actor of the Player's ASC.
 * 		2) The Actor is NOT player controlled (a wall, tree, or even an AI controlled Pawn)
 * 			- AIAbilitySystemComponent will be used (exists on this component)
 * 			- In this case, we need to make our Pawn both the Avatar Actor and Owner Actor of his ASC.
 * 
 * 
 * 
 * 
 * Key Features:
 * 		1) Gameplay Abilities
 * 			- Fill out StartingAbilities with the Gameplay Ability classes that you want to be given.
 * 			- If you want a Spec Handle for a starting Ability, use the OnGiveStartingAbilities delegate to give the Ability by Spec Handle.
 * 			- Any Abilities with their SourceObject as this Actor will be automatically cleared from the ASC on UnPossessed.
 * 				- If you need an Ability to persist between Characters make sure you set its SourceObject to the Player State (or something persistent) on Give Ability.
 * 
 * 		2) Attribute Sets
 * 			- Fill out StartingAttributeSets with the Attribute Set classes that you want to be created and added.
 * 			- If, for some reason, you need advanced control over this then use the OnAddStartingAttributeSets delegate to add any created Attribute Sets to the ASC.
 * 
 * 		3) Gameplay Effects
 * 			- Fill out StartingEffects with any Gameplay Effects that you want to be applied on startup (e.g. GE_InitCharacter, GE_HealthRegen).
 * 
 * 		4) Gameplay Tags
 * 			- Use to RemoveLooseAvatarRelatedTagsDelegate delegate to remove all avatar-related Tags from the ASC.
 * 				- An example of how your game might implement this is by removing all Tags with the "Character", "Pawn", or "Actor" parent Tags (this sounds like a pain).
 * 				- Also we haven't needed to use this yet so we don't know the best way to remove these Tags. The only way I can think to do this is by dynamically
 * 				defining a Gameplay Effect and applying it.
 * 			- NOTE: This should be a last resort solution. If you really need this much functionality, then maybe just put the ASC on the Character.
 * 
 */
UCLASS(ClassGroup=(AbilitySystemSetup), meta=(BlueprintSpawnableComponent))
class ABILITYSYSTEMSETUP_API UAbilitySystemSetupComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UAbilitySystemSetupComponent(const FObjectInitializer& ObjectInitializer);


	// Ability sets to grant to this pawn's ability system.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySets")
		TArray<TSubclassOf<UAbilitySet>> AbilitySets;


	/**
	 * ----- Public Functions for Owner to Call -----
	 */
	/** Sets the Avatar Actor with the ASC */
	void InitializeAbilitySystemComponent(UAbilitySystemComponent* ASC);
	/** Clears the Avatar Actor from the ASC */
	void UninitializeAbilitySystemComponent();
	/** Called by the owning pawn when the Pawn's Controller changes i.e. PossessedBy(), UnPossessed(), and OnRep_Controller() */
	void HandleControllerChanged();
	/** Called at the end of your Pawn's SetupPlayerInputComponent() event */
	void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent);




	/** Broadcasted when the Ability System is set up and ready to go */
	FAbilitySystemSetupDelegate OnInitializeAbilitySystemComponentDelegate;
	/**
	 * Server and client event for removing all Loose AvatarActor-related Tags.
	 * NOTE: See example implementation of this event in "C_AbilitySystemSetupCharacter.cpp".
	 */
	FAbilitySystemSetupDelegate RemoveLooseAvatarRelatedTagsDelegate;

protected:
	virtual void OnRegister() override;

private:
	/**
	 * ----- Initialization Functions -----
	 */
	/** Makes the input events work for GAS */
	void BindAbilitySystemInput(UInputComponent* InputComponent);

	/**
	 * ----- Uninitialization Functions -----
	 */
	/** Removes all Loose Gameplay Tags that external sources specified we should remove */
	void RemoveLooseAvatarRelatedTags();




	/**
	 * ----- Internal members -----
	 */
	/** Abilities, Active Effects, and Attribute Sets to keep track of so we can clean them up from our ASC on UnPossess */
	TArray<FAbilitySetGrantedHandles> GrantHandles;
	/** Indicates that the list of AbilitySets has been granted */
	uint8 bGrantedAbilitySets : 1;
	/** Indicates that input has already been binded with the Ability System */
	uint8 bAbilitySystemInputBinded : 1;
	UPROPERTY()
		TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	/** AttributeSets that have been created. Kept track of so that we can add and remove them when needed. */
	UPROPERTY()
		TArray<UAttributeSet*> CreatedAttributeSets;
};
