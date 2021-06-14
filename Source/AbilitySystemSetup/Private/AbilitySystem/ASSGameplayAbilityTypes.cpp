// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ASSGameplayAbilityTypes.h"

#include "AbilitySystem/ASSAbilitySystemComponent.h"



FASSGameplayAbilityActorInfo::FASSGameplayAbilityActorInfo()
{

}


void FASSGameplayAbilityActorInfo::InitFromActor(AActor* InOwnerActor, AActor* InAvatarActor, UAbilitySystemComponent* InAbilitySystemComponent)
{
	Super::InitFromActor(InOwnerActor, InAvatarActor, InAbilitySystemComponent);

    // Get our ASC
    ASSAbilitySystemComponent = Cast<UASSAbilitySystemComponent>(InAbilitySystemComponent);


    //OnInited.Broadcast(); // The reason this base class can't is because it would be done in the Super call and wouldn't be done after the subclass initialization. =@REVIEW MARKER@= Broadcast this at the end of your Actor Info's InitFromActor(). TODO: find a better solution than this
}

void FASSGameplayAbilityActorInfo::SetAvatarActor(AActor* InAvatarActor)
{
    Super::SetAvatarActor(InAvatarActor);


}

void FASSGameplayAbilityActorInfo::ClearActorInfo()
{
    Super::ClearActorInfo();


    ASSAbilitySystemComponent = nullptr;
}
