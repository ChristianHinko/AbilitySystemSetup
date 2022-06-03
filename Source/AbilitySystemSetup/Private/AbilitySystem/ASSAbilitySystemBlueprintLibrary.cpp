// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ASSAbilitySystemBlueprintLibrary.h"

#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "AbilitySystem/TargetActor/ASSGameplayAbilityTargetDataFilter.h"



UAttributeSet* UASSAbilitySystemBlueprintLibrary::GetAttributeSet(const UAbilitySystemComponent* InASC, const TSubclassOf<UAttributeSet> InAttributeSetClass)
{
	// Potentially find an Attribute Set of this class
	UAttributeSet* const * const FoundAttributeSet = InASC->GetSpawnedAttributes().FindByPredicate(
		[&InAttributeSetClass](const UAttributeSet* AS)
		{
			return (AS->GetClass() == InAttributeSetClass);
		}
	);

	if (FoundAttributeSet)
	{
		// Found it!
		return *FoundAttributeSet;
	}

	return nullptr;
}


void UASSAbilitySystemBlueprintLibrary::ExecuteGameplayCueLocal(const UAbilitySystemComponent* InASC, const FGameplayTag& InGameplayCueTag, const FGameplayCueParameters& InGameplayCueParameters)
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(InASC->GetOwner(), InGameplayCueTag, EGameplayCueEvent::Type::Executed, InGameplayCueParameters);
}
void UASSAbilitySystemBlueprintLibrary::AddGameplayCueLocal(const UAbilitySystemComponent* InASC, const FGameplayTag& InGameplayCueTag, const FGameplayCueParameters& InGameplayCueParameters)
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(InASC->GetOwner(), InGameplayCueTag, EGameplayCueEvent::Type::OnActive, InGameplayCueParameters);
}
void UASSAbilitySystemBlueprintLibrary::RemoveGameplayCueLocal(const UAbilitySystemComponent* InASC, const FGameplayTag& InGameplayCueTag, const FGameplayCueParameters& InGameplayCueParameters)
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(InASC->GetOwner(), InGameplayCueTag, EGameplayCueEvent::Type::Removed, InGameplayCueParameters);
}

void UASSAbilitySystemBlueprintLibrary::GetActiveAbilitiesWithTags(const UAbilitySystemComponent* InASC, const FGameplayTagContainer& InTags, TArray<UGameplayAbility*>& OutActiveAbilities)
{
	// Get the active specs by tags
	TArray<FGameplayAbilitySpec*> ActiveAbilitySpecsByTags;
	InASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(InTags, ActiveAbilitySpecsByTags, false);

	// Get all of the instances of these specs
	for (const FGameplayAbilitySpec* Spec : ActiveAbilitySpecsByTags)
	{
		for (UGameplayAbility* ActiveAbility : Spec->GetAbilityInstances())
		{
			OutActiveAbilities.Add(ActiveAbility);
		}
	}
}

FGameplayAbilitySpecHandle UASSAbilitySystemBlueprintLibrary::FindAbilitySpecHandleFromClass(UAbilitySystemComponent* InASC, const TSubclassOf<UGameplayAbility> InAbilityClass, const UObject* InSourceObject)
{
	// Find by class
	FScopedAbilityListLock ActiveScopeLock(*InASC); // ABILITYLIST_SCOPE_LOCK
	for (const FGameplayAbilitySpec& Spec : InASC->GetActivatableAbilities())
	{
		if (Spec.Ability->GetClass() == InAbilityClass)
		{
			// Use the specified source object
			if (InSourceObject && Spec.SourceObject != InSourceObject)
			{
				continue;
			}

			// Found it
			return Spec.Handle;
		}
	}

	return FGameplayAbilitySpecHandle();
}

