// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

#include "DS_AbilitySystemSetup.generated.h"



/**
 * 
 */
UCLASS(config=Game, defaultconfig, meta=(DisplayName = "Ability System Setup"))
class ABILITYSYSTEMSETUP_API UDS_AbilitySystemSetup : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UDS_AbilitySystemSetup(const FObjectInitializer& ObjectInitializer);


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", config, meta = (DisplayName = "AbilityInputID Enum Name"))
		FString AbilityInputIDEnumName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", config, meta = (DisplayName = "ConfirmTarget Input Action Name"))
		FString ConfirmTargetInputActionName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", config, meta = (DisplayName = "CancelTarget Input Action Name"))
		FString CancelTargetInputActionName;

};
