// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilitySystem/Types/ASSAbilitySet.h"

#include "ASSActorComponent_AvatarActorExtension.generated.h"


class UAbilitySystemComponent;



DECLARE_MULTICAST_DELEGATE_OneParam(FAvatarExtensionDelegate, UAbilitySystemComponent* const);


/**
 * Provides common GAS initialization/uninitialization logic with Ability Sets granted while initialized.
 * This component is to be used by avatar actors only.
 * 
 * For initialization, it sets us up as the AvatarActor for the ASC and grants Ability Sets (allowing you to choose 
 * starting Abilities, Effects, and Attribute Sets in BP). For uninitialization it ungrants the granted Ability Sets, 
 * gives external sources an opportunity to remove Loose Gameplay Tags (this is the only manual cleanup),
 * and disassociates us from the ASC.
 * 
 * This component does not automate anything. You have to manually call on provided functions for anything to happen......
 * 
 * 
 * ----------------------------------
 *                Setup
 * ----------------------------------
 * 
 * Recomended callsites for initialization and uninitialization:
 *            PostInitializeComponents()
 *                - Call InitializeAbilitySystemComponent() after the Super call
 *            EndPlay()
 *                - Call UninitializeAbilitySystemComponent() before the Super call
 */
UCLASS(ClassGroup=(AbilitySystemSetup), meta=(BlueprintSpawnableComponent))
class ABILITYSYSTEMSETUP_API UASSActorComponent_AvatarActorExtension : public UActorComponent
{
    GENERATED_BODY()

public:
    UASSActorComponent_AvatarActorExtension(const FObjectInitializer& ObjectInitializer);


    /** Ability sets to grant to this avatar's ability system. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySets")
        TArray<TSubclassOf<UASSAbilitySet>> AbilitySets;

    /** Sets the Avatar Actor with the ASC */
    virtual void InitializeAbilitySystemComponent(UAbilitySystemComponent* InASC);
    /** Clears the Avatar Actor from the ASC */
    virtual void UninitializeAbilitySystemComponent();

    /** Broadcasted when the Ability System is set up and ready to go */
    FAvatarExtensionDelegate OnInitializeAbilitySystemComponentDelegate;
    /** Server and client event for removing all Loose AvatarActor-related Tags. */
    FAvatarExtensionDelegate RemoveLooseAvatarRelatedTagsDelegate;

    bool IsInitializedWithASC() const { return bInitialized; }

protected:
    //  BEGIN UActorComponent interface
    virtual void OnRegister() override;
    //  END UActorComponent interface

    /** The initialized ASC */
    UPROPERTY(Transient)
        TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

private:
    /** Broadcasts event to allow external sources to cleanup any Loose Gameplay Tags they were managing */
    void RemoveLooseAvatarRelatedTags();

    /** Abilities, Active Effects, and Attribute Sets to keep track of so we can clean them up from our ASC on UnPossess */
    TArray<FASSAbilitySetGrantedHandles> GrantedHandles;
    /** Indicates that the list of AbilitySets has been granted */
    uint8 bGrantedAbilitySets : 1;
    /** Indicates that we are initialized with an Ability System Component */
    uint8 bInitialized : 1;
};
