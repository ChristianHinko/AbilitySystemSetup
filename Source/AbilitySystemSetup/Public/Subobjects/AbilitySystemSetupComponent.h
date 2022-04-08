// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "AbilitySystemSetupComponent.generated.h"


class UGameplayAbility;
class UAbilitySystemComponent;
class UGameplayEffect;
enum class EGameplayEffectReplicationMode : uint8;
struct FGameplayAbilitySpec;
class UAttributeSet;



DECLARE_MULTICAST_DELEGATE_TwoParams(FAbilitySystemComponentChangeDelegate, UAbilitySystemComponent* const/*, PreviousASC*/, UAbilitySystemComponent* const/*, NewASC*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FAbilitySystemSetupDelegate, UAbilitySystemComponent*);


/**
 * NOTE: Actor with this component must be the avatar actor.
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
 * Having the ASC on the Player State can make unpossession and re-possession a huge pain since given Abilities and added Attribute Sets exist on the Player State rather than the Pawn. So character-
 * specific stats will persist across possessions and respawns.
 * We provide some helpful bools to make unpossession a little better:
 * 		- Use bRemoveAttributeSetsOnUnPossessed to remove added Attribute Sets
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
 * 			- Any Attribute Sets owned by this Actor will be automatically removed from the ASC on UnPossessed (assuming bRemoveAttributeSetsOnUnPossessed).
 * 				- If you need an Attribute Set to persist between Characters make sure you manually set its outer to the Player State (or something persistent). Even better, create
 * 				the Attribute Set as a default subobject on the Player State class (if possible) and it will automatically be added to the Player's ASC and persist accross possessions.
 * 
 * 		3) Gameplay Effects
 * 			- Fill out StartingEffects with any Gameplay Effects that you want to be applied on startup (e.g. GE_InitCharacter, GE_HealthRegen).
 * 
 * 		4) Gameplay Tags
 * 			- Use to OnRemoveAvatarRelatedTags delegate to remove all Character-related Tags from the ASC.
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


	/** NOTE: No AbilitySpecHandles are tracked upon give. These Abilities must be activated by class or by Ability tag. These Abilities are assigned EAbilityInputID::None */
	UPROPERTY(EditDefaultsOnly, Category = "AbilitySystemSetup|Abilities")
		TArray<TSubclassOf<UGameplayAbility>> StartingAbilities;
	/** Attribute Sets to create and add on startup */
	UPROPERTY(EditDefaultsOnly, Category = "AbilitySystemSetup|AttributeSets")
		TArray<TSubclassOf<UAttributeSet>> StartingAttributeSets;
	/** These Effects are only applied one time on startup (example starting effects: GE_InitCharacter, GE_HealthRegen) */
	UPROPERTY(EditDefaultsOnly, Category = "AbilitySystemSetup|Effects")
		TArray<TSubclassOf<UGameplayEffect>> StartingEffects;
	/**
	 * Takes this object's Attribute Set(s) away from the current ASC. This is on by default to prevent the potential problem of the ASC having 2 Attribute Sets of the same class.
	 * However if the ASC no longer has this object's Attribute Set, Gameplay Effects can no longer modify their Attributes.
	 * Disabling this would be useful for features such as switching to possessing a drone mid-game. Which case you would obviously want to keep your character's health Attribute Sets and such.
	 */
	UPROPERTY(EditAnywhere, Category = "AbilitySystemSetup|AttributeSets")
		uint8 bRemoveAttributeSetsOnUnPossessed : 1;


	// Public functions for owner to call..........
	/** Sets the Avatar Actor with the ASC */
	void InitializeAbilitySystemComponent(UAbilitySystemComponent* ASC);
	/** Clears the Avatar Actor from the ASC */
	void UninitializeAbilitySystemComponent();
	/** Called by the owning pawn when the Pawn's Controller changes i.e. PossessedBy(), UnPossessed(), and OnRep_Controller() */
	void HandleControllerChanged();
	/** Called at the end of your Pawn's SetupPlayerInputComponent() event */
	void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent);




	/** Broadcasted when the Ability System is set up and ready to go */
	FAbilitySystemComponentChangeDelegate OnAbilitySystemSetUpDelegate;
	/** Broadcasted when the Ability System is set up BUT before starting Effects are applied and before starting Abilities are given */
	FAbilitySystemComponentChangeDelegate OnAbilitySystemSetUpPreInitializedDelegate;
	
	/**
	 * Server only event for giving starting Attribute Sets via C++.
	 * NOTE: See example implementation of this event in "C_AbilitySystemSetupCharacter.cpp".
	 */
	FAbilitySystemSetupDelegate AddStartingAttributeSetsDelegate;
	/**
	 * Server only event for giving starting abilities via C++.
	 * NOTE: See example implementation of this event in "C_AbilitySystemSetupCharacter.cpp".
	 */
	FAbilitySystemSetupDelegate GiveStartingAbilitiesDelegate;
	/**
	 * Server only event for removing all AvatarActor-related Tags.
	 * NOTE: See example implementation of this event in "C_AbilitySystemSetupCharacter.cpp".
	 */
	FAbilitySystemSetupDelegate RemoveAvatarRelatedTagsDelegate;

protected:
	virtual void InitializeComponent() override;

private:
	/** Makes the input events work for GAS */
	void BindAbilitySystemInput(UInputComponent* InputComponent);
	/** Add starting Attribute Sets to the ASC using the StartingAttributeSets array and broadcasting OnAddStartingAttributeSets */
	void AddStartingAttributeSets();
	/** Apply all Effects listed in StartingEffects */
	void ApplyStartingEffects();
	/** Give all Abilities listed in StartingAbilities. */
	bool GiveStartingAbilities();

	/** Removes all Abilities that we've given to the ASC */
	int32 ClearGivenAbilities();
	/** Removes all Attribute Sets that we added to the ASC */
	int32 RemoveOwnedAttributeSets();




	//			Internal members:
	/** In most cases, our AvatarActor */
	APawn* OwningPawn;
	/** Shows that we already have input binded with the Ability System */
	uint8 bAbilitySystemInputBinded : 1;
	/** Indicates that we have not applied the StartingEffects yet */
	uint8 bFirstInitialization : 1;
	UPROPERTY()
		TWeakObjectPtr<UAbilitySystemComponent> CurrentASC;
	UPROPERTY()
		TWeakObjectPtr<UAbilitySystemComponent> PreviousASC;
	/** AttributeSets that have been created. Kept track of so that we can add and remove them when needed. */
	UPROPERTY()
		TArray<UAttributeSet*> CreatedAttributeSets;
	// This is temporary - in UE5, APawn has its own PreviousController variable that we can use rather than making our own
	UPROPERTY()
		TWeakObjectPtr<AController> PreviousController;
};
