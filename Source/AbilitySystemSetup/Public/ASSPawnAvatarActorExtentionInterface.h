// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ASSAvatarActorExtentionInterface.h"
#include "ASSPawnAvatarActorExtensionStruct.h"
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
     *        specialized FASSPawnAvatarActorExtensionStruct extention struct.
     */
    virtual FASSPawnAvatarActorExtensionStruct& GetASSAvatarActorExtension() override = 0;
};
