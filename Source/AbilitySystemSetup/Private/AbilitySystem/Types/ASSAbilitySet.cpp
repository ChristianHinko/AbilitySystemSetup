
// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Types/ASSAbilitySet.h"

#include "GCUtils_Log.h"
#include "AbilitySystemComponent.h"

DEFINE_LOG_CATEGORY(LogASSAbilitySet)

///////////////////////////////////////
/// FASSAbilitySetGrantedHandles
///////////////////////////////////////

void FASSAbilitySetGrantedHandles::RemoveFromAbilitySystemComponent()
{
    UAbilitySystemComponent* asc = AbilitySystemComponent.Get();
    if (!asc)
    {
        GC_LOG_STR_NO_CONTEXT(LogASSAbilitySet, Log, TEXT("Ability system component no longer valid. No point in removing anything since it was probably destroyed already, so we'll just stop tracking them."));
        Clear();
        return;
    }

    if (!ensureAlways(asc->IsOwnerActorAuthoritative()))
    {
        GC_LOG_FMT_NO_CONTEXT(LogASSAbilitySet, Error, TEXT("Tried to remove granted sets from [%s] but we weren't the server. Not doing anything."), *GetNameSafe(asc));
        return;
    }

    GC_LOG_STR_NO_CONTEXT(LogASSAbilitySet, Log, GCUtils::Materialize(TStringBuilder<128>()) << TEXT("Removing granted sets from [") << GCUtils::String::GetUObjectNameSafe(asc) << TEXT("]."));

    // Clear Abilities.
    for (const FGameplayAbilitySpecHandle& specHandle : AbilitySpecHandles)
    {
        if (specHandle.IsValid())
        {
            asc->ClearAbility(specHandle);
        }
    }

    // Remove Effects.
    for (const FActiveGameplayEffectHandle& activeHandle : ActiveEffectHandles)
    {
        if (activeHandle.IsValid())
        {
            asc->RemoveActiveGameplayEffect(activeHandle);
        }
    }

    // Remove Attribute Sets.
    for (UAttributeSet* attributeSet : GrantedAttributeSets)
    {
        asc->RemoveSpawnedAttribute(attributeSet);
    }

    asc->ForceReplication();

    Clear();
}

void FASSAbilitySetGrantedHandles::Clear()
{
    // Empty everything.
    AbilitySystemComponent = nullptr;
    AbilitySpecHandles.Reset();
    ActiveEffectHandles.Reset();
    GrantedAttributeSets.Reset();
}






///////////////////////////////////////
/// UASSAbilitySet
///////////////////////////////////////

void UASSAbilitySet::GrantToAbilitySystemComponent(UAbilitySystemComponent& InASC, UObject& InSourceObject, FASSAbilitySetGrantedHandles& OutGrantedHandles) const
{
    if (!InASC.GetOwnerActor())
    {
        GC_LOG_FMT_UOBJECT(&InASC, LogASSAbilitySet, Error, TEXT("Tried to grant to [%s] but its owning actor wasn't valid. Returning and doing nothing."), *InASC.GetName());
        return;
    }

    if (!ensureAlways(InASC.IsOwnerActorAuthoritative()))
    {
        GC_LOG_FMT_UOBJECT(&InASC, LogASSAbilitySet, Error, TEXT("Tried to grant to [%s] but we weren't the server. Returning and doing nothing."), *InASC.GetName());
        return;
    }


    // Inject the ASC.
    OutGrantedHandles.AbilitySystemComponent = &InASC;

    // Grant our Attribute Sets.
    uint16 attributeSetsGranted = 0;
    for (const TSubclassOf<UAttributeSet> attributeSetClass : GrantedAttributeSets)
    {
        if (!attributeSetClass)
        {
            continue;
        }

        UAttributeSet* newAttributeSet = NewObject<UAttributeSet>(InASC.GetOwnerActor(), attributeSetClass);
        InASC.AddAttributeSetSubobject(newAttributeSet);

        OutGrantedHandles.GrantedAttributeSets.Add(newAttributeSet);

        GC_LOG_FMT_UOBJECT(&InASC, LogASSAbilitySet, Log, TEXT("[%s] granted attrubute set [%s]."), *GetName(), *GetNameSafe(newAttributeSet));

        ++attributeSetsGranted;
    }

    GC_LOG_FMT_UOBJECT(&InASC, LogASSAbilitySet, Log, TEXT("[%s] granted a total of [%i] attrubute sets."), *GetName(), attributeSetsGranted);

    InASC.ForceReplication();

    // Grant our Gameplay Effects.
    uint16 effectsGranted = 0;
    for (const TSubclassOf<UGameplayEffect> effectClass : GrantedEffects)
    {
        if (!effectClass)
        {
            continue;
        }

        FGameplayEffectContextHandle contextHandle = InASC.MakeEffectContext();
        contextHandle.AddSourceObject(&InSourceObject);
        const FActiveGameplayEffectHandle activeHandle = InASC.ApplyGameplayEffectToSelf(effectClass.GetDefaultObject(), /*, GetLevel()*/1, contextHandle);

        OutGrantedHandles.ActiveEffectHandles.Add(activeHandle);

        GC_LOG_FMT_UOBJECT(&InASC, LogASSAbilitySet, Log, TEXT("[%s] granted effect [%s]."), *GetName(), *GetNameSafe(effectClass.GetDefaultObject()));

        ++effectsGranted;
    }

    GC_LOG_FMT_UOBJECT(&InASC, LogASSAbilitySet, Log, TEXT("[%s] granted a total of [%i] effects."), *GetName(), effectsGranted);

    // Grant our Gameplay Abilities.
    uint16 abilitiesGranted = 0;
    for (const TSubclassOf<UGameplayAbility> abilityClass : GrantedAbilities)
    {
        if (!abilityClass)
        {
            continue;
        }

        const FGameplayAbilitySpecHandle specHandle = InASC.GiveAbility(FGameplayAbilitySpec(abilityClass, /*, GetLevel()*/1, INDEX_NONE, &InSourceObject));

        OutGrantedHandles.AbilitySpecHandles.Add(specHandle);

        GC_LOG_FMT_UOBJECT(&InASC, LogASSAbilitySet, Log, TEXT("[%s] granted ability [%s]."), *GetName(), *GetNameSafe(abilityClass.GetDefaultObject()));

        ++abilitiesGranted;
    }

    GC_LOG_FMT_UOBJECT(&InASC, LogASSAbilitySet, Log, TEXT("[%s] granted a total of [%i] abilities."), *GetName(), abilitiesGranted);
}
