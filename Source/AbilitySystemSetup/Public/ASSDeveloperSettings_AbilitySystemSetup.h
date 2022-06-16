// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

#include "ASSDeveloperSettings_AbilitySystemSetup.generated.h"



/**
 * The plugin's developer settings is useful so that we can avoid hardcoding game specific values in the GAS setup logic.
 * This helps us keep the GAS setup generic to any game.
 */
UCLASS(config=Game, defaultconfig, meta=(DisplayName = "Ability System Setup"))
class ABILITYSYSTEMSETUP_API UASSDeveloperSettings_AbilitySystemSetup : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UASSDeveloperSettings_AbilitySystemSetup(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", config, meta = (DisplayName = "AbilityInputID Enum Name"))
		FString AbilityInputIDEnumName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", config, meta = (DisplayName = "ConfirmTarget Input Action Name"))
		FString ConfirmTargetInputActionName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", config, meta = (DisplayName = "CancelTarget Input Action Name"))
		FString CancelTargetInputActionName;
};
