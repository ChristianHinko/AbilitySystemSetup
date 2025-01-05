// Fill out your copyright notice in the Description page of Project Settings.

#include "ASSAbilitySystemComponent.h"

#include "GCUtils_Log.h"

DEFINE_LOG_CATEGORY(LogASSAbilitySystemComponent)

UASSAbilitySystemComponent::UASSAbilitySystemComponent(const FObjectInitializer& objectInitializer)
    : Super(objectInitializer)
{

}

#if DO_CHECK
void UASSAbilitySystemComponent::BindToInputComponent(UInputComponent* inputComponent)
{
    checkNoEntry();
}

void UASSAbilitySystemComponent::BindAbilityActivationToInputComponent(UInputComponent* inputComponent, FGameplayAbilityInputBinds bindInfo)
{
    checkNoEntry();
}

void UASSAbilitySystemComponent::AbilityLocalInputPressed(int32 inputID)
{
    checkNoEntry();
}

void UASSAbilitySystemComponent::AbilityLocalInputReleased(int32 inputID)
{
    checkNoEntry();
}
#endif // #if DO_CHECK

#if DO_CHECK || !NO_LOGGING
void UASSAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& abilitySpec)
{
    const bool hasValidSourceObj = abilitySpec.SourceObject.IsValid();
    GC_CLOG_STR_UOBJECT(
        this,
        !hasValidSourceObj,
        LogASSAbilitySystemComponent,
        Warning,
        TEXT("`SourceObject` was not valid when ability was given. Someone must have forgotten to set it when giving the ability.")
    );
    check(hasValidSourceObj);

    Super::OnGiveAbility(abilitySpec);
}
#endif // #if DO_CHECK || !NO_LOGGING

void UASSAbilitySystemComponent::FullReset()
{
    // Stop ASC from doing things.
    DestroyActiveState();

    if (IsOwnerActorAuthoritative())
    {
        // Ungive abilities. Will remove all abilitity tags/blocked bindings as well.
        ClearAllAbilities();

        // Clear effects. Will remove all given tags and cues as well.
        for (const FActiveGameplayEffect& effect : &ActiveGameplayEffects)
        {
            RemoveActiveGameplayEffect(effect.Handle);
        }

        // Remove attribute sets
        RemoveAllSpawnedAttributes();
    }


    // If cue still exists because it was not from an effect.
    RemoveAllGameplayCues();

    // Now clean up any loose gameplay tags.
    ResetTagMap();
    GetMinimalReplicationTags_Mutable().RemoveAllTags(); // This line may not be necessary.

    // Give clients changes ASAP.
    ForceReplication();
}
