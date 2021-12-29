// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "AbilitySystemSetupComponent.generated.h"


class IAbilitySystemSetupInterface;
class UASSGameplayAbility;
class UASSAbilitySystemComponent;
class IAbilitySystemInterface;
class UGameplayEffect;
enum class EGameplayEffectReplicationMode : uint8;
struct FGameplayAbilitySpec;



DECLARE_MULTICAST_DELEGATE(FSetupWithAbilitySystemDelegate);


/**
 * For Avatar Actors of UAbilitySystemComponents that may be possessed by different Owner Actors.
 * 
 * This was designed to be as flexable as possible for things regarding unpossesion/reposession of the Pawn. Having the ASC on the Player State (or just not on the Pawn in general) can make unpossesion and repossession a huge pain since
 * granted Abilities and Attribute Sets exist on the Player State's ASC (because the pawn doesn't have one unless it's an AI).
 * To make this less painful, some useful bools were implemented:
 *		- bUnregisterAttributeSetsOnUnpossessed
 *		- bRemoveAbilitiesOnUnpossessed
 *		- bRemoveCharacterTagsOnUnpossessed
 *		
 * 
 * Some tips:
 * 		1) On UnPossessed of this Pawn (a lot of times also means that we are about to be possessed by another Controller)
 * 			- Removes granted Abilities
 *			- Unregisters Attribute Sets
 *			
 * 		2) Levels
 *			- Not supported yet (we just haven't had a need for them yet)
 * 			- Always passed in as 1 for now. GetLevel() is commented out at all places where the level is passed in.
 *			- TODO: Maybe call on the AbilitySystemSetupInterface to get this info
 * 			
 * 		3) Abilities
 * 			- Any Abilities added with its SourceObject being this component's owning Actor will be automatically removed from the ASC on UnPossessed. If you want an Ability to persist between characters make sure you manually set its SourceObject to the PlayerState. (This may be kind of an annoying solution)
 * 			- If you want a Spec Handle for a starting Ability, grant it in GrantStartingAbilities() using AbilitySystemSetupInterface.
 * 			
 *		4) Attribute Sets
 * 			- Any Attribute Sets owned by this component's owning Actor will be automatically removed from the ASC on UnPossessed (owner actor is automatically set by the engine). If you want an attribute set to persist between characters make sure you manually set its owner to the PlayerState. (still haven't tested with non subobject attribute sets)
 * 			- To use Attribute Sets, create them in CreateAttributeSets() using the AbilitySystemSetupInterface and register them with your ASC in RegisterAttributeSets() using the AbilitySystemSetupInterface.
 * 			
 *		4) Gameplay Effects
 * 			- To set default Attribute values via Gameplay Effect, set DefaultAttributeValuesEffectTSub in BP to your GE.
 * 			- Gameplay Effects that you want applied initially (BeginPlay-type of GEs) can be done by filling out the EffectsToApplyOnStartup array in BP. (ie. GE_HealthRegen or GE_StaminaRegen)
 * 			
 *		5) Gameplay Tags		=@REVIEW MARKER@=
 * 			- Right now I have a setup for it calling RemoveAllCharacterTags() to remove any character tags from the Player's ASC on Unpossess if bRemoveCharacterTagsOnUnpossessed is set to true, but the function is NOT implemented.
 * 					There is no way of determining what tags to take off of the Player's ASC when unpossessing/destroying this character which is why it is not implemented. If we can figure out a way to give an owner to
 * 					specific tags without putting them on the actually actor, then this should be easily possible. You could implement it by removing all tags from the ASC that has a parent of "Character" so we know all
 * 					the "Character" tags will get removed on unpossess, but that removes some flexability for organization with gameplay tags. Havn't thought of a better way though. If you really feel you need this feature
 * 					implemented somehow, it might be better to just make a new GAS setup with the ASC on the character class.
 * 			- Was struggling on comming up with a good system to determine what tags to remove on Unpossess. We could try removing any tags that has "Character" as one of the parents in the tag, but then this limits us
 * 					us into only using tags with the parent of "Character" for character related stuff (ie. we wouldn't be able to throw a generic "Actor.IsOnFire" tag onto the character because then on Unpossess it wouldn't get removed).
 * 					Another thing to think about is how we will remove these tags. This leads us into having to make a simple and efficient way to add/remove tags through code for all machines. Will we use a GE? This will take care of
 * 					replicating it to all machines, but then you have to have a predefined GE asset that does it. But we would like to have the game figure out at runtime what tags to remove. Reliable RPCs using LooseGameplayTags is an
 * 					option but not prefered, (not very efficient).
 * 					if that tag is on our ASC, it wouldn't be removed.
 *			
 * 		6) AIAbilitySystemComponent
 * 			- Subclasses can disable this feature by calling DoNotCreateDefaultSubobject() through the constructor using the ObjectInitializer.
 *			- TODO: wait how do we do this because now we are an Actor Component
 *			
 * 
 * Some problems to avoid:
 * 		1) Keeping your Attribute Sets registered on UnPossessed (via bUnregisterAttributeSetsOnUnpossessed = false) will give you problems if your newly registered Attribute Set (from a new possession) is the same Class as the old Attribute Set (or both old and new Attribute Sets have a same Attribute from a shared parent).
 *			- To avoid this, have a different Attribute Set for each Character (avoid inheritance from parent Attribute Sets). - I know this sucks but you don't really have to do this if your game doesn't need advanced Pawn possession features.
 * 			
 *		2) If you unpossess a Pawn and unregister his Attribute Sets (without being possessed by someone else), his Attributes are no longer associated with an ASC.
 * 			- Do not apply Effects to Pawns that have unregistered Attribute Sets (it will crash).
 * 				-(Possible future solution: have the AIAbilitySystemComponent take over when a Player unpossesses a character and unregisters its attributes and register those attribute sets with the AI ASC)
 *					-(Possible work around: have an AI controller possess the unpossessed character which will call SetUpWithAbilitySystem for the AIAbilitySystemComponent and register those attribute with the AI ASC)
 *			
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
	UPROPERTY(/*Replicated*/)	// replicated can be helpful for debugging issues
		UASSAbilitySystemComponent* PlayerAbilitySystemComponent;
	/**
	 * This is used if an AIController is posessing. However, it is also used as a placeholder ASC for before the Player possesses this character (so we can give abilities and stuff).
	 * These abilities will be transfered from this ASC to the Player's (this allows us to give abilities early on)
	 */
	UPROPERTY(VisibleAnywhere, Category = "AbilitySystemSetup")
		UASSAbilitySystemComponent* AIAbilitySystemComponent;

