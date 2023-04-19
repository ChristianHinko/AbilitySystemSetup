// Fill out your copyright notice in the Description page of Project Settings.


#include "Subobjects/ASSActorComponent_AvatarActorExtension.h"

#include "AbilitySystem/ASSAbilitySystemBlueprintLibrary.h"
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
#include "Subobjects/ASSActorComponent_PawnAvatarActorExtension.h"
#endif // !(UE_BUILD_SHIPPING || UE_BUILD_TEST)



UASSActorComponent_AvatarActorExtension::UASSActorComponent_AvatarActorExtension(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;

	bInitialized = false;
}

void UASSActorComponent_AvatarActorExtension::OnRegister()
{
	Super::OnRegister();


#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (GetOwner()->IsA<APawn>() && this->IsA<UASSActorComponent_PawnAvatarActorExtension>() == false)
	{
		UE_LOG(LogASSAvatarExtensionComponent, Error, TEXT("%s() Incorrect use of %s. Use the pawn-specific version: %s. Component owner is a Pawn: [%s]"), ANSI_TO_TCHAR(__FUNCTION__), TNameOf<UASSActorComponent_AvatarActorExtension>::GetName(), TNameOf<UASSActorComponent_PawnAvatarActorExtension>::GetName(), *GetNameSafe(GetOwner()));
		check(0);
	}

	TArray<UActorComponent*> AvatarActorExtensionComponents;
	GetOwner()->GetComponents(ThisClass::StaticClass(), AvatarActorExtensionComponents);
	if (AvatarActorExtensionComponents.Num() > 1)
	{
		UE_LOG(LogASSAvatarExtensionComponent, Error, TEXT("%s() No more than one %s is allowed. Component owner: [%s]"), ANSI_TO_TCHAR(__FUNCTION__), TNameOf<UASSActorComponent_AvatarActorExtension>::GetName(), *GetNameSafe(GetOwner()));
		check(0);
	}
#endif // !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
}


