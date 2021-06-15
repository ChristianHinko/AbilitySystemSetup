// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetDataFilter.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemSetup/Private/Utilities/ASSLogCategories.h"

#include "ASSGameplayAbilityTargetDataFilter.generated.h"



/**
 * Base actor target filter.
 * Has ability to only consider Actors that implement the IAbilitySystemInterface.
 * 
 * Note: implementing one of these is actually really confusing to think about sometimes especially because of the bReverseFilter.
 * If you are going to make your own FilterPassesForActor(), the strat is to call the Super last and only have your custom filtering return
 * false (don't return true before calling the Super because then you lose the Super's chance to return false). And before you return false, make
 * sure bReverseFilter is false before doing so.
 */
USTRUCT(BlueprintType)
struct ABILITYSYSTEMSETUP_API FASSGameplayTargetDataFilter : public FGameplayTargetDataFilter
{
	GENERATED_BODY()

	FASSGameplayTargetDataFilter()
	{
		bOnlyAcceptAbilitySystemInterfaces = true;
	}
	virtual ~FASSGameplayTargetDataFilter()
	{
	}


	virtual bool FilterPassesForActor(const AActor* ActorToBeFiltered) const override
	{
		if (bOnlyAcceptAbilitySystemInterfaces)
		{
			//if (ActorToBeFiltered->Implements<UAbilitySystemInterface>() == false)
			//{
			//	return false; // we don't check bReverseFilter here because bOnlyAcceptAbilitySystemInterfaces shouldn't be affected by the bReverseFilter
			//}


			const IAbilitySystemInterface* AbilitySystem = Cast<IAbilitySystemInterface>(ActorToBeFiltered);
			if (!AbilitySystem)
			{
				return false; // we don't check bReverseFilter here because bOnlyAcceptAbilitySystemInterfaces shouldn't be affected by the bReverseFilter
			}

			if (IsValid(AbilitySystem->GetAbilitySystemComponent()) == false)
			{
				UE_LOG(LogGameplayAbilityTargetActorSetup, Warning, TEXT("%s(): %s's GetAbilitySystemComponent() returned NULL. Returned false - we shouldn't let null Ability System Components pass the filter"), *FString(__FUNCTION__), *(ActorToBeFiltered->GetName()));
				return false; // we don't check bReverseFilter here because bOnlyAcceptAbilitySystemInterfaces shouldn't be affected by the bReverseFilter
			}
		}


		return Super::FilterPassesForActor(ActorToBeFiltered);
	}


	/**
	 * Exclusively accept Actors that implement the IAbilitySystemInterface.
	 * Note: this setting is not affected by bReverseFilter
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Filter")
		bool bOnlyAcceptAbilitySystemInterfaces;

};




/////////////////////////////////////////////////////////
/// FGTDF_MultiFilter
/////////////////////////////////////////////////////////


/**
 * Actor target filter that can filter by an array of actor types
 */
USTRUCT(BlueprintType)
struct ABILITYSYSTEMSETUP_API FGTDF_MultiFilter : public FASSGameplayTargetDataFilter
{
	GENERATED_BODY()

	FGTDF_MultiFilter()
	{

	}
	virtual ~FGTDF_MultiFilter()
	{
	}


	virtual bool FilterPassesForActor(const AActor* ActorToBeFiltered) const override
	{
		if (RequiredActorClasses.Num() > 0)
		{
			// Check if this Actor is one of the RequiredActorClasses
			bool bWasRequiredActor = false;
			for (TSubclassOf<AActor> RequiredActorTSub : RequiredActorClasses)
			{
				if (RequiredActorTSub && ActorToBeFiltered->IsA(RequiredActorTSub))
				{
					bWasRequiredActor = true;
				}
			}

			// If this Actor was one of the RequiredActorClasses
			if (bWasRequiredActor)
			{
				if (bReverseFilter == true)
				{
					// Do the opposite and don't let them pass
					return false;
				}
			}
			else
			{
				if (bReverseFilter == false) // only return false if we are a normal filter
				{
					return false;
				}
			}
		}



		// If our custom multi logic didn't do anything, just run the Super
		return Super::FilterPassesForActor(ActorToBeFiltered);
	}

	/** Subclass actors must be one of these to pass the filter. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Filter")
		TArray<TSubclassOf<AActor>> RequiredActorClasses;

};