public:
	UAbilitySystemSetupComponent(const FObjectInitializer& ObjectInitializer);


	/** Hooks the Avatar Actor to the ASC when it's a Player Controller */
	void SetupWithAbilitySystemPlayerControlled(APlayerState* PlayerState);
	/** Hooks the Avatar Actor to the ASC when it's an AI Controller */
	void SetupWithAbilitySystemAIControlled();

	UASSAbilitySystemComponent* GetAIAbilitySystemComponent() const { return AIAbilitySystemComponent; }


	/**
	 * Called from both SetupPlayerInputComponent() and OnRep_PlayerState() because of a potential race condition where the Player Controller might
	 * call ClientRestart() which calls SetupPlayerInputComponent() before the Player State is repped to the client so the Player State would be null in SetupPlayerInputComponent().
	 * Conversely, the Player State might be repped before the Player Controller calls ClientRestart() so the Actor's Input Component would be null in OnRep_PlayerState().
	 */
	void BindASCInput(UInputComponent* InputComponent);

	/**
	 * On OwningPawn becomes UnPossessed.
	 * NOTE: CALL THIS BEFORE Super::UnPossessed()
	 */
	virtual void UnPossessed();

protected:
	virtual void InitializeComponent() override;

	APawn* OwningPawn;
	IAbilitySystemInterface* OwningAbilitySystemInterface;
	IAbilitySystemSetupInterface* OwningAbilitySystemSetupInterface;



public:
#pragma region AbilitySystemSetup Delegates
	FSetupWithAbilitySystemDelegate SetupWithAbilitySystemCompleted;
	/**
	 * This is broadcasted when the ability system is set up but startup effects aren't applied, default attributes aren't initialized, and
	 * starting abilities aren't granted.
	 */
	FSetupWithAbilitySystemDelegate PreApplyStartupEffects;
