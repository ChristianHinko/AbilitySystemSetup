// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ASSAbilitySystemBlueprintLibrary.h"

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

FGameplayTargetDataFilterHandle UASSAbilitySystemBlueprintLibrary::MakeASSFilterHandle(const FASSGameplayTargetDataFilter& SSFilter, AActor* FilterActor)
{
	FGameplayTargetDataFilterHandle FilterHandle;
	FASSGameplayTargetDataFilter* NewFilter = new FASSGameplayTargetDataFilter(SSFilter);
	NewFilter->InitializeFilterContext(FilterActor);
	FilterHandle.Filter = TSharedPtr<FASSGameplayTargetDataFilter>(NewFilter);
	return FilterHandle;
}
FGameplayTargetDataFilterHandle UASSAbilitySystemBlueprintLibrary::MakeMultiFilterHandle(const FGTDF_MultiFilter& SSFilter, AActor* FilterActor)
{
	FGameplayTargetDataFilterHandle FilterHandle;
	FGTDF_MultiFilter* NewFilter = new FGTDF_MultiFilter(SSFilter);
	NewFilter->InitializeFilterContext(FilterActor);
	FilterHandle.Filter = TSharedPtr<FGTDF_MultiFilter>(NewFilter);
	return FilterHandle;
}
