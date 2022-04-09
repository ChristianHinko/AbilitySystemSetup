// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySet.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemSetup/Private/Utilities/ASSLogCategories.h"



//////////////////////////////////
/// UAbilitySystemGrantHandles
//////////////////////////////////

UAbilitySystemGrantHandles::UAbilitySystemGrantHandles(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UAbilitySystemGrantHandles::RemoveFromAbilitySystemComponent(UAbilitySystemComponent* ASC)
{
	if (!IsValid(ASC))
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Tried to remove Ability Set from %s but ASC was NULL. Returning and doing nothing"), ANSI_TO_TCHAR(__FUNCTION__), *GetNameSafe(ASC));
		return;
	}
	if (ASC->IsOwnerActorAuthoritative() == false)
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Tried to remove Ability Set from %s without authority. Returning and doing nothing"), ANSI_TO_TCHAR(__FUNCTION__), *GetNameSafe(ASC));
		return;
	}


	for (const FGameplayAbilitySpecHandle& SpecHandle : AbilitySpecHandles)
	{
		if (SpecHandle.IsValid())
		{
			ASC->ClearAbility(SpecHandle);
		}
	}

	for (const FActiveGameplayEffectHandle& ActiveHandle : ActiveEffectHandles)
	{
		if (ActiveHandle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(ActiveHandle);
		}
	}

	for (UAttributeSet* AttributeSet : GrantedAttributeSets)
	{
		ASC->GetSpawnedAttributes_Mutable().Remove(AttributeSet);
	}

	ASC->ForceReplication();

	// TODO: right now this thing assumes you will not re-use our granted stuff
	AbilitySpecHandles.Reset();
	ActiveEffectHandles.Reset();
	GrantedAttributeSets.Reset();
}


//////////////////////////////////
/// UAbilitySystemGrantSet
//////////////////////////////////

UAbilitySystemGrantSet::UAbilitySystemGrantSet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UAbilitySystemGrantSet::GrantToAbilitySystemComponent(UAbilitySystemComponent* ASC, UObject* SourceObject, UAbilitySystemGrantHandles*& OutGrantHandles) const
{
	if (!IsValid(ASC))
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Tried to grant Ability Set to %s but ASC was NULL. Returning and doing nothing"), ANSI_TO_TCHAR(__FUNCTION__), *GetNameSafe(ASC));
		return;
	}
	if (ASC->IsOwnerActorAuthoritative() == false)
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Tried to grant Ability Set to %s without authority. Returning and doing nothing"), ANSI_TO_TCHAR(__FUNCTION__), *GetNameSafe(ASC));
		return;
	}


	// Grant our Gameplay Abilities
	int AbilityIndex = 0;
	for (const TSubclassOf<UGameplayAbility> AbilityClass : GrantedAbilities)
	{
		if (!IsValid(AbilityClass))
		{
			UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() GrantedAbilities[%d] on ability set [%s] is not valid."), ANSI_TO_TCHAR(__FUNCTION__), AbilityIndex, *GetNameSafe(this));
			continue;
		}

		const FGameplayAbilitySpecHandle SpecHandle = ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, /*, GetLevel()*/1, INDEX_NONE, SourceObject));

		if (OutGrantHandles)
		{
			OutGrantHandles->AbilitySpecHandles.Add(SpecHandle);
		}

		++AbilityIndex;
	}

	// Grant our Gameplay Effects
	int EffectIndex = 0;
	for (const TSubclassOf<UGameplayEffect> EffectClass : GrantedEffects)
	{
		if (!IsValid(EffectClass))
		{
			UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() GrantedEffects[%d] on ability set [%s] is not valid."), ANSI_TO_TCHAR(__FUNCTION__), EffectIndex, *GetNameSafe(this));
			continue;
		}

		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		ContextHandle.AddSourceObject(SourceObject);
		const FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectToSelf(EffectClass.GetDefaultObject(), /*, GetLevel()*/1, ContextHandle);

		if (OutGrantHandles)
		{
			OutGrantHandles->ActiveEffectHandles.Add(ActiveHandle);
		}

		++EffectIndex;
	}

	// Grant our Attribute Sets
	int AttributeSetIndex = 0;
	for (const TSubclassOf<UGameplayEffect> AttributeSetClass : GrantedAttributeSets)
	{
		if (!IsValid(AttributeSetClass))
		{
			UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() GrantedAttributeSets[%d] on ability set [%s] is not valid."), ANSI_TO_TCHAR(__FUNCTION__), AttributeSetIndex, *GetNameSafe(this));
			continue;
		}

		// TODO: right now this thing assumes you will not re-use Attribute Set instances
		UAttributeSet* NewAttributeSet = NewObject<UAttributeSet>(SourceObject, AttributeSetClass);
		NewAttributeSet->Rename(nullptr, SourceObject);
		ASC->AddAttributeSetSubobject(NewAttributeSet);

		if (OutGrantHandles)
		{
			OutGrantHandles->GrantedAttributeSets.Add(NewAttributeSet);
		}

		++AttributeSetIndex;
	}

	ASC->ForceReplication();
}
