// Fill out your copyright notice in the Description page of Project Settings.

#include "ActorComponents/ASSActorComponent_AvatarActorExtension.h"

#include "Types/ASSAbilitySet.h"
#include "ASSAbilitySystemBlueprintLibrary.h"
#include "GCUtils_Log.h"
#if !NO_LOGGING || DO_CHECK
#include "ActorComponents/ASSActorComponent_PawnAvatarActorExtension.h"
#endif // #if !NO_LOGGING || DO_CHECK

DEFINE_LOG_CATEGORY(LogASSAvatarActorExtensionComponent)

UASSActorComponent_AvatarActorExtension::UASSActorComponent_AvatarActorExtension(const FObjectInitializer& inObjectInitializer)
    : Super(inObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UASSActorComponent_AvatarActorExtension::OnRegister()
{
    Super::OnRegister();

    check(GetOwner());

#if !NO_LOGGING || DO_CHECK
    if (GetOwner()->IsA<APawn>() && this->IsA<UASSActorComponent_PawnAvatarActorExtension>() == false)
    {
        GC_LOG_STR_UOBJECT(this,
            LogASSAvatarActorExtensionComponent,
            Error,
            GCUtils::Materialize(TStringBuilder<512>()) << TEXT("Incorrect usage of this component. Use the pawn specific version: [") << TNameOf<UASSActorComponent_PawnAvatarActorExtension>::GetName() << TEXT("]. Component owner is a Pawn: [") << GCUtils::String::GetUObjectNameSafe(GetOwner()) << TEXT("].")
            );
        check(0);
    }

    TArray<UActorComponent*> avatarActorExtensionComponents;
    GetOwner()->GetComponents(ThisClass::StaticClass(), avatarActorExtensionComponents);
    if (avatarActorExtensionComponents.Num() > 1)
    {
        GC_LOG_STR_UOBJECT(this,
            LogASSAvatarActorExtensionComponent,
            Error,
            GCUtils::Materialize(TStringBuilder<512>()) << TEXT("No more than one [") << TNameOf<UASSActorComponent_AvatarActorExtension>::GetName() << TEXT("] is allowed. Component owner: [") << GCUtils::String::GetUObjectNameSafe(GetOwner()) << TEXT("].")
            );
        check(0);
    }
#endif // !NO_LOGGING || DO_CHECK
}

void UASSActorComponent_AvatarActorExtension::InitializeAbilitySystemComponent(UAbilitySystemComponent& inASC)
{
    check(GetOwner());

    if (AbilitySystemComponent == &inASC)
    {
        GC_LOG_STR_UOBJECT(this,
            LogASSAvatarActorExtensionComponent,
            Warning,
            TEXT("Called again after already being initialized - no need to proceed. We should probably track down the reason this is being called even after it's already initialized.")
            );
        return;
    }
    // Resolve edge case: You forgot to uninitialize the inASC before initializing a new one.
    if (bInitialized || AbilitySystemComponent.IsValid())
    {
        GC_LOG_STR_UOBJECT(this,
            LogASSAvatarActorExtensionComponent,
            Warning,
            TEXT("Looks like you forgot to uninitialize the ASC before initializing a new one. Maybe you forgot to uninitialize on UnPossessed() - this will probably cause unwanted side-effects such as Gameplay Effects lingering after UnPossessed(). We are uninitializing the ASC for you before initializing the new one BUT you should manually do this instead to prevent lingering stuff.")
            );
        UninitializeAbilitySystemComponent();
    }

    const AActor* currentAvatar = inASC.GetAvatarActor(); // the passed in ASC's old avatar
    AActor* newAvatarToUse = GetOwner();            // new avatar for the passed in ASC
    GC_LOG_STR_UOBJECT(this,
        LogASSAvatarActorExtensionComponent,
        Verbose,
        GCUtils::Materialize(TStringBuilder<512>()) << TEXT("Setting up ASC [") << inASC.GetFName() << TEXT("] on actor [") << GCUtils::String::GetUObjectNameSafe(newAvatarToUse) << TEXT("] with owner [") << GCUtils::String::GetUObjectNameSafe(inASC.GetOwnerActor()) << TEXT("] and avatar actor [") << GCUtils::String::GetUObjectNameSafe(currentAvatar) << TEXT("].")
        );

    // Resolve edge cases: You forgot to uninitialize the ASC before initializing a new one    OR    destruction of previous avatar hasn't been replicated yet (because of lagged client)
    if (currentAvatar != nullptr && currentAvatar != newAvatarToUse) // If we are switching avatars (there was previously one in use)
    {
        if (ThisClass* previous = currentAvatar->FindComponentByClass<ThisClass>()) // Get the previous ASSActorComponent_AvatarActorExtension (the extension component of the old avatar actor)
        {
            if (previous->AbilitySystemComponent == &inASC)
            {
                // Our old avatar actor forgot to uninitialize the ASC    OR    our old avatar actor hasn't been destroyed by replication yet during respawn
                // We will uninitialize the ASC from the old avatar before initializing it with this new avatar
                GC_CLOG_STR_UOBJECT(this,
                    GetOwnerRole() == ROLE_Authority, // Only on the Authority because we can be certain there is something wrong if the server gets here (regardless this log should catch our attention and get us to fix it).
                    LogASSAvatarActorExtensionComponent,
                    Warning,
                    TEXT("Looks like you forgot to uninitialize the ASC before initializing a new one. Maybe you forgot to uninitialize on UnPossessed() - this will probably cause unwanted side-effects such as Gameplay Effects lingering after UnPossessed(). We are uninitializing the ASC for you before initializing the new one BUT you should manually do this instead to prevent lingering stuff.")
                    );
                previous->UninitializeAbilitySystemComponent(); // kick out the old avatar from the ASC
            }
        }
    }

    inASC.InitAbilityActorInfo(inASC.GetOwnerActor(), newAvatarToUse);

    // Grant abilities, effects, and attribute aets
    if (GetOwnerRole() == ROLE_Authority)
    {
        if (!bGrantedAbilitySets)
        {
            for (const TSubclassOf<UASSAbilitySet> abilitySet : AbilitySets)
            {
                if (abilitySet)
                {
                    FASSAbilitySetGrantedHandles& newAbilitySetGrantedHandles = GrantedHandles.AddDefaulted_GetRef();
                    abilitySet.GetDefaultObject()->GrantToAbilitySystemComponent(inASC, *GetOwner(), newAbilitySetGrantedHandles);
                }
            }
            bGrantedAbilitySets = true;
        }
    }

    bInitialized = true;
    AbilitySystemComponent = &inASC;
    OnInitializeAbilitySystemComponentDelegate.Broadcast(inASC);
}

void UASSActorComponent_AvatarActorExtension::UninitializeAbilitySystemComponent()
{
    check(GetOwner());

    UAbilitySystemComponent* asc = AbilitySystemComponent.Get();
    if (!asc)
    {
        GC_LOG_STR_UOBJECT(this,
            LogASSAvatarActorExtensionComponent,
            Log,
            GCUtils::Materialize(TStringBuilder<256>()) << TEXT("Tried uninitializing ASC for actor [") << GCUtils::String::GetUObjectNameSafe(GetOwner()) << TEXT("], but there's no ASC to uninitialize.")
            );
        return;
    }

    if (!ensureAlways(asc->GetAvatarActor() == GetOwner()))
    {
        GC_LOG_STR_UOBJECT(this,
            LogASSAvatarActorExtensionComponent,
            Error,
            GCUtils::Materialize(TStringBuilder<256>()) << TEXT("Tried uninitializing the ASC for actor [") << GCUtils::String::GetUObjectNameSafe(GetOwner()) << TEXT("], but the actor wasn't the avatar actor.")
            );
        return;
    }

    GC_LOG_FMT_UOBJECT(this,
        LogASSAvatarActorExtensionComponent,
        Log,
        TEXT("Uninitializing ability system component [%s] for actor [%s]."), *asc->GetName(), *GetOwner()->GetName());

    bInitialized = false;

    // Cancel ongoing stuff.
    asc->CancelAbilities(nullptr, nullptr);
    asc->RemoveAllGameplayCues();


    // Remove granted ability sets.
    if (GetOwnerRole() == ROLE_Authority)
    {
        for (FASSAbilitySetGrantedHandles grantHandle : GrantedHandles)
        {
            grantHandle.RemoveFromAbilitySystemComponent();
        }
    }

    // Remove loose gameplay tags.
    RemoveLooseAvatarRelatedTags(*asc);


    // Clear the avatar actor from the ASC
    if (asc->GetOwnerActor())
    {
        // Clear our avatar actor from it (this will re-init other actor info as well).
        asc->SetAvatarActor(nullptr);
    }
    else
    {
        // Clear ALL actor info because don't even have an owner actor for some reason.
        asc->ClearActorInfo();
    }

    AbilitySystemComponent = nullptr;
}

void UASSActorComponent_AvatarActorExtension::RemoveLooseAvatarRelatedTags(UAbilitySystemComponent& inASC)
{
    RemoveLooseAvatarRelatedTagsDelegate.Broadcast(inASC);
}
