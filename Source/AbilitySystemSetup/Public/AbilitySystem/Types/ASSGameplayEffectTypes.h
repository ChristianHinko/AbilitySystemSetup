// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"

#include "ASSGameplayEffectTypes.generated.h"



/**
 * Base FGameplayEffectContext struct
 */
USTRUCT()
struct ABILITYSYSTEMSETUP_API FASSGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

public:
	FASSGameplayEffectContext();

	virtual UScriptStruct* GetScriptStruct() const override { return StaticStruct(); }

	virtual FASSGameplayEffectContext* Duplicate() const override
	{
		FASSGameplayEffectContext* NewContext = new FASSGameplayEffectContext(); // allocate our version
		*NewContext = *this;
		if (GetHitResult())
		{
			// Does a deep copy of the hit result
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;
};

template<>
struct TStructOpsTypeTraits<FASSGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FASSGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};
