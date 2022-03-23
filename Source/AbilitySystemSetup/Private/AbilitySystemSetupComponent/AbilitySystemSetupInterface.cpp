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
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!IsValid(ASC))
	{
		return;
	}

	if (UASSAbilitySystemBlueprintLibrary::GetAttributeSet<UAS_Health>(ASC) == nullptr)
	{
		HealthAttributeSet->Rename(nullptr, this);
		ASC->AddAttributeSetSubobject(HealthAttributeSet);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s() Failed to add HealthAttributeSet - a UAS_Health has already been added to the Character's ASC. Skipped adding this Attribute Set."), ANSI_TO_TCHAR(__FUNCTION__));
	}
}

void IAbilitySystemSetupInterface::GiveStartingAbilities()
{
	//Super::GiveStartingAbilities();
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!IsValid(ASC))
	{
		return;
	}

	// NOTE: No need to pass in an InputID since our subclassed ASC sets it in its overrided OnGiveAbility()
	CharacterJumpAbilitySpecHandle = ASC->GiveAbility(FGameplayAbilitySpec(CharacterJumpAbilityTSub, /*GetLevel()*/1, -1, this));
	CharacterCrouchAbilitySpecHandle = ASC->GiveAbility(FGameplayAbilitySpec(CharacterCrouchAbilityTSub, /*GetLevel()*/1, -1, this));
	CharacterRunAbilitySpecHandle = ASC->GiveAbility(FGameplayAbilitySpec(CharacterRunAbilityTSub, /*GetLevel()*/1, -1, this));
}
#endif
