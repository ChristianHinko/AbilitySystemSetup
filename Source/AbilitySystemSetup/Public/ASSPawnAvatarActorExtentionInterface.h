// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ASSAvatarActorExtentionInterface.h"
#include "ActorComponents/ASSActorComponent_PawnAvatarActorExtension.h"
#include "ASSPawnAvatarActorExtentionInterface.generated.h"

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UASSPawnAvatarActorExtentionInterface : public UASSAvatarActorExtentionInterface
{
    GENERATED_BODY()
};

class ABILITYSYSTEMSETUP_API IASSPawnAvatarActorExtentionInterface : public IASSAvatarActorExtentionInterface
{
    GENERATED_BODY()

public:
    
    /**
     * @brief Accessor for the avatar actor extention struct. This overrides exists to return the more
     *        specialized FASSActorComponent_PawnAvatarActorExtension extention struct.
     */
    virtual FASSActorComponent_PawnAvatarActorExtension& GetASSAvatarActorExtension() override = 0;
};
