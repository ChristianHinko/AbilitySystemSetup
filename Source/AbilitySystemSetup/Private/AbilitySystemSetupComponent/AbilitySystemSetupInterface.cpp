// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystemSetupComponent/AbilitySystemSetupInterface.h"



/////////////////////////////////////////////////////////////////////////////
/// 
/// EXAMPLE USAGE OF AbilitySystemSetupComponent EVENTS
/// 

#if 0
void IAbilitySystemSetupInterface::RegisterAttributeSets()
{
	//Super::RegisterAttributeSets();


	if (UASSAbilitySystemBlueprintLibrary::GetAttributeSet<UAS_Health>(GetAbilitySystemComponent()) == nullptr)
	{
		HealthAttributeSet->Rename(nullptr, this);
		GetAbilitySystemComponent()->AddAttributeSetSubobject(HealthAttributeSet);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s() Failed to add HealthAttributeSet - a UAS_Health has already been added to the Character's ASC. Skipped adding this Attribute Set."), ANSI_TO_TCHAR(__FUNCTION__));
	}
}

void IAbilitySystemSetupInterface::GiveStartingAbilities()
{
	//Super::GiveStartingAbilities();


	// NOTE: No need to pass in an InputID since our subclassed ASC sets it in its overrided OnGiveAbility()
	CharacterJumpAbilitySpecHandle = GetAbilitySystemComponent()->GiveAbility(FGameplayAbilitySpec(CharacterJumpAbilityTSub, /*GetLevel()*/1, -1, this));
	CharacterCrouchAbilitySpecHandle = GetAbilitySystemComponent()->GiveAbility(FGameplayAbilitySpec(CharacterCrouchAbilityTSub, /*GetLevel()*/1, -1, this));
	CharacterRunAbilitySpecHandle = GetAbilitySystemComponent()->GiveAbility(FGameplayAbilitySpec(CharacterRunAbilityTSub, /*GetLevel()*/1, -1, this));
}
#endif
