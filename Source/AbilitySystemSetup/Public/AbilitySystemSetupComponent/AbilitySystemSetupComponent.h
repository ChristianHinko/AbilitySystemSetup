// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "AbilitySystemSetupComponent.generated.h"


class IAbilitySystemSetupInterface;
class UGameplayAbility;
class UAbilitySystemComponent;
class IAbilitySystemInterface;
class UGameplayEffect;
enum class EGameplayEffectReplicationMode : uint8;
struct FGameplayAbilitySpec;
class UAttributeSet;



DECLARE_MULTICAST_DELEGATE_TwoParams(FAbilitySystemComponentChangeDelegate, UAbilitySystemComponent* const/*, PreviousASC*/, UAbilitySystemComponent* const/*, NewASC*/);


/**
 * Provides the minimal and necessary setup for Ability System Components.
 * Provides a nice setup for ASCs where the Avatar Actor differs from the Owner Actor (this was the primary motivation behind this component).
 * Provides setup functions for unpossession and re-possession the Avatar Actor.
 * 
 * For Player controlled pawns, this currently expects your ASC to be on your PlayerState. We do however plan on also supporting the ASC being on the Pawn for games where that is needed. We have a method for achieving this (just make use of the AIAbilitySystemComponent for that situation).
 * The Player State's ASC is kept track of by the PlayerAbilitySystemComponent pointer.
 * 
 * This component is flexible enough to be used on any Actor that needs a setup with Ability System.
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
 * 		- Use bClearAbilitiesOnUnPossessed to clear given Abilities
 * 		- Use bRemoveCharacterTagsOnUnpossessed to remove character tags (not implemented)
 * 
 * 
 * Key Features:
 * 		1) Gameplay Abilities
 * 			- Fill out StartingAbilities with the Gameplay Ability classes that you want to be given.
 * 			- If you want a Spec Handle for a starting Ability, use IAbilitySystemSetupInterface::GiveStartingAbilities() to give the Ability by Spec Handle.
 * 			- Any Abilities with their SourceObject as this Actor will be automatically cleared from the ASC on UnPossessed (assuming bClearAbilitiesOnUnPossessed).
 * 				- If you need an Ability to persist between Characters make sure you set its SourceObject to the Player State (or something persistent) on Give Ability.
 * 
 * 		2) Attribute Sets
 * 			- Fill out StartingAttributeSets with the Attribute Set classes that you want to be created and added.
 * 			- If, for some reason, you need advanced control over this then use IAbilitySystemSetupInterface::AddAttributeSets() to add any created Attribute Sets to the ASC.
 * 			- Any Attribute Sets owned by this Actor will be automatically removed from the ASC on UnPossessed (assuming bRemoveAttributeSetsOnUnPossessed).
 * 				- If you need an Attribute Set to persist between Characters make sure you manually set its outer to the Player State (or something persistent). Even better, create
 * 				the Attribute Set as a default subobject on the Player State class (if possible) and it will automatically be added to the Player's ASC and persist accross possessions.
 * 
 * 		3) Gameplay Effects
 * 			- Fill out InitializationEffectTSub for using Attribute Set initialization via Gameplay Effect.
 * 			- Fill out StartingEffects with any Gameplay Effects that you want to be applied on startup (e.g. GE_HealthRegen or GE_StaminaRegen).
 * 
 * 		4) Gameplay Tags
 * 			- If you need bRemoveCharacterTagsOnUnpossessed, then implement RemoveAllCharacterTags() to remove all character related Tags from the ASC.
 * 				- An example of how your game might implement this is by removing all Tags with the "Character", "Pawn", and "Actor" parent Tags (this sounds like a pain).
 * 				- Also we haven't needed to use this yet so we don't know the best way to remove these Tags. The only way I can think to do this is by dynamically
 * 				defining a Gameplay Effect in the RemoveAllCharacterTags() function and applying it.
 * 			- NOTE: This should be a last resort solution. If you really need this much functionality, then maybe just put the ASC on the Character.
 * 
 */