#pragma endregion

	/** Default Attributes values on startup. This should be an instant GE with the Modifier Op set to Override so you can choose what the starting Attribute values will be on spawn */
	UPROPERTY(EditAnywhere, Category = "AbilitySystemSetup|Effects")
		TSubclassOf<UGameplayEffect> DefaultAttributeValuesEffectTSub;
	/** These Effects are only applied one time on startup (ie. GE_HealthRegen, GE_StaminaRegen) */
	UPROPERTY(EditAnywhere, Category = "AbilitySystemSetup|Effects")
		TArray<TSubclassOf<UGameplayEffect>> EffectsToApplyOnStartup;

protected:
	/** Called only on server. This is the earliest place you can grant an Ability. */
	bool GrantStartingAbilities();

	/** Note: No AbilitySpecHandles are tracked upon grant. These Abilities must be activated by class or by ability tag. These Abilities are assigned EAbilityInputID::None */
	UPROPERTY(EditAnywhere, Category = "AbilitySystemSetup|Abilities")
		TArray<TSubclassOf<UASSGameplayAbility>> NonHandleStartingAbilities;



	/** Decide which replication mode you want for the AIAbilitySystemComponent. Should normally be set to Minimal. Only change if you know what your doing */
	UPROPERTY(EditAnywhere, Category = "AbilitySystemSetup|AI")
		EGameplayEffectReplicationMode AIAbilitySystemComponentReplicationMode;


	/**
	 * Takes this object's Attribute Set(s) away from the current ASC. This is on by default to prevent the potential problem of the ASC having 2 Attribute Sets of the same class.
	 * However if the ASC no longer has this object's Attribute Set, Gameplay Effects can no longer modify their Attributes.
	 * Disabling this would be useful for features such as switching to possessing a drone mid-game. Which case you would obviously want to keep your character's health Attribute Sets and such.
	 */
	UPROPERTY(EditAnywhere, Category = "AbilitySystemSetup|Config")
		uint8 bUnregisterAttributeSetsOnUnpossessed : 1;
	/** Remove all Abilities that were granted by us on unpossess */
	UPROPERTY(EditAnywhere, Category = "AbilitySystemSetup|Config")
		uint8 bRemoveAbilitiesOnUnpossessed : 1;
	/**
	 * --- CURRENTLY DOES NOTHING. IMPLEMENT RemoveAllCharacterTags() FOR THIS TO DO SOMETHING ---
	 * Removes all tags relating to this specific character from the PlayerState's ASC
	 * Remove all tags related to the character, that way when we possess a new character,
	 * the old tags don't interfere with the new character.
	 */
	UPROPERTY(EditAnywhere, Category = "AbilitySystemSetup|Config")
		uint8 bRemoveCharacterTagsOnUnpossessed : 1;



	/** Removes all Attribute Sets that we added to the ASC */
	int32 UnregisterOwnedAttributeSets();
	/** Removes all Abilities that we granted to the ASC */
	int32 RemoveOwnedAbilities();
	/** NOT IMPLEMENTED YET! Removes all tags relating to this specific character from the PlayerState's ASC */
	int32 RemoveAllCharacterTags();


private:
	UASSAbilitySystemComponent* PreviousPlayerASC;

	// TODO: This is temporary - in UE5, APawn has its own PreviousController variable that we can use rather than making our own
	UPROPERTY()
		AController* PreviousController;


	/** Initialize Attribute values using the DefaultAttributeValuesEffect */
	void InitializeAttributes();
	/** Apply all Effects listed in EffectsToApplyOnStartup */
	void ApplyStartupEffects();

	TArray<FGameplayAbilitySpec> PendingAbilitiesToSync;


	// Internal state bools:

	/** Indicates that we already created Attribute Sets and registered them, initialized the Attributes, and applied the startup Effects */
	uint8 bInitialized : 1;
	/** Shows that we already have input binded with the ability system */
	uint8 bASCInputBound : 1;
	/** Indicates we currently should be dealing with the AIAbilitySystemComponent (this is commonly paired with "!IsPlayerControlled()") */
	uint8 bShouldHandleAIAbilitySystemSetup : 1;
};
