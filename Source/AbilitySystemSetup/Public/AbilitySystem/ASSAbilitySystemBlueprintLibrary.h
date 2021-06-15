// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/TargetActor/ASSGameplayAbilityTargetDataFilter.h"

#include "ASSAbilitySystemBlueprintLibrary.generated.h"



/**
 * Base AbilitySystemBlueprintLibrary
 */
UCLASS()
class ABILITYSYSTEMSETUP_API UASSAbilitySystemBlueprintLibrary : public UAbilitySystemBlueprintLibrary
{
	GENERATED_BODY()
	
public:
	/** Create a handle for filtering target data, filling out all fields */
	UFUNCTION(BlueprintPure, Category = "Filter")
		static FGameplayTargetDataFilterHandle MakeASSFilterHandle(const FASSGameplayTargetDataFilter& SSFilter, AActor* FilterActor);
	/** Create a handle for filtering target data, filling out all fields */
	UFUNCTION(BlueprintPure, Category = "Filter")
		static FGameplayTargetDataFilterHandle MakeMultiFilterHandle(const FGTDF_MultiFilter& SSFilter, AActor* FilterActor);

};