UCLASS(ClassGroup=(AbilitySystemSetup), meta=(BlueprintSpawnableComponent))
class ABILITYSYSTEMSETUP_API UAbilitySystemSetupComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	// Only one of these ASC will be active at a time:

	/**
	 * Points to the PlayerState's ASC
	 */
	UPROPERTY()
		TWeakObjectPtr<UAbilitySystemComponent> PlayerAbilitySystemComponent;
	/**
	 * This is used if an AIController is posessing. However, it is also used as a placeholder ASC when before the Player possesses this Character (so we can give Abilities and stuff).
	 * These Abilities will be transfered from this ASC to the Player's (this allows us to give Abilities early on).
	 * TODO: Maybe rename this to ActorAbilitySystemComponent or AvatarAbilitySystemComponent to make it more generic.
	 * 
	 * This can be disabled by calling DoNotCreateDefaultSubobject() in the constructor using the ObjectInitializer. NOTE: You would have to subclass this component to do this.
	 * TODO: We should have the Owning Actor inject his ASC here (and make this a weak pointer) instead of us making it for them.
	 */
	UPROPERTY(VisibleAnywhere, Category = "AbilitySystemSetup")
		UAbilitySystemComponent* AIAbilitySystemComponent;

public:
	UAbilitySystemSetupComponent(const FObjectInitializer& ObjectInitializer);


	/**
	 * Gets the active Ability System Component.
	 * That is, the Player's ASC if Player controlled or the AI's ASC if AI controlled.
	 * 
	 * Return this in your IAbilitySystemInterface::GetAbilitySystemComponent() implementation.
	 */
	UAbilitySystemComponent* GetAbilitySystemComponent() const;



	// TODO: Try to mix both of these setup functions into one SetUpWithAbilitySystem() function. Also try to get rid of us depending on the Player State.
	/**
	 * Sets the Avatar Actor with the ASC when it is Player controlled
	 */
	void SetupWithAbilitySystemPlayerControlled(APlayerState* PlayerState);
	/**
	 * Sets the Avatar Actor with the ASC when it is AI controlled
	 */
	void SetupWithAbilitySystemAIControlled();



	UAbilitySystemComponent* GetAIAbilitySystemComponent() const { return AIAbilitySystemComponent; }


	/**
	 * Called from both SetupPlayerInputComponent() and OnRep_PlayerState() because of a potential race condition where the Player Controller might
	 * call ClientRestart() which calls SetupPlayerInputComponent() before the Player State is repped to the client so the Player State would be null in SetupPlayerInputComponent().
	 * Conversely, the Player State might be repped before the Player Controller calls ClientRestart() so the Actor's Input Component would be null in OnRep_PlayerState().
	 */
	void BindASCInput(UInputComponent* InputComponent);

	/**
	 * On OwningPawn becomes UnPossessed.
	 * NOTE: Call this BEFORE Super::UnPossessed()!
	 */
	virtual void UnPossessed();

protected:
	virtual void InitializeComponent() override;

	APawn* OwningPawn;
	IAbilitySystemSetupInterface* OwningAbilitySystemSetupInterface;

public:
	/**
	 * Broadcasted when the Ability System is set up and ready to go
	 */
	FAbilitySystemComponentChangeDelegate OnAbilitySystemSetUp;
	/**
	 * Broadcasted when the Ability System is set up BUT before starting Effects are applied, before Attributes are initialized, and before starting Abilities are given
	 */
	FAbilitySystemComponentChangeDelegate OnAbilitySystemSetUpPreInitialized;


	/**
	 * Attribute Sets to create and add on startup
	 */
	UPROPERTY(EditDefaultsOnly, Category = "AbilitySystemSetup|AttributeSets")
		TArray<TSubclassOf<UAttributeSet>> StartingAttributeSets;
	/**
	 * Initialization effect to give starting values for variables/attributes
	 */
	UPROPERTY(EditDefaultsOnly, Category = "AbilitySystemSetup|AttributeSets")
		TSubclassOf<UGameplayEffect> InitializationEffectTSub;

	/**
	 * These Effects are only applied one time on startup and after the InitializationEffectTSub effect
	 * Example starting effects: GE_HealthRegen, GE_StaminaRegen
	 */
	UPROPERTY(EditDefaultsOnly, Category = "AbilitySystemSetup|Effects")
		TArray<TSubclassOf<UGameplayEffect>> StartingEffects;

