// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/ASSAbilitySystemComponent.h"

#include "AbilitySystemActor.generated.h"



//////////////////////////////// THIS CLASS MAY BE VERY OUT OF DATE!!! WE UPDATE AbilitySystemCharacter BUT HAVEN'T UPDATED THIS AFTER //////////////
/**
 * A Base GAS Actor class
 * 
 * NOT TESTED YET!!!!!!!!!!!!!!!!!!
 * Code in this class is not just copied from the AbilitySystemCharacter class. The AbilitySystemActor class has a specific implementation that differs a little from the AbilitySystemCharacter class.
 *
 * Some tips:
 *		1) Levels
 *			- Always passed in as 1 because this template does not come with a level system. GetLevel() is what you should make if your wanting a level system. This is commented out at all places where the level is passed in. since no player will ever possess this actor and the ASC always lives on here, you can put the GetLevel() function in this class.
 *		2) Abilities
 *			- If you have a starting ability just override GrantStartingAbilities() and call Super in the beginning, then add your own logic
 *		3) Attribute Sets
 *			- Subclasses should override RegisterAttributeSets() and call the Super at the beginning if they would like to add more attribute sets than the default pawn one
 *		4) Gameplay Effects
 *			- Every actor should initialize their attributes through the DefaultAttributeValuesEffect GameplayEffectTSub in BP.
 *			- Any gameplay effects that this actor should have on spawn should be filled in in the EffectsToApplyOnStartup TArray in BP. (ie. GE_HealthRegen or GE_StaminaRegen)
 *		5) Gmeplay Tags
 *			- Nothing special here
 *		6) Respawning
 *			- You can have a respawning system for this actor without destroying the actor or its ASC. Just reset its ASC using the custom function we wrote in the base ASC class FullReset() and also set bAttributesAndStartupEffectsInitialized to false so you can initialize again using SetupWithAbilitySystem(). This however assumes that all that needs to be reset for the actor is the stuff belonging to the ASC (ie. its AttributeSets). If more needs to be reset you can just add it
 */
UCLASS()
class ABILITYSYSTEMSETUP_API AAbilitySystemActor : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAbilitySystemActor();

	//BEGIN IAbilitySystemInterface
	UASSAbilitySystemComponent* GetAbilitySystemComponent() const override { return ActorAbilitySystemComponent; }
	//END IAbilitySystemInterface

protected:
	UPROPERTY(/*Replicated*/)	// Replicated can be helpful for debugging issues
		UASSAbilitySystemComponent* ActorAbilitySystemComponent;

	virtual void PostInitializeComponents() override;

#pragma region Effects
	/** Default attributes values for a Actor on spawn. This should be an instant GE with the Modifier Op set to Override so you can choose what the Actor's starting attribute values will be on spawn */
	UPROPERTY(EditAnywhere, Category = "AbilitySystemSetup|Effects")
		TSubclassOf<UGameplayEffect> DefaultAttributeValuesEffect;
	/** These effects are only applied one time on startup (ie. GE_HealthRegen, GE_StaminaRegen) */
	UPROPERTY(EditAnywhere, Category = "AbilitySystemSetup|Effects")
		TArray<TSubclassOf<UGameplayEffect>> EffectsToApplyOnStartup;
#pragma endregion


	/** Decide which replication mode you want for this Actor's AbilitySystemComponent. */
	UPROPERTY(EditAnywhere, Category = "AbilitySystemSetup|Config")
		EGameplayEffectReplicationMode ActorAbilitySystemComponentReplicationMode;

	/** The function that does all the necissary setup to be used with GAS. Calls most functions in this base Actor class */
	void SetupWithAbilitySystem();
	/** Called on the server and client. Override this to create new AttributeSets using NewObject(). This is if you have more than the default one. (call super in the beginning, then add your own logic) */
	virtual void CreateAttributeSets();
	/** Called on the server and client. Override this to register your created AttributeSets (from CreateAttributeSets()) with the ASC. This is if you have more than the default one. (call super in the beginning, then add your own logic) */
	virtual void RegisterAttributeSets();
	/** Initialize the Actor's attributes using the DefaultAttributeValuesEffect */
	void InitializeAttributes();
	/** Will apply all effects in EffectsToApplyOnStartup. */
	void ApplyStartupEffects();
	/** Called only on server. This is the earliest place you can grant an ability. (If overriding, return false if the Super returnes false, then add your own logic) */
	virtual bool GrantStartingAbilities();

	/**
	 * Set to true if you want this actor to reference another actor's ability system component rather than creating its own. Ex: A weapon with its ActorAbilitySystemComponent set to the player state's ASC.
	 * Note: This will require some custom work in the subclass. Ex: in our weapon class, in OnRep_Owner() we set the weapon's ActorAbilitySystemComponent to the owner's (casted to an AbilitySystemCharacter) ASC.
	 * Note: Make sure you set ActorAbilitySystemComponent before SetupWithAbilitySystem() is called. To do this, you will most likely have to override PostInitializeComponents() and call AAbilitySystemActor::Super::PostInitializeComponents(). This
	 * skips the SetupWithAbilitySystem() and it is your job to call it elsewhere (some time after you set ActorAbilitySystemComponent).
	 */
	uint8 bWithoutAbilitySystemComponentSubobject : 1;

private:
	/** Indicates that we already created attribute sets and registered them, Initialized the attributes, and applied the startup effects */
	uint8 bAbilitySystemSetupInitialized : 1;

};
