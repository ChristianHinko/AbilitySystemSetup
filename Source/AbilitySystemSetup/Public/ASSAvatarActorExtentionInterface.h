// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ASSAvatarActorExtentionInterface.generated.h"

struct FASSActorComponent_AvatarActorExtension;

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UASSAvatarActorExtentionInterface : public UInterface
{
    GENERATED_BODY()
};

class ABILITYSYSTEMSETUP_API IASSAvatarActorExtentionInterface
{
    GENERATED_BODY()

public:

    /**
     * @brief Accessor for the avatar actor extention struct.
     */
    virtual FASSActorComponent_AvatarActorExtension& GetASSAvatarActorExtension() = 0;
};
