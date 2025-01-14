// Fill out your copyright notice in the Description page of Project Settings.

#include "ASSAbilitySystemComponent.h"

#include "GCUtils_Log.h"
#include "ASSNativeGameplayTags.h"
#include "ASSUtils.h"
#include "GCUtils.h"
#include "ISNativeGameplayTags.h"

DEFINE_LOG_CATEGORY_STATIC(LogASSAbilitySystemComponent, Log, All);

UASSAbilitySystemComponent::UASSAbilitySystemComponent(const FObjectInitializer& objectInitializer)
    : Super(objectInitializer)
{
}

void UASSAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& abilitySpec)
{
#if !NO_LOGGING || DO_ENSURE
    {
        const bool hasValidSourceObject = abilitySpec.SourceObject.IsValid();

        GC_LOG_STR_UOBJECT(
            this,
            LogASSAbilitySystemComponent,
            Warning,
            GCUtils::Materialize(TStringBuilder<512>())
                << TEXT("Source object is null/garbage for the given spec.")
                TEXT(" ")
                TEXT("Ability CDO: '") << GetFNameSafe(abilitySpec.Ability) << TEXT("'.")
            );

        ensure(hasValidSourceObject);
    }
#endif // #if !NO_LOGGING || DO_ENSURE

    Super::OnGiveAbility(abilitySpec);

    const UGameplayAbility* abilityCDO = abilitySpec.Ability;
    if (!ensure(abilityCDO))
    {
        GC_LOG_STR_UOBJECT(
            this,
            LogASSAbilitySystemComponent,
            Error,
            GCUtils::Materialize(TStringBuilder<512>())
                << TEXT("Ability CDO is null for the given spec.")
            );
        return;
    }

#if !NO_LOGGING || DO_ENSURE
    {
        const bool hasAnyAssetTag = abilityCDO->GetAssetTags().IsEmpty() == false;
        if (!ensure(hasAnyAssetTag))
        {
            GC_LOG_STR_UOBJECT(
                this,
                LogASSAbilitySystemComponent,
                Warning,
                GCUtils::Materialize(TStringBuilder<512>())
                    << TEXT("Ability implementor forgot to assign an ability tag to this ability. We try to enforce activating abilities by tag for organization reasons.")
                    TEXT(" ")
                    TEXT("Ability CDO: '") << abilityCDO->GetFName() << TEXT("'.")
                );
        }

        const bool hasInputActionTag = abilityCDO->GetAssetTags().HasTag(ISNativeGameplayTags::InputAction);
        if (!ensure(hasInputActionTag))
        {
            GC_LOG_STR_UOBJECT(
                this,
                LogASSAbilitySystemComponent,
                Warning,
                GCUtils::Materialize(TStringBuilder<512>())
                    << TEXT("Ability implementor forgot to assign an input action tag to this ability.")
                    TEXT(" ")
                    TEXT("We enforce this so that a given an input action can identify any abilities it activates.")
                    TEXT(" ")
                    TEXT("If the ability isn't intended to be activated by input you can suppress this with the '") << ISNativeGameplayTags::InputAction_None.GetTag().GetTagName() << TEXT("' tag.")
                    TEXT(" ")
                    TEXT("Ability CDO: '") << abilityCDO->GetFName() << TEXT("'.")
                );
        }
    }
#endif // #if !NO_LOGGING || DO_ENSURE

    const bool isPassiveAbility = abilityCDO->GetAssetTags().HasTag(ASSNativeGameplayTags::Ability_Type_Passive);
    if (isPassiveAbility)
    {
        // We activate passive abilities when given.
        ASSUtils::TryActivateAbilityPassive(*this, abilitySpec);
    }
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
