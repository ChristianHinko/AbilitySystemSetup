// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ASSAttributeSet.h"
#include "AbilitySystemComponent.h"

#include "AS_Damage.generated.h"



/**
 * 
 */
UCLASS()
class ABILITYSYSTEMSETUP_API UAS_Damage : public UASSAttributeSet
{
	GENERATED_BODY()

public:
	// Constructor and default values
	UAS_Damage();

	// Attributes
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OutgoingDamage, Category = "Attributes")
		FGameplayAttributeData OutgoingDamage;
	ATTRIBUTE_ACCESSORS(UAS_Damage, OutgoingDamage)

	UFUNCTION()
		virtual void OnRep_OutgoingDamage(const FGameplayAttributeData& ServerBaseValue);
};