void UASSActorComponent_AvatarActorExtension::InitializeAbilitySystemComponent(UAbilitySystemComponent* InASC)
{
	if (!IsValid(InASC))
	{
		UE_LOG(LogASSAvatarExtensionComponent, Error, TEXT("%s() failed to setup with GAS because InASC passed in was NULL"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}
	if (AbilitySystemComponent == InASC)
	{
		UE_LOG(LogASSAvatarExtensionComponent, Verbose, TEXT("%s() called again after already being initialized - no need to proceed"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}
	// Resolve edge case: You forgot to uninitialize the InASC before initializing a new one
	if (bInitialized || AbilitySystemComponent.IsValid())
	{
		UE_LOG(LogASSAvatarExtensionComponent, Warning, TEXT("%s() - Looks like you forgot to uninitialize the ASC before initializing a new one. Maybe you forgot to uninitialize on UnPossessed() - this will probably cause unwanted side-effects such as Gameplay Effects lingering after UnPossessed(). We are uninitializing the ASC for you before initializing the new one BUT you should manually do this instead to prevent lingering stuff."), ANSI_TO_TCHAR(__FUNCTION__));
		UninitializeAbilitySystemComponent();
	}

	AActor* CurrentAvatar = InASC->GetAvatarActor(); // the passed in ASC's old avatar
	AActor* NewAvatarToUse = GetOwner();			 // new avatar for the passed in ASC
	UE_LOG(LogASSAvatarExtensionComponent, Verbose, TEXT("%s() setting up ASC: [%s] on actor: [%s] with owner: [%s] and Avatar Actor: [%s]"), ANSI_TO_TCHAR(__FUNCTION__), *GetNameSafe(InASC), *GetNameSafe(NewAvatarToUse), *GetNameSafe(InASC->GetOwnerActor()), *GetNameSafe(CurrentAvatar));

	// Resolve edge cases: You forgot to uninitialize the ASC before initializing a new one    OR    destruction of previous avatar hasn't been replicated yet (because of lagged client)
	if ((CurrentAvatar != nullptr) && (CurrentAvatar != NewAvatarToUse))	// if we are switching avatars (there was previously one in use)
	{
		if (ThisClass* Previous = CurrentAvatar->FindComponentByClass<ThisClass>())		// get the previous ASSActorComponent_AvatarActorExtension (the extension component of the old avatar actor)
		{
			if (Previous->AbilitySystemComponent == InASC)
			{
				// Our old avatar actor forgot to uninitialize the ASC    OR    our old avatar actor hasn't been destroyed by replication yet during respawn
				// We will uninitialize the ASC from the old avatar before initializing it with this new avatar
				UE_CLOG(GetOwnerRole() == ROLE_Authority, LogASSAvatarExtensionComponent, Warning, TEXT("%s() - Looks like you forgot to uninitialize the ASC before initializing a new one. Maybe you forgot to uninitialize on UnPossessed() - this will probably cause unwanted side-effects such as Gameplay Effects lingering after UnPossessed(). We are uninitializing the ASC for you before initializing the new one BUT you should manually do this instead to prevent lingering stuff."), ANSI_TO_TCHAR(__FUNCTION__));	// only on the Authority because we can be certain there is something wrong if the server gets here (regardless this log should catch our attention and get us to fix it)
				Previous->UninitializeAbilitySystemComponent();		// kick out the old avatar from the ASC
			}
		}
	}



	AbilitySystemComponent = InASC;
	AbilitySystemComponent->InitAbilityActorInfo(InASC->GetOwnerActor(), NewAvatarToUse);

	// Grant Abilities, Active Effects, and Attribute Sets
	if (GetOwnerRole() == ROLE_Authority)
	{
		if (!bGrantedAbilitySets)
		{
			for (const TSubclassOf<UASSAbilitySet> AbilitySet : AbilitySets)
			{
				if (IsValid(AbilitySet))
				{
					FASSAbilitySetGrantedHandles& NewAbilitySetGrantedHandles = GrantedHandles.AddDefaulted_GetRef(); // currently is empty but we will give this its proper data next
					AbilitySet.GetDefaultObject()->GrantToAbilitySystemComponent(InASC, GetOwner(), NewAbilitySetGrantedHandles); // grant AbilitySet as well as give the newly added handle its proper data
				}
			}
			bGrantedAbilitySets = true;
		}
	}

	bInitialized = true;
	OnInitializeAbilitySystemComponentDelegate.Broadcast(AbilitySystemComponent.Get());
}
void UASSActorComponent_AvatarActorExtension::UninitializeAbilitySystemComponent()
{
	bInitialized = false;

	if (AbilitySystemComponent.IsValid())
	{
		if (AbilitySystemComponent->GetAvatarActor() == GetOwner())
		{
			// Cancel ongoing stuff
			AbilitySystemComponent->CancelAbilities(nullptr, nullptr);
			AbilitySystemComponent->RemoveAllGameplayCues();


			// Remove granted AbilitySets
			if (GetOwnerRole() == ROLE_Authority)
			{
				for (FASSAbilitySetGrantedHandles GrantHandle : GrantedHandles)
				{
					GrantHandle.RemoveFromAbilitySystemComponent();
				}
			}

			// Remove Loose Gameplay Tags
			RemoveLooseAvatarRelatedTags();


			// Clear the AvatarActor from the ASC
			if (IsValid(AbilitySystemComponent->GetOwnerActor()))
			{
				// Clear our avatar actor from it (this will re-init other actor info as well)
				AbilitySystemComponent->SetAvatarActor(nullptr);
			}
			else
			{
				// Clear ALL actor info because don't even have an owner actor for some reason
				AbilitySystemComponent->ClearActorInfo();
			}
		}
		else
		{
			UE_LOG(LogASSAvatarExtensionComponent, Error, TEXT("%s() Tried uninitializing the ASC when the actor with this component was not the avatar actor"), ANSI_TO_TCHAR(__FUNCTION__));
		}
	}

	AbilitySystemComponent = nullptr;
}

void UASSActorComponent_AvatarActorExtension::RemoveLooseAvatarRelatedTags()
{
	if (AbilitySystemComponent.IsValid())
	{
		RemoveLooseAvatarRelatedTagsDelegate.Broadcast(AbilitySystemComponent.Get());
	}
}