void UASSAbilitySystemBlueprintLibrary::GiveAbilities(UAbilitySystemComponent* InASC, const TArray<FGameplayAbilitySpec>& InAbilities)
{
	if (::IsValid(InASC) == false) // use global scope to avoid using the UAbilitySystemBlueprintLibrary::IsValid()
	{
		UE_LOG(LogGameplayAbilitySetup, Warning, TEXT("%s() InASC was not valid when trying to give list of abilities. Did nothing"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}
	if (InASC->IsOwnerActorAuthoritative() == false)
	{
		UE_LOG(LogGameplayAbilitySetup, Warning, TEXT("%s() called without Authority. Did nothing"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	for (const FGameplayAbilitySpec& SpecToGive : InAbilities)
	{
		if (InASC->GetActivatableAbilities().ContainsByPredicate(
			[&SpecToGive](const FGameplayAbilitySpec& Spec)
			{
				return Spec.Ability == SpecToGive.Ability;
			}
		) == false)
		{
			InASC->GiveAbility(SpecToGive);
		}
	}
}

void UASSAbilitySystemBlueprintLibrary::TargetConfirmByAbility(UAbilitySystemComponent* InASC, const UGameplayAbility* InAbility)
{
	// Callbacks may modify the spawned target actor array so iterate over a copy instead
	TArray<AGameplayAbilityTargetActor*> LocalTargetActors = InASC->SpawnedTargetActors;
	InASC->SpawnedTargetActors.Reset();
	for (AGameplayAbilityTargetActor* TargetActor : LocalTargetActors)
	{
		if (TargetActor)
		{
			if (TargetActor->IsConfirmTargetingAllowed())
			{
				if (TargetActor->OwningAbility == InAbility) // =@MODIFIED MARKER@= wrapped in this if statement
				{
					//TODO: There might not be any cases where this bool is false
					if (!TargetActor->bDestroyOnConfirmation)
					{
						InASC->SpawnedTargetActors.Add(TargetActor);
					}
					TargetActor->ConfirmTargeting();
				}
			}
			else
			{
				InASC->SpawnedTargetActors.Add(TargetActor);
			}
		}
	}
}
void UASSAbilitySystemBlueprintLibrary::TargetCancelByAbility(UAbilitySystemComponent* InASC, const UGameplayAbility* InAbility)
{
	// Callbacks may modify the spawned target actor array so iterate over a copy instead
	TArray<AGameplayAbilityTargetActor*> LocalTargetActors = InASC->SpawnedTargetActors;
	InASC->SpawnedTargetActors.Reset();
	for (AGameplayAbilityTargetActor* TargetActor : LocalTargetActors)
	{
		if (TargetActor)
		{
			if (TargetActor->OwningAbility == InAbility) // =@MODIFIED MARKER@= wrapped in this if statement
			{
				TargetActor->CancelTargeting();
			}
			else // =@MODIFIED MARKER@= add this else statement
			{
				InASC->SpawnedTargetActors.Add(TargetActor);
			}
		}
	}
}

FGameplayTargetDataFilterHandle UASSAbilitySystemBlueprintLibrary::MakeASSFilterHandle(const FASSGameplayTargetDataFilter& ASSFilter, AActor* SelfActor)
{
	FGameplayTargetDataFilterHandle FilterHandle;
	FASSGameplayTargetDataFilter* NewFilter = new FASSGameplayTargetDataFilter(ASSFilter);
	NewFilter->InitializeFilterContext(SelfActor);
	FilterHandle.Filter = TSharedPtr<FGameplayTargetDataFilter>(NewFilter);
	return FilterHandle;
}
FGameplayTargetDataFilterHandle UASSAbilitySystemBlueprintLibrary::MakeMultiFilterHandle(const FGTDF_MultiFilter& MultiFilter, AActor* SelfActor)
{
	FGameplayTargetDataFilterHandle FilterHandle;
	FGTDF_MultiFilter* NewFilter = new FGTDF_MultiFilter(MultiFilter);
	NewFilter->InitializeFilterContext(SelfActor);
	FilterHandle.Filter = TSharedPtr<FGameplayTargetDataFilter>(NewFilter);
	return FilterHandle;
}
