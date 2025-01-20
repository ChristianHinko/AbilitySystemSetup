// Fill out your copyright notice in the Description page of Project Settings.

#include "ActorComponents/ASSActorComponent_AvatarActorExtension.h"

#include "Types/ASSAbilitySet.h"
#include "AbilitySystemComponent.h"
#include "GCUtils_Log.h"
#if !NO_LOGGING || DO_CHECK
#include "ActorComponents/ASSActorComponent_PawnAvatarActorExtension.h"
#endif // #if !NO_LOGGING || DO_CHECK
#include "ASSPawnAvatarActorExtentionInterface.h"

DEFINE_LOG_CATEGORY_STATIC(LogASSAvatarActorExtensionComponent, Log, All);

void FASSActorComponent_AvatarActorExtension::InitializeAbilitySystemComponent(UAbilitySystemComponent& inASC, AActor& avatarActor)
{
    if (AbilitySystemComponent == &inASC)
    {
        GC_LOG_STR_UOBJECT(&avatarActor,
            LogASSAvatarActorExtensionComponent,
            Warning,
            TEXT("Called again after already being initialized - no need to proceed. We should probably track down the reason this is being called even after it's already initialized.")
            );
        return;
    }
    // Resolve edge case: You forgot to uninitialize the inASC before initializing a new one.
    if (bInitialized || AbilitySystemComponent.IsValid())
    {
        GC_LOG_STR_UOBJECT(&avatarActor,
            LogASSAvatarActorExtensionComponent,
            Warning,
            TEXT("Looks like you forgot to uninitialize the ASC before initializing a new one. Maybe you forgot to uninitialize on UnPossessed() - this will probably cause unwanted side-effects such as Gameplay Effects lingering after UnPossessed(). We are uninitializing the ASC for you before initializing the new one BUT you should manually do this instead to prevent lingering stuff.")
            );
        UninitializeAbilitySystemComponent(avatarActor);
    }

    AActor* currentAvatar = inASC.GetAvatarActor(); // The passed in ASC's old avatar.
    AActor* newAvatarToUse = &avatarActor;                // New avatar for the passed in ASC.
    GC_LOG_STR_UOBJECT(&avatarActor,
        LogASSAvatarActorExtensionComponent,
        Verbose,
        GCUtils::Materialize(TStringBuilder<512>()) << TEXT("Setting up ASC `") << inASC.GetFName() << TEXT("` on actor `") << GCUtils::String::GetUObjectNameSafe(newAvatarToUse) << TEXT("` with owner `") << GCUtils::String::GetUObjectNameSafe(inASC.GetOwnerActor()) << TEXT("` and avatar actor `") << GCUtils::String::GetUObjectNameSafe(currentAvatar) << TEXT("`.")
        );

    // Resolve edge cases: You forgot to uninitialize the ASC before initializing a new one    OR    destruction of previous avatar hasn't been replicated yet (because of lagged client).
    if (currentAvatar != nullptr && currentAvatar != newAvatarToUse) // If we are switching avatars (there was previously one in use).
    {
        if (IASSPawnAvatarActorExtentionInterface* previousAvatarActorInterface = Cast<IASSPawnAvatarActorExtentionInterface>(currentAvatar)) // Get the previous ASSActorComponent_AvatarActorExtension (the extension component of the old avatar actor).
        {
            FASSActorComponent_PawnAvatarActorExtension& previousAvatarActorExtension = previousAvatarActorInterface->GetASSAvatarActorExtension();

            if (previousAvatarActorExtension.AbilitySystemComponent == &inASC)
            {
                // Our old avatar actor forgot to uninitialize the ASC    OR    our old avatar actor hasn't been destroyed by replication yet during respawn.
                // We will uninitialize the ASC from the old avatar before initializing it with this new avatar.
                GC_CLOG_STR_UOBJECT(&avatarActor,
                    avatarActor.GetLocalRole() == ROLE_Authority, // Only on the Authority because we can be certain there is something wrong if the server gets here (regardless this log should catch our attention and get us to fix it).
                    LogASSAvatarActorExtensionComponent,
                    Warning,
                    TEXT("Looks like you forgot to uninitialize the ASC before initializing a new one. Maybe you forgot to uninitialize on UnPossessed() - this will probably cause unwanted side-effects such as Gameplay Effects lingering after UnPossessed(). We are uninitializing the ASC for you before initializing the new one BUT you should manually do this instead to prevent lingering stuff.")
                );
                previousAvatarActorExtension.UninitializeAbilitySystemComponent(avatarActor); // Kick out the old avatar from the ASC.
            }
        }

    }

    inASC.InitAbilityActorInfo(inASC.GetOwnerActor(), newAvatarToUse);

    // Grant abilities, effects, and attribute aets
    if (avatarActor.GetLocalRole() == ROLE_Authority)
    {
        if (!bGrantedAbilitySets)
        {
            for (const TSubclassOf<UASSAbilitySet> abilitySet : AbilitySets)
            {
                if (abilitySet)
                {
                    FASSAbilitySetGrantedHandles& newAbilitySetGrantedHandles = GrantedHandles.AddDefaulted_GetRef();
                    abilitySet.GetDefaultObject()->GrantToAbilitySystemComponent(inASC, avatarActor, newAbilitySetGrantedHandles);
                }
            }
            bGrantedAbilitySets = true;
        }
    }

    bInitialized = true;
    AbilitySystemComponent = &inASC;
    OnInitializeAbilitySystemComponentDelegate.Broadcast(inASC);
}

void FASSActorComponent_AvatarActorExtension::UninitializeAbilitySystemComponent(AActor& avatarActor)
{
    UAbilitySystemComponent* asc = AbilitySystemComponent.Get();
    if (!asc)
    {
        GC_LOG_STR_UOBJECT(&avatarActor,
            LogASSAvatarActorExtensionComponent,
            Log,
            GCUtils::Materialize(TStringBuilder<256>()) << TEXT("Tried uninitializing ASC for actor [") << avatarActor.GetFName().ToString() << TEXT("], but there's no ASC to uninitialize.")
            );
        return;
    }

    if (!ensureAlways(asc->GetAvatarActor() == &avatarActor))
    {
        GC_LOG_STR_UOBJECT(&avatarActor,
            LogASSAvatarActorExtensionComponent,
            Error,
            GCUtils::Materialize(TStringBuilder<256>()) << TEXT("Tried uninitializing the ASC for actor [") << avatarActor.GetFName().ToString() << TEXT("], but the actor wasn't the avatar actor.")
            );
        return;
    }

    GC_LOG_FMT_UOBJECT(&avatarActor,
        LogASSAvatarActorExtensionComponent,
        Log,
        TEXT("Uninitializing ability system component [%s] for actor [%s]."), *asc->GetName(), *avatarActor.GetFName().ToString());

    bInitialized = false;

    // Cancel ongoing stuff.
    asc->CancelAbilities(nullptr, nullptr);
    asc->RemoveAllGameplayCues();


    // Remove granted ability sets.
    if (avatarActor.GetLocalRole() == ROLE_Authority)
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

void FASSActorComponent_AvatarActorExtension::RemoveLooseAvatarRelatedTags(UAbilitySystemComponent& inASC)
{
    RemoveLooseAvatarRelatedTagsDelegate.Broadcast(inASC);
}
