// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Types/AbilitySet.h"

#include "AbilitySystemComponent.h"



///////////////////////////////////////
/// FAbilitySetGrantedHandles
///////////////////////////////////////

void FAbilitySetGrantedHandles::RemoveFromAbilitySystemComponent()
{
	if (AbilitySystemComponent.IsValid())
	{
		if (AbilitySystemComponent->IsOwnerActorAuthoritative())
		{
			// Clear Abilities
			for (const FGameplayAbilitySpecHandle& SpecHandle : AbilitySpecHandles)
			{
				if (SpecHandle.IsValid())
				{
					AbilitySystemComponent->ClearAbility(SpecHandle);
				}
			}

			// Remove Effects
			for (const FActiveGameplayEffectHandle& ActiveHandle : ActiveEffectHandles)
			{
				if (ActiveHandle.IsValid())
				{
					AbilitySystemComponent->RemoveActiveGameplayEffect(ActiveHandle);
				}
			}

			// Remove Attribute Sets
			for (UAttributeSet* AttributeSet : GrantedAttributeSets)
			{
				AbilitySystemComponent->GetSpawnedAttributes_Mutable().Remove(AttributeSet);
			}

			AbilitySystemComponent->ForceReplication();
		}
		else
		{
			UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Tried to remove granted sets from %s without authority"), ANSI_TO_TCHAR(__FUNCTION__), *(AbilitySystemComponent->GetName()));
		}
	}
	else
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Tried to remove granted sets from %s but AbilitySystemComponent was NULL"), ANSI_TO_TCHAR(__FUNCTION__), *(AbilitySystemComponent->GetName()));
	}

	// Empty everything
	AbilitySystemComponent = nullptr;
	AbilitySpecHandles.Reset();
	ActiveEffectHandles.Reset();
	GrantedAttributeSets.Reset();
}






///////////////////////////////////////
/// UAbilitySet
///////////////////////////////////////

void UAbilitySet::GrantToAbilitySystemComponent(UAbilitySystemComponent* ASC, UObject* SourceObject, FAbilitySetGrantedHandles& OutGrantedHandles) const
{
	if (!IsValid(ASC))
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Tried to grant but ASC was NULL. Returning and doing nothing"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}
	if (ASC->IsOwnerActorAuthoritative() == false)
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Tried to grant to %s without authority. Returning and doing nothing"), ANSI_TO_TCHAR(__FUNCTION__), *(ASC->GetName()));
		return;
	}

	
	// Inject the ASC
	OutGrantedHandles.AbilitySystemComponent = ASC;

	// Grant our Attribute Sets
	int AttributeSetIndex = 0;
	for (const TSubclassOf<UAttributeSet> AttributeSetClass : GrantedAttributeSets)
	{
		if (!IsValid(AttributeSetClass))
		{
			UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() GrantedAttributeSets[%d] on AbilitySet [%s] is not valid."), ANSI_TO_TCHAR(__FUNCTION__), AttributeSetIndex, *GetName());
			continue;
		}

		UAttributeSet* NewAttributeSet = NewObject<UAttributeSet>(SourceObject, AttributeSetClass);
		NewAttributeSet->Rename(nullptr, SourceObject);
		ASC->AddAttributeSetSubobject(NewAttributeSet);

		OutGrantedHandles.GrantedAttributeSets.Add(NewAttributeSet);

		++AttributeSetIndex;
	}

	ASC->ForceReplication();

	// Grant our Gameplay Effects
	int EffectIndex = 0;
	for (const TSubclassOf<UGameplayEffect> EffectClass : GrantedEffects)
	{
		if (!IsValid(EffectClass))
		{
			UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() GrantedEffects[%d] on AbilitySet [%s] is not valid."), ANSI_TO_TCHAR(__FUNCTION__), EffectIndex, *GetName());
			continue;
		}

		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		ContextHandle.AddSourceObject(SourceObject);
		const FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectToSelf(EffectClass.GetDefaultObject(), /*, GetLevel()*/1, ContextHandle);

		OutGrantedHandles.ActiveEffectHandles.Add(ActiveHandle);

		++EffectIndex;
	}

	// Grant our Gameplay Abilities
	int AbilityIndex = 0;
	for (const TSubclassOf<UGameplayAbility> AbilityClass : GrantedAbilities)
	{
		if (!IsValid(AbilityClass))
		{
			UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() GrantedAbilities[%d] on AbilitySet [%s] is not valid."), ANSI_TO_TCHAR(__FUNCTION__), AbilityIndex, *GetName());
			continue;
		}

		const FGameplayAbilitySpecHandle SpecHandle = ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, /*, GetLevel()*/1, INDEX_NONE, SourceObject));

		OutGrantedHandles.AbilitySpecHandles.Add(SpecHandle);

		++AbilityIndex;
	}
}
