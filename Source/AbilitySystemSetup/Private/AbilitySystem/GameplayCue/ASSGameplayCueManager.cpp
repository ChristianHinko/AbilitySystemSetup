// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayCue/ASSGameplayCueManager.h"



bool UASSGameplayCueManager::ShouldAsyncLoadRuntimeObjectLibraries() const
{
	// Idea: maybe check this machines memory specs to determine this ret val


	return false;
}

void UASSGameplayCueManager::FlushPendingCues()
{
	Super::FlushPendingCues();


}