protected:
	/** Called only on server. This is the earliest place you can give an Ability. */
	bool GiveStartingAbilities();

	/** NOTE: No AbilitySpecHandles are tracked upon give. These Abilities must be activated by class or by Ability tag. These Abilities are assigned EAbilityInputID::None */
	UPROPERTY(EditDefaultsOnly, Category = "AbilitySystemSetup|Abilities")
		TArray<TSubclassOf<UGameplayAbility>> StartingAbilities;


	/**
	 * Takes this object's Attribute Set(s) away from the current ASC. This is on by default to prevent the potential problem of the ASC having 2 Attribute Sets of the same class.
	 * However if the ASC no longer has this object's Attribute Set, Gameplay Effects can no longer modify their Attributes.
	 * Disabling this would be useful for features such as switching to possessing a drone mid-game. Which case you would obviously want to keep your character's health Attribute Sets and such.
	 */
	UPROPERTY(EditAnywhere, Category = "AbilitySystemSetup|Config")
		uint8 bRemoveAttributeSetsOnUnPossessed : 1;
	/**
	 * Remove all Abilities that were given by us on unpossess
	 */
	UPROPERTY(EditAnywhere, Category = "AbilitySystemSetup|Config")
		uint8 bClearAbilitiesOnUnPossessed : 1;
	/**
	 * --- CURRENTLY DOES NOTHING. IMPLEMENT RemoveAllCharacterTags() FOR THIS TO DO SOMETHING ---
	 * Removes all Tags relating to this specific character from the PlayerState's ASC
	 * Remove all Tags related to the character, that way when we possess a new character,
	 * the old Tags don't interfere with the new character.
	 */
	UPROPERTY(EditAnywhere, Category = "AbilitySystemSetup|Config")
		uint8 bRemoveCharacterTagsOnUnpossessed : 1;


	/** Removes all Attribute Sets that we added to the ASC */
	int32 RemoveOwnedAttributeSets();
	/** Removes all Abilities that we've given to the ASC */
	int32 ClearGivenAbilities();
	/** NOT IMPLEMENTED YET! Removes all Tags relating to this specific character from the PlayerState's ASC */
	int32 RemoveAllCharacterTags();

private:
	UPROPERTY()
		TWeakObjectPtr<UAbilitySystemComponent> PreviousASC;

	// TODO: This is temporary - in UE5, APawn has its own PreviousController variable that we can use rather than making our own
	UPROPERTY()
		TWeakObjectPtr<AController> PreviousController;


	/** Add Attribute Sets to the ASC using the StartingAttributeSets array and calling on IAbilitySystemSetupInterface::AddAttributeSets() */
	void AddAttributeSets();
	/** Initialize Attribute values using the InitializationEffectTSub */
	void InitializeAttributes();
	/** Apply all Effects listed in StartingEffects */
	void ApplyStartingEffects();

	/** AttributeSets that have been created. Kept track of so that we can add and remove them when needed. */
	UPROPERTY()
		TArray<UAttributeSet*> CreatedAttributeSets;


	TArray<FGameplayAbilitySpec> PendingAbilitiesToTransfer;



	// Internal state bools:

	/** Indicates that we already created Attribute Sets and added them, initialized the Attributes, and applied the starting Effects */
	uint8 bInitialized : 1;
	/** Shows that we already have input binded with the Ability System */
	uint8 bASCInputBound : 1;
};
