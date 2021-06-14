// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"

#include "ASSAbilitySystemComponent.generated.h"



class UASSGameplayAbility;


#if 0
/**
 *														--------------Example InputIDEnum--------------
 *	Enum that makes GAS aware of which abilities are binded to which input in your project settings. Recommended spot to declare would be in child ASC.
 *	"Unset" at 0 and "NoInput" at 1 is MANDITORY in your enum so the ability can enforce a good practice. We have a system so that if an ability's inputId is unset, it will throw an exception,
 *	forcing you to give each ability an input id. This is good practice since using ability input ids integrates stuff more into GAS.
 *
 *	Do not forget to update your enum whenever you modify the inputs in your project settings. They must match exactly.
 */
UENUM()
enum/* class*/ EAbilityInputID/* : uint8*/ // i would want to be able to use enum class here. but there is no implicit int conversion with them. and our base ASSGameplayAbility has to store AbilityInputID as an integer
{
	// 0
	// This means the ability implementor forgot to set an AbilityInputId in their ability's constructor ("Unset" is every ability's default value)
	Unset,
	// 1
	// This means the ability is triggered without input (probably gameplay code)
	NoInput,

	// 2
	Run,
	// 3
	Jump,
	// 4
	Crouch,
	// 5
	Interact,
	// 6
	PrimaryFire,
	// 7
	SecondaryFire,
	// 8
	Reload,
	// 9
	Item0,
	// 10
	Item1,
	// 11
	Item2,
	// 12
	Item3,
	// 13
	Item4,
	// 14
	SwitchWeapon,
	// 15
	NextItem,
	// 16
	PreviousItem,
	// 17
	DropItem,
	// 18
	Pause,
	// 19
	ScoreSheet,


	// MAX
	MAX					UMETA(Hidden) // show a Max value since this isn't an enum class
};
#endif

/**
 * Base Ability System Component
 */
UCLASS()
class ABILITYSYSTEMSETUP_API UASSAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UASSAbilitySystemComponent();


	virtual bool ShouldDoServerAbilityRPCBatch() const override { return true; }

	FGameplayAbilitySpecHandle GrantAbility(TSubclassOf<UASSGameplayAbility> ASSNewAbility, UObject* InSourceObject, int32 level = 1);
	FGameplayAbilitySpecHandle GrantAbility(TSubclassOf<UGameplayAbility> NewAbility, UObject* InSourceObject, int32 level = 1);
	void GrantAbilities(TArray<FGameplayAbilitySpec> Abilities);

	/** Gives abilities that an other given ASC has */
	void RecieveAbilitiesFrom(UAbilitySystemComponent* From);

	virtual void TargetConfirmByAbility(UGameplayAbility* AbilityToConfirmTargetOn);
	virtual void TargetCancelByAbility(UGameplayAbility* AbilityToCancelTargetOn);

	/** This override adds a check to see if we should confirm/cancel target actors associated with the InputID on keypress */
	virtual void BindAbilityActivationToInputComponent(UInputComponent* InputComponent, FGameplayAbilityInputBinds BindInfo) override;

	/** Version of function in AbilitySystemGlobals that returns correct type */
	static UASSAbilitySystemComponent* GetAbilitySystemComponentFromActor(const AActor* Actor, bool LookForComponent = false);

	/** Returns a list of currently active ability instances that match the tags */
	void GetActiveAbilitiesWithTags(const FGameplayTagContainer& GameplayTagContainer, TArray<UGameplayAbility*>& ActiveAbilities);

	/** Returns an ability spec handle from a class. If modifying call MarkAbilitySpecDirty */
	FGameplayAbilitySpecHandle FindAbilitySpecHandleFromClass(TSubclassOf<UGameplayAbility> AbilityClass, UObject* OptionalSourceObject = nullptr);

	// Gameplay cue helpers for running them locally
	void ExecuteGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);
	void AddGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);
	void RemoveGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);

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