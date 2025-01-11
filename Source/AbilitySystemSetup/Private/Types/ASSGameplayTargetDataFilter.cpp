// Fill out your copyright notice in the Description page of Project Settings.

#include "Types/ASSGameplayTargetDataFilter.h"

#include "GCUtils_Log.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogASSGameplayTargetDataFilter, Log, All);

FASSGameplayTargetDataFilter::FASSGameplayTargetDataFilter()
{
    bOnlyAcceptAbilitySystemInterfaces = true;
}

bool FASSGameplayTargetDataFilter::FilterPassesForActor(const AActor* ActorToBeFiltered) const // TODO: require the AActor param.
{
    if (bOnlyAcceptAbilitySystemInterfaces)
    {
        //if (ActorToBeFiltered->Implements<UAbilitySystemInterface>() == false)
        //{
        //    return false; // we don't check bReverseFilter here because bOnlyAcceptAbilitySystemInterfaces shouldn't be affected by the bReverseFilter
        //}


        const IAbilitySystemInterface* AbilitySystem = Cast<IAbilitySystemInterface>(ActorToBeFiltered);
        if (!AbilitySystem)
        {
            return false; // we don't check bReverseFilter here because bOnlyAcceptAbilitySystemInterfaces shouldn't be affected by the bReverseFilter
        }

        if (IsValid(AbilitySystem->GetAbilitySystemComponent()) == false)
        {
            GC_LOG_STR_UOBJECT(
                ActorToBeFiltered,
                LogASSGameplayTargetDataFilter,
                Warning,
                GCUtils::Materialize(TStringBuilder<512>()) << ActorToBeFiltered->GetName() << TEXT("'s GetAbilitySystemComponent() returned NULL. Returned false - we shouldn't let null Ability System Components pass the filter")
            );
            return false; // we don't check bReverseFilter here because bOnlyAcceptAbilitySystemInterfaces shouldn't be affected by the bReverseFilter
        }
    }


    return ASSFilterPassesForActor(ActorToBeFiltered);
}

bool FASSGameplayTargetDataFilter::ASSFilterPassesForActor(const AActor* ActorToBeFiltered) const
{
    return Super::FilterPassesForActor(ActorToBeFiltered);
}

/////////////////////////////////////////////////////////
/// FASSGameplayTargetDataFilter_MultiFilter
/////////////////////////////////////////////////////////

FASSGameplayTargetDataFilter_MultiFilter::FASSGameplayTargetDataFilter_MultiFilter()
{

}

bool FASSGameplayTargetDataFilter_MultiFilter::ASSFilterPassesForActor(const AActor* ActorToBeFiltered) const
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

        // If this Actor wasn't one of the RequiredActorClasses
        if (!bWasRequiredActor)
        {
            return false;
        }
    }
    if (FilteredActorClasses.Num() > 0)
    {
        // Check if this Actor is one of the FilteredActorClasses
        bool bWasFilteredActor = false;
        for (TSubclassOf<AActor> FilteredActorTSub : FilteredActorClasses)
        {
            if (FilteredActorTSub && ActorToBeFiltered->IsA(FilteredActorTSub))
            {
                bWasFilteredActor = true;
            }
        }

        // If this Actor was one of the FilteredActorClasses
        if (bWasFilteredActor)
        {
            return false;
        }
    }



    // If our custom multi logic didn't do anything, just run the Super
    return Super::ASSFilterPassesForActor(ActorToBeFiltered);
}
