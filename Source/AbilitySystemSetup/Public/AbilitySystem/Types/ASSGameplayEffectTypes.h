// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"

#include "ASSGameplayEffectTypes.generated.h"



/**
 * Base FGameplayEffectContext struct. Nothing different from parent yet
 */
USTRUCT()
struct ABILITYSYSTEMSETUP_API FASSGameplayEffectContext : public FGameplayEffectContext
{
    GENERATED_BODY()

public:
    FASSGameplayEffectContext();

    /** Returns the actual struct used for serialization, subclasses must override this! */
    virtual UScriptStruct* GetScriptStruct() const override
    {
        return StaticStruct();
    }

    /** Creates a copy of this context, used to duplicate for later modifications */
    virtual FASSGameplayEffectContext* Duplicate() const override
    {
        FASSGameplayEffectContext* NewContext = new FASSGameplayEffectContext();
        *NewContext = *this;
        NewContext->AddActors(Actors);
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
		WithCopy = true		// Necessary so that TSharedPtr<FHitResult> Data is copied around
	};
};
