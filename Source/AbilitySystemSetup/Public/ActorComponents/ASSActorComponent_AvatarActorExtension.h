// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "ASSActorComponent_AvatarActorExtension.generated.h"

class UAbilitySystemComponent;
class UASSAbilitySet;
struct FASSAbilitySetGrantedHandles;

DECLARE_LOG_CATEGORY_EXTERN(LogASSAvatarActorExtensionComponent, Log, All)

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

    DECLARE_MULTICAST_DELEGATE_OneParam(FAvatarExtensionNativeDelegate, UAbilitySystemComponent& /* inASC */);

public:
    UASSActorComponent_AvatarActorExtension(const FObjectInitializer& inObjectInitializer);

protected:
    //  BEGIN UActorComponent interface
    virtual void OnRegister() override;
    //  END UActorComponent interface

public: // Extension functions for owner to call

    /**
     * @brief Sets the avatar actor with the ASC.
     * @param inASC The ability system component to associate the avatar actor with.
     */
    virtual void InitializeAbilitySystemComponent(UAbilitySystemComponent& inASC);

    /**
     * @brief Clears the Avatar Actor from the ASC.
     */
    virtual void UninitializeAbilitySystemComponent();

private:

    /**
     * @brief Broadcasts event to allow external sources to cleanup any loose gameplay tags they were managing.
     * @param inASC: The ASC to remove the loose tags from.
     */
    void RemoveLooseAvatarRelatedTags(UAbilitySystemComponent& inASC);

public:

    /**
     * @brief Returns whether or not this component has finished setting up the avatar actor with its ability system component.
     */
    FORCEINLINE bool IsInitializedWithASC() const { return bInitialized; }

protected:

    /** The initialized ASC */
    UPROPERTY(Transient)
    TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

    /** Ability sets to grant to this avatar's ability system. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilitySystemSetup | AbilitySets")
    TArray<TSubclassOf<UASSAbilitySet>> AbilitySets;

private:

    /** Abilities, Active Effects, and Attribute Sets to keep track of so we can clean them up from our ASC on UnPossess */
    TArray<FASSAbilitySetGrantedHandles> GrantedHandles;

    /** Indicates that the list of AbilitySets has been granted */
    uint8 bGrantedAbilitySets : 1 = false;

    /** Indicates that we are initialized with an Ability System Component */
    uint8 bInitialized : 1 = false;

public:

    /** Broadcasted when the Ability System is set up and ready to go */
    FAvatarExtensionNativeDelegate OnInitializeAbilitySystemComponentDelegate;

    /** Server and client event for removing all Loose AvatarActor-related Tags. */
    FAvatarExtensionNativeDelegate RemoveLooseAvatarRelatedTagsDelegate;
};
