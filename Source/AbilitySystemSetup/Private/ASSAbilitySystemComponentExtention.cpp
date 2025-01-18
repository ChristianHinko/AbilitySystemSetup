// Fill out your copyright notice in the Description page of Project Settings.

#include "ASSAbilitySystemComponentExtention.h"

#include "ASSUtils.h"
#include "GCUtils_Log.h"
#include "ISNativeGameplayTags.h"

DEFINE_LOG_CATEGORY_STATIC(LogASSAbilitySystemComponentExtention, Log, All);

#if DO_CHECK
void FASSAbilitySystemComponentExtention::BindToInputComponent_ReplaceSuper(UInputComponent* inputComponent)
{
    checkNoEntry();
}

void FASSAbilitySystemComponentExtention::BindAbilityActivationToInputComponent_ReplaceSuper(UInputComponent* inputComponent, FGameplayAbilityInputBinds bindInfo)
{
    checkNoEntry();
}

void FASSAbilitySystemComponentExtention::AbilityLocalInputPressed_ReplaceSuper(int32 inputID)
{
    checkNoEntry();
}

void FASSAbilitySystemComponentExtention::AbilityLocalInputReleased_ReplaceSuper(int32 inputID)
{
    checkNoEntry();
}
#endif // #if DO_CHECK

#if DO_CHECK || !NO_LOGGING
void FASSAbilitySystemComponentExtention::OnGiveAbility_PostSuper(UAbilitySystemComponent& asc, FGameplayAbilitySpec& abilitySpec)
{
    const UGameplayAbility* abilityCDO = abilitySpec.Ability;
    if (!ensure(abilityCDO))
    {
        GC_LOG_STR_UOBJECT(
            (&asc),
            LogASSAbilitySystemComponentExtention,
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
                (&asc),
                LogASSAbilitySystemComponentExtention,
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
                (&asc),
                LogASSAbilitySystemComponentExtention,
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
        ASSUtils::TryActivateAbilityPassive(asc, abilitySpec);
    }
}
#endif // #if DO_CHECK || !NO_LOGGING
