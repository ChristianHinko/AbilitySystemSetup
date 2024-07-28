// Fill out your copyright notice in the Description page of Project Settings.


#include "Subobjects/ASSActorComponent_AvatarActorExtension.h"

#include "AbilitySystem/Types/ASSAbilitySet.h"
#include "AbilitySystem/ASSAbilitySystemBlueprintLibrary.h"
#include "GCUtils_Log.h"
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
#include "Subobjects/ASSActorComponent_PawnAvatarActorExtension.h"
#endif // !(UE_BUILD_SHIPPING || UE_BUILD_TEST)

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

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    if (GetOwner()->IsA<APawn>() && this->IsA<UASSActorComponent_PawnAvatarActorExtension>() == false)
    {
        GC_LOG_UOBJECT(this, LogASSAvatarActorExtensionComponent, Error, TEXT("Incorrect usage of this component. Use the pawn specific version: [%s]. Component owner is a Pawn: [%s]"), TNameOf<UASSActorComponent_PawnAvatarActorExtension>::GetName(), *GetNameSafe(GetOwner()));
        check(0);
    }

    TArray<UActorComponent*> avatarActorExtensionComponents;
    GetOwner()->GetComponents(ThisClass::StaticClass(), avatarActorExtensionComponents);
    if (avatarActorExtensionComponents.Num() > 1)
    {
        GC_LOG_UOBJECT(this, LogASSAvatarActorExtensionComponent, Error, TEXT("No more than one [%s] is allowed. Component owner: [%s]"), TNameOf<UASSActorComponent_AvatarActorExtension>::GetName(), *GetNameSafe(GetOwner()));
        check(0);
    }
#endif // !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
}


void UASSActorComponent_AvatarActorExtension::InitializeAbilitySystemComponent(UAbilitySystemComponent& inASC)
{
    if (AbilitySystemComponent == &inASC)
    {
        GC_LOG_UOBJECT(this, LogASSAvatarActorExtensionComponent, Verbose, TEXT("Called again after already being initialized - no need to proceed"));
        return;
    }
    // Resolve edge case: You forgot to uninitialize the inASC before initializing a new one
    if (bInitialized || AbilitySystemComponent.IsValid())
    {
        GC_LOG_UOBJECT(this,
            LogASSAvatarActorExtensionComponent,
            Warning,
            TEXT("Looks like you forgot to uninitialize the ASC before initializing a new one. Maybe you forgot to uninitialize on UnPossessed() - this will probably cause unwanted side-effects such as Gameplay Effects lingering after UnPossessed(). We are uninitializing the ASC for you before initializing the new one BUT you should manually do this instead to prevent lingering stuff."));
        UninitializeAbilitySystemComponent();
    }

    AActor* currentAvatar = inASC.GetAvatarActor(); // the passed in ASC's old avatar
    AActor* newAvatarToUse = GetOwner();            // new avatar for the passed in ASC
    GC_LOG_UOBJECT(this,
        LogASSAvatarActorExtensionComponent,
        Verbose,
        TEXT("Setting up ASC [%s] on actor [%s] with owner [%s] and avatar actor [%s]"), *inASC.GetName(), *GetNameSafe(newAvatarToUse), *GetNameSafe(inASC.GetOwnerActor()), *GetNameSafe(currentAvatar));

    // Resolve edge cases: You forgot to uninitialize the ASC before initializing a new one    OR    destruction of previous avatar hasn't been replicated yet (because of lagged client)
    if (currentAvatar != nullptr && currentAvatar != newAvatarToUse) // If we are switching avatars (there was previously one in use)
    {
        if (ThisClass* previous = currentAvatar->FindComponentByClass<ThisClass>()) // Get the previous ASSActorComponent_AvatarActorExtension (the extension component of the old avatar actor)
        {
            if (previous->AbilitySystemComponent == &inASC)
            {
                // Our old avatar actor forgot to uninitialize the ASC    OR    our old avatar actor hasn't been destroyed by replication yet during respawn
                // We will uninitialize the ASC from the old avatar before initializing it with this new avatar
                GC_CLOG_UOBJECT(this,
                    GetOwnerRole() == ROLE_Authority,
                    LogASSAvatarActorExtensionComponent,
                    Warning,
                    TEXT("Looks like you forgot to uninitialize the ASC before initializing a new one. Maybe you forgot to uninitialize on UnPossessed() - this will probably cause unwanted side-effects such as Gameplay Effects lingering after UnPossessed(). We are uninitializing the ASC for you before initializing the new one BUT you should manually do this instead to prevent lingering stuff."));    // only on the Authority because we can be certain there is something wrong if the server gets here (regardless this log should catch our attention and get us to fix it)
                previous->UninitializeAbilitySystemComponent(); // kick out the old avatar from the ASC
            }
        }
    }

    inASC.InitAbilityActorInfo(inASC.GetOwnerActor(), newAvatarToUse);

    // Grant Abilities, Active Effects, and Attribute Sets
    if (GetOwnerRole() == ROLE_Authority)
    {
        if (!bGrantedAbilitySets)
        {
            for (const TSubclassOf<UASSAbilitySet> abilitySet : AbilitySets)
            {
                if (IsValid(abilitySet))
                {
                    FASSAbilitySetGrantedHandles& newAbilitySetGrantedHandles = GrantedHandles.AddDefaulted_GetRef(); // currently is empty but we will give this its proper data next
                    abilitySet.GetDefaultObject()->GrantToAbilitySystemComponent(&inASC, GetOwner(), newAbilitySetGrantedHandles); // grant AbilitySet as well as give the newly added handle its proper data
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
    UAbilitySystemComponent* asc = AbilitySystemComponent.Get();
    if (!asc)
    {
        GC_LOG_UOBJECT(this,
            LogASSAvatarActorExtensionComponent,
            Log,
            TEXT("Tried uninitializing ASC for actor [%s], but there's no ASC to uninitialize. "));
        return;
    }

    if (!ensureAlways(asc->GetAvatarActor() == GetOwner()))
    {
        GC_LOG_UOBJECT(this,
            LogASSAvatarActorExtensionComponent,
            Error,
            TEXT("Tried uninitializing the ASC for actor [%s], but the actor wasn't the avatar actor."));
        return;
    }

    bInitialized = false;

    // Cancel ongoing stuff
    asc->CancelAbilities(nullptr, nullptr);
    asc->RemoveAllGameplayCues();


    // Remove granted AbilitySets
    if (GetOwnerRole() == ROLE_Authority)
    {
        for (FASSAbilitySetGrantedHandles grantHandle : GrantedHandles)
        {
            grantHandle.RemoveFromAbilitySystemComponent();
        }
    }

    // Remove Loose Gameplay Tags
    RemoveLooseAvatarRelatedTags(*asc);


    // Clear the AvatarActor from the ASC
    if (asc->GetOwnerActor())
    {
        // Clear our avatar actor from it (this will re-init other actor info as well)
        asc->SetAvatarActor(nullptr);
    }
    else
    {
        // Clear ALL actor info because don't even have an owner actor for some reason
        asc->ClearActorInfo();
    }

    AbilitySystemComponent = nullptr;
}

void UASSActorComponent_AvatarActorExtension::RemoveLooseAvatarRelatedTags(UAbilitySystemComponent& inASC)
{
    RemoveLooseAvatarRelatedTagsDelegate.Broadcast(inASC);
}
