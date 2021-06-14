// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

#include "BAA_GameplayTagAddedRemoved.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameplayTagAddedRemoved, FGameplayTag, Tag);


/**
 * Blueprint node to automatically register a listener for FGameplayTags added and removed.
 * Useful to use in Blueprint/UMG.
 */
UCLASS(BlueprintType, meta = (ExposedAsyncProxy = AsyncTask))
class ABILITYSYSTEMSETUP_API UBAA_GameplayTagAddedRemoved : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
		FOnGameplayTagAddedRemoved OnTagAdded;

	UPROPERTY(BlueprintAssignable)
		FOnGameplayTagAddedRemoved OnTagRemoved;

	/** Listens for FGameplayTags added and removed. */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
		static UBAA_GameplayTagAddedRemoved* ListenForGameplayTagAddedOrRemoved(UAbilitySystemComponent* AbilitySystemComponent, FGameplayTagContainer Tags);

	/**
	 * You must call this function manually when you want the AsyncTask to end.
	 * For UMG Widgets, you would call it in the Widget's Destruct event.
	 */	
	UFUNCTION(BlueprintCallable)
		void EndTask();

protected:
	UPROPERTY()
		UAbilitySystemComponent* ASC;

	FGameplayTagContainer Tags;

	virtual void TagChanged(const FGameplayTag Tag, int32 NewCount);
};
