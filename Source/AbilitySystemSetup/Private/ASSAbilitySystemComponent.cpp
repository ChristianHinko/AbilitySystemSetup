// Fill out your copyright notice in the Description page of Project Settings.

#include "ASSAbilitySystemComponent.h"

#include "GCUtils_Log.h"

DEFINE_LOG_CATEGORY(LogASSAbilitySystemComponent)

UASSAbilitySystemComponent::UASSAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    /** The linked Anim Instance that this component will play montages in. Use NAME_None for the main anim instance. (Havn't explored this much yet) */
    AffectedAnimInstanceTag = NAME_None;
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

void UASSAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    if (AbilitySpec.SourceObject.IsValid() == false)
    {
        GC_LOG_STR_UOBJECT(
            this,
            LogASSAbilitySystemComponent,
            Warning,
            TEXT("SourceObject was not valid when Ability was given. Someone must have forgotten to set it when giving the Ability")
        );
        check(0);
    }
#endif // !(UE_BUILD_SHIPPING || UE_BUILD_TEST)

    Super::OnGiveAbility(AbilitySpec);
}

void UASSAbilitySystemComponent::FullReset()
{
    //    Stop ASC from doing things
    DestroyActiveState();


    if (IsOwnerActorAuthoritative())
    {
        //    Ungive abilities. Will remove all abilitity tags/blocked bindings as well
        ClearAllAbilities();

        //    Clear Effects. Will remove all given tags and cues as well
        for (const FActiveGameplayEffect& Effect : &ActiveGameplayEffects)
        {
            RemoveActiveGameplayEffect(Effect.Handle);
        }

        //    Remove Attribute Sets
        RemoveAllSpawnedAttributes();
    }


    //    If cue still exists because it was not from an effect
    RemoveAllGameplayCues();

    //    Now clean up any loose gameplay tags
    ResetTagMap();
    GetMinimalReplicationTags_Mutable().RemoveAllTags();        //    This line may not be necessary

    //    Give clients changes ASAP
    ForceReplication();
}
