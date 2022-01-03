// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Types/ASSGameplayAbilityTypes.h"

#include "AbilitySystem/ASSAbilitySystemComponent.h"



FASSGameplayAbilityActorInfo::FASSGameplayAbilityActorInfo()
{

}


void FASSGameplayAbilityActorInfo::InitFromActor(AActor* InOwnerActor, AActor* InAvatarActor, UAbilitySystemComponent* InAbilitySystemComponent)
{
	ASSInitFromActor(InOwnerActor, InAvatarActor, InAbilitySystemComponent);
    OnInitted.Broadcast();
}
void FASSGameplayAbilityActorInfo::ASSInitFromActor(AActor* InOwnerActor, AActor* InAvatarActor, UAbilitySystemComponent* InAbilitySystemComponent)
{
	Super::InitFromActor(InOwnerActor, InAvatarActor, InAbilitySystemComponent);

    // Get our ASC
    ASSAbilitySystemComponent = Cast<UASSAbilitySystemComponent>(InAbilitySystemComponent);
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
