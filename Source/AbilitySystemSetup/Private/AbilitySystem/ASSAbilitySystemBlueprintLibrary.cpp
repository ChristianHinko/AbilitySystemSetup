// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ASSAbilitySystemBlueprintLibrary.h"

#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "AbilitySystem/TargetActor/ASSGameplayAbilityTargetDataFilter.h"



UAttributeSet* UASSAbilitySystemBlueprintLibrary::GetAttributeSet(const UAbilitySystemComponent* ASC, const TSubclassOf<UAttributeSet> AttributeSetClass)
{
	// Potentially find an Attribute Set of this class
	UAttributeSet* const * const FoundAttributeSet = ASC->GetSpawnedAttributes().FindByPredicate(
		[&AttributeSetClass](const UAttributeSet* AS)
		{
			return (AS->GetClass() == AttributeSetClass);
		}
	);

	if (FoundAttributeSet)
	{
		// Found it!
		return *FoundAttributeSet;
	}

	return nullptr;
}


void UASSAbilitySystemBlueprintLibrary::ExecuteGameplayCueLocal(const UAbilitySystemComponent* InASC, const FGameplayTag& GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters)
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(InASC->GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::Executed, GameplayCueParameters);
}
void UASSAbilitySystemBlueprintLibrary::AddGameplayCueLocal(const UAbilitySystemComponent* InASC, const FGameplayTag& GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters)
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(InASC->GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::OnActive, GameplayCueParameters);
}
void UASSAbilitySystemBlueprintLibrary::RemoveGameplayCueLocal(const UAbilitySystemComponent* InASC, const FGameplayTag& GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters)
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(InASC->GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::Removed, GameplayCueParameters);
}

void UASSAbilitySystemBlueprintLibrary::GetActiveAbilitiesWithTags(const UAbilitySystemComponent* InASC, const FGameplayTagContainer& GameplayTagContainer, TArray<UGameplayAbility*>& ActiveAbilities)
{
	TArray<FGameplayAbilitySpec*> AbilitiesToActivate;
	InASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(GameplayTagContainer, AbilitiesToActivate, false);

	// Iterate the list of all ability specs
	for (FGameplayAbilitySpec* Spec : AbilitiesToActivate)
	{
		// Iterate all instances on this ability spec
		TArray<UGameplayAbility*> AbilityInstances = Spec->GetAbilityInstances();

		for (UGameplayAbility* ActiveAbility : AbilityInstances)
		{
			ActiveAbilities.Add(Cast<UGameplayAbility>(ActiveAbility));
		}
	}
}

FGameplayAbilitySpecHandle UASSAbilitySystemBlueprintLibrary::FindAbilitySpecHandleFromClass(UAbilitySystemComponent* InASC, TSubclassOf<UGameplayAbility> AbilityClass, UObject* OptionalSourceObject)
{
	FScopedAbilityListLock ActiveScopeLock(*InASC); // ABILITYLIST_SCOPE_LOCK

	for (const FGameplayAbilitySpec& Spec : InASC->GetActivatableAbilities())
	{
		TSubclassOf<UGameplayAbility> SpecAbilityClass = Spec.Ability->GetClass();
		if (SpecAbilityClass == AbilityClass)
		{
			if (!OptionalSourceObject || (OptionalSourceObject && Spec.SourceObject == OptionalSourceObject))
			{
				return Spec.Handle;
			}
		}
	}

	return FGameplayAbilitySpecHandle();
}

void UASSAbilitySystemBlueprintLibrary::GiveAbilities(UAbilitySystemComponent* ASC, const TArray<FGameplayAbilitySpec>& Abilities)
{
	if (::IsValid(ASC) == false)	// :: uses global scope instead to avoid using the UAbilitySystemBlueprintLibrary::IsValid()
	{
		UE_LOG(LogGameplayAbilitySetup, Warning, TEXT("%s() ASC was not valid when trying to give list of abilities. Did nothing"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}
	if (ASC->IsOwnerActorAuthoritative() == false)
	{
		UE_LOG(LogGameplayAbilitySetup, Warning, TEXT("%s() called without Authority. Did nothing"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	for (const FGameplayAbilitySpec& SpecToGive : Abilities)
	{
		if (ASC->GetActivatableAbilities().ContainsByPredicate(
			[&SpecToGive](const FGameplayAbilitySpec& Spec)
			{
				return Spec.Ability == SpecToGive.Ability;
			}
		) == false)
		{
			ASC->GiveAbility(SpecToGive);
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
