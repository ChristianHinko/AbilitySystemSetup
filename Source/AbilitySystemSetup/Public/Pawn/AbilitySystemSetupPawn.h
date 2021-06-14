// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemSetupComponent/AbilitySystemSetupInterface.h"
#include "AbilitySystem/ASSAbilitySystemComponent.h"

#include "AbilitySystemSetupPawn.generated.h"


class UASSAbilitySystemComponent;
class UAbilitySystemSetupComponent;
class AAbilitySystemPlayerState;



UCLASS()
class ABILITYSYSTEMSETUP_API AAbilitySystemSetupPawn : public APawn, public IAbilitySystemInterface, public IAbilitySystemSetupInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, Category = "AbilitySystem")
		UAbilitySystemSetupComponent* AbilitySystemSetup;

public:
	AAbilitySystemSetupPawn(const FObjectInitializer& ObjectInitializer);


	/** Inherited via IAbilitySystemInterface. Returns the ASC (either the AI one or the PS one) */
	UASSAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UAbilitySystemSetupComponent* GetAbilitySystemSetup() const override { return AbilitySystemSetup; }

protected:
	UPROPERTY()
		AAbilitySystemPlayerState* AbilitySystemPlayerState;

	//BEGIN APawn Interface
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void OnRep_Controller() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void UnPossessed() override;
	//END APawn Interface


	//BEGIN IAbilitySystemSetupInterface Interface
	virtual void CreateAttributeSets() override;
	virtual void RegisterAttributeSets() override;
	virtual void GrantStartingAbilities() override;
	//END IAbilitySystemSetupInterface Interface


#pragma region Input Events
	//Actions
	virtual void OnConfirmTargetPressed();
	virtual void OnConfirmTargetReleased();

	virtual void OnCancelTargetPressed();
	virtual void OnCancelTargetReleased();

#pragma endregion

private:
	// Only one of these ASC will be active at a time:

	/**
	 * Points to the PlayerState's ASC
	 */
	UPROPERTY(/*Replicated*/)	// Replicated can be helpful for debugging issues
		UASSAbilitySystemComponent* PlayerAbilitySystemComponent;
	/**
	 * This is used if an AIController is possessing. However, it is also used as a placeholder ASC for before the Player possesses this Pawn (so we can give Abilities and stuff).
	 * These abilities will be transfered from this ASC to the Player's (this allows us to give Abilities as soon as possible). It is also used for creating Attribute Sets early on.
	 */
	UPROPERTY()
		UASSAbilitySystemComponent* AIAbilitySystemComponent;

};
