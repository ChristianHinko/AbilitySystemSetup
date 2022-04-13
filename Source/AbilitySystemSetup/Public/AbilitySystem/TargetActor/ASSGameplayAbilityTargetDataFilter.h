// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetDataFilter.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemSetup/Private/Utilities/ASSLogCategories.h"

#include "ASSGameplayAbilityTargetDataFilter.generated.h"



/**
 * Base Target Data Filter.
 * Has ability to only consider Actors that implement the IAbilitySystemInterface.
 * 
 * NOTE: implementing one of these is actually really confusing to think about sometimes especially because of the bReverseFilter (ACTUALLY just ignore bReverseFilter, just let that be a thing that they do in the Super).
 * If you are going to make your own FilterPassesForActor(), the strat is to call the Super last and only have your custom filtering return
 * false (don't return true before calling the Super because then you lose the Super's chance to return false).
 * 
 * When overriding FilterPassesForActor(), always call the Super at the end and only return false before doing so.
 * Override ASSFilterPassesForActor() instead because FASSGameplayTargetDataFilter marks FilterPassesForActor() as final. The reason for this is 
 * we always need this implementation to run before any child implementations.
 * 
 * NOTE: Yeah just ignore bReverseFilter it just ruins things. We tried using it and it makes things really annoying.
 * If you really want something like this idk why they don't just ``!`` the return value.
 */
USTRUCT(BlueprintType)
struct ABILITYSYSTEMSETUP_API FASSGameplayTargetDataFilter : public FGameplayTargetDataFilter
{
	GENERATED_BODY()

	FASSGameplayTargetDataFilter();


	virtual bool FilterPassesForActor(const AActor* ActorToBeFiltered) const override final;
	/**
	 * Override ASSFilterPassesForActor() instead because FASSGameplayTargetDataFilter marks FilterPassesForActor() as final. The reason for this is
	 * we always need this implementation to run before any child implementations.
	 */
	virtual bool ASSFilterPassesForActor(const AActor* ActorToBeFiltered) const;

	/**
	 * Exclusively accept Actors that implement the IAbilitySystemInterface.
	 * NOTE: This setting is not affected by bReverseFilter
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Filter")
		bool bOnlyAcceptAbilitySystemInterfaces;

};




/////////////////////////////////////////////////////////
/// FGTDF_MultiFilter
/////////////////////////////////////////////////////////


/**
 * Target Data Filter that can filter by an array of Actor types
 */
USTRUCT(BlueprintType)
struct ABILITYSYSTEMSETUP_API FGTDF_MultiFilter : public FASSGameplayTargetDataFilter
{
	GENERATED_BODY()

	FGTDF_MultiFilter();


	virtual bool ASSFilterPassesForActor(const AActor* ActorToBeFiltered) const override;

	/** Subclass actors must be one of these to pass the filter. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Filter")
		TArray<TSubclassOf<AActor>> RequiredActorClasses;
	/** Subclass actors must NOT be one of these to pass the filter. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Filter")
		TArray<TSubclassOf<AActor>> FilteredActorClasses;

};
