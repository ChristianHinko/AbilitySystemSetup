
// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Types/ASSAbilitySet.h"

#include "AbilitySystemComponent.h"



///////////////////////////////////////
/// FASSAbilitySetGrantedHandles
///////////////////////////////////////

void FASSAbilitySetGrantedHandles::RemoveFromAbilitySystemComponent()
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
				AbilitySystemComponent->RemoveSpawnedAttribute(AttributeSet);
			}

			AbilitySystemComponent->ForceReplication();
		}
		else
		{
			UE_LOG(LogASSAbilitySet, Error, TEXT("%s() Tried to remove granted sets from %s without authority"), ANSI_TO_TCHAR(__FUNCTION__), *(AbilitySystemComponent->GetName()));
		}
	}
	else
	{
		UE_LOG(LogASSAbilitySet, Error, TEXT("%s() Tried to remove granted sets from %s but AbilitySystemComponent was NULL"), ANSI_TO_TCHAR(__FUNCTION__), *(AbilitySystemComponent->GetName()));
	}

	// Empty everything
	AbilitySystemComponent = nullptr;
	AbilitySpecHandles.Reset();
	ActiveEffectHandles.Reset();
	GrantedAttributeSets.Reset();
}






///////////////////////////////////////
/// UASSAbilitySet
///////////////////////////////////////

void UASSAbilitySet::GrantToAbilitySystemComponent(UAbilitySystemComponent* InASC, UObject* InSourceObject, FASSAbilitySetGrantedHandles& OutGrantedHandles) const
{
	if (!IsValid(InASC))
	{
		UE_LOG(LogASSAbilitySet, Error, TEXT("%s() Tried to grant but ASC was NULL. Returning and doing nothing"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}
	if (InASC->IsOwnerActorAuthoritative() == false)
	{
		UE_LOG(LogASSAbilitySet, Error, TEXT("%s() Tried to grant to %s without authority. Returning and doing nothing"), ANSI_TO_TCHAR(__FUNCTION__), *(InASC->GetName()));
		return;
	}


	// Inject the ASC
	OutGrantedHandles.AbilitySystemComponent = InASC;

	// Grant our Attribute Sets
	int AttributeSetIndex = 0;
	for (const TSubclassOf<UAttributeSet> AttributeSetClass : GrantedAttributeSets)
	{
		if (!IsValid(AttributeSetClass))
		{
			UE_LOG(LogASSAbilitySet, Error, TEXT("%s() GrantedAttributeSets[%d] on ASSAbilitySet [%s] is not valid."), ANSI_TO_TCHAR(__FUNCTION__), AttributeSetIndex, *GetName());
			continue;
		}

		UAttributeSet* NewAttributeSet = NewObject<UAttributeSet>(InASC->GetOwnerActor(), AttributeSetClass);
		InASC->AddAttributeSetSubobject(NewAttributeSet);

		OutGrantedHandles.GrantedAttributeSets.Add(NewAttributeSet);

		++AttributeSetIndex;
	}

	InASC->ForceReplication();

	// Grant our Gameplay Effects
	int EffectIndex = 0;
	for (const TSubclassOf<UGameplayEffect> EffectClass : GrantedEffects)
	{
		if (!IsValid(EffectClass))
		{
			UE_LOG(LogASSAbilitySet, Error, TEXT("%s() GrantedEffects[%d] on ASSAbilitySet [%s] is not valid."), ANSI_TO_TCHAR(__FUNCTION__), EffectIndex, *GetName());
			continue;
		}

		FGameplayEffectContextHandle ContextHandle = InASC->MakeEffectContext();
		ContextHandle.AddSourceObject(InSourceObject);
		const FActiveGameplayEffectHandle ActiveHandle = InASC->ApplyGameplayEffectToSelf(EffectClass.GetDefaultObject(), /*, GetLevel()*/1, ContextHandle);

		OutGrantedHandles.ActiveEffectHandles.Add(ActiveHandle);

		++EffectIndex;
	}

	// Grant our Gameplay Abilities
	int AbilityIndex = 0;
	for (const TSubclassOf<UGameplayAbility> AbilityClass : GrantedAbilities)
	{
		if (!IsValid(AbilityClass))
		{
			UE_LOG(LogASSAbilitySet, Error, TEXT("%s() GrantedAbilities[%d] on ASSAbilitySet [%s] is not valid."), ANSI_TO_TCHAR(__FUNCTION__), AbilityIndex, *GetName());
			continue;
		}

		const FGameplayAbilitySpecHandle SpecHandle = InASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, /*, GetLevel()*/1, INDEX_NONE, InSourceObject));

		OutGrantedHandles.AbilitySpecHandles.Add(SpecHandle);

		++AbilityIndex;
	}
}
