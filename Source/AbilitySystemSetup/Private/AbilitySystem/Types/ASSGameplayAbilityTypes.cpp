// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Types/ASSGameplayAbilityTypes.h"

#include "AbilitySystem/ASSAbilitySystemComponent.h"
#include "BlueprintFunctionLibraries/GCBlueprintFunctionLibrary_ActorHelpers.h"



void FASSGameplayAbilityActorInfo::InitFromActor(AActor* InOwnerActor, AActor* InAvatarActor, UAbilitySystemComponent* InAbilitySystemComponent)
{
	Super::InitFromActor(InOwnerActor, InAvatarActor, InAbilitySystemComponent);


	// Get the Controller
	Controller = UGCBlueprintFunctionLibrary_ActorHelpers::GetTypedOwnerIncludingSelfCasted<AController>(InOwnerActor);

	// Get our ASC
	ASSAbilitySystemComponent = Cast<UASSAbilitySystemComponent>(InAbilitySystemComponent);
}

void FASSGameplayAbilityActorInfo::ClearActorInfo()
{
	Super::ClearActorInfo();

	Controller = nullptr;
	ASSAbilitySystemComponent = nullptr;
}
