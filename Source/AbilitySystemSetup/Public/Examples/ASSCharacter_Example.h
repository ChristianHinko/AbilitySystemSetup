// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/ASSAbilitySystemComponent.h"

#include "ASSCharacter_Example.generated.h"


class UASSActorComponent_AbilitySystemSetup;


/**
 * Example implementation of a Character initializing with an ASC by using the UASSActor_Example.
 * Feel free to subclass if lazy.
 */
UCLASS()
class ABILITYSYSTEMSETUP_API AASSCharacter_Example : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, Category = "AbilitySystem")
		TObjectPtr<UASSActorComponent_AbilitySystemSetup> AbilitySystemSetupComponent;

public:
	AASSCharacter_Example(const FObjectInitializer& ObjectInitializer);

	UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	virtual void PreInitializeComponents();

	//BEGIN APawn Interface
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void OnRep_PlayerState() override;
	virtual void OnRep_Controller() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	//END APawn Interface

	//BEGIN AbilitySystemSetupDelegate
	virtual void OnRemoveLooseAvatarRelatedTags(UAbilitySystemComponent* ASC);
	//END AbilitySystemSetupDelegate


	//BEGIN Input actions
	virtual void OnConfirmTargetPressed();
	virtual void OnConfirmTargetReleased();

	virtual void OnCancelTargetPressed();
	virtual void OnCancelTargetReleased();
	//END Input actions

};
