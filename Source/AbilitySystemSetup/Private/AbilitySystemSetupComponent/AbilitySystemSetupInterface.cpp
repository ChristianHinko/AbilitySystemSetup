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


	if (IsValid(MyAttributeSet) && UASSAbilitySystemBlueprintLibrary::GetAttributeSet<UAS_MyAttributeSet>(GetAbilitySystemComponent()) == nullptr)
	{
		MyAttributeSet->Rename(nullptr, this);
		GetAbilitySystemComponent()->AddAttributeSetSubobject(MyAttributeSet);
	}
	else
	{
		UE_CLOG((GetLocalRole() == ROLE_Authority), LogTemp, Warning, TEXT("%s() MyAttributeSet was either NULL or already added to the character's ASC. Character: %s"), ANSI_TO_TCHAR(__FUNCTION__), *GetName());
	}

	if (IsValid(MyOtherAttributeSet) && UASSAbilitySystemBlueprintLibrary::GetAttributeSet<UAS_MyOtherAttributeSet>(GetAbilitySystemComponent()) == nullptr)
	{
		MyOtherAttributeSet->Rename(nullptr, this);
		GetAbilitySystemComponent()->AddAttributeSetSubobject(MyOtherAttributeSet);
	}
	else
	{
		UE_CLOG((GetLocalRole() == ROLE_Authority), LogTemp, Warning, TEXT("%s() MyOtherAttributeSet was either NULL or already added to the character's ASC. Character: %s"), ANSI_TO_TCHAR(__FUNCTION__), *GetName());
	}
}

void IAbilitySystemSetupInterface::GiveStartingAbilities()
{
	//Super::GiveStartingAbilities();


	MyAbilitySpecHandle = GetAbilitySystemComponent()->GiveAbility(FGameplayAbilitySpec(MyAbilityTSub, /*GetLevel()*/1, -1, this));
	MyOtherAbilitySpecHandle = GetAbilitySystemComponent()->GiveAbility(FGameplayAbilitySpec(MyOtherAbilityTSub, /*GetLevel()*/1, -1, this));
}
#endif
