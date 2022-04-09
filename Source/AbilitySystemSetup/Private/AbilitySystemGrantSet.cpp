// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystemGrantSet.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemSetup/Private/Utilities/ASSLogCategories.h"



///////////////////////////////////////
/// FAbilitySystemGrantHandles
///////////////////////////////////////

FAbilitySystemGrantHandles::FAbilitySystemGrantHandles()
{

}

void FAbilitySystemGrantHandles::RemoveFromAbilitySystemComponent()
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

void FAbilitySystemGrantHandles::TransferTo(UAbilitySystemComponent* NewASC)
{
	if (AbilitySystemComponent.IsValid() == false)
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Tried to transfer granted sets but our AbilitySystemComponent was NULL. Called RemoveFromAbilitySystemComponent() and returned"), ANSI_TO_TCHAR(__FUNCTION__));
		RemoveFromAbilitySystemComponent();
		return;
	}
	if (AbilitySystemComponent->IsOwnerActorAuthoritative() == false)
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Tried to transfer granted sets without authority. Returning and doing nothing"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}
	if (!IsValid(NewASC))
	{
		UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() Tried to transfer granted sets but NewASC was NULL. Returning and doing nothing"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	// Set AbilitySystemComponent
	UAbilitySystemComponent* const OldAbilitySystemComponent = AbilitySystemComponent.Get();
	AbilitySystemComponent = NewASC;


	// Transfer our Attribute Sets
	for (UAttributeSet* AttributeSet : GrantedAttributeSets)
	{
		if (!IsValid(AttributeSet))
		{
			UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() AttributeSet was NULL when transfering to %s. Skipping this Attribute Set"), ANSI_TO_TCHAR(__FUNCTION__), *(NewASC->GetName()));
			continue;
		}

		// Remove it from the OldAbilitySystemComponent
		OldAbilitySystemComponent->GetSpawnedAttributes_Mutable().Remove(AttributeSet);
		// Add it to the NewASC
		NewASC->AddAttributeSetSubobject(AttributeSet);
	}

	OldAbilitySystemComponent->ForceReplication();
	NewASC->ForceReplication();

	// Transfer our active Effects
	TArray<FActiveGameplayEffectHandle> ActiveEffectHandlesCopy = ActiveEffectHandles;
	ActiveEffectHandles.Empty();
	for (const FActiveGameplayEffectHandle& ActiveHandle : ActiveEffectHandlesCopy)
	{
		if (ActiveHandle.IsValid() == false)
		{
			continue;
		}

		// Apply it to the NewASC
		if (const FActiveGameplayEffect* ActiveEffect = OldAbilitySystemComponent->GetActiveGameplayEffect(ActiveHandle))
		{
			// TODO: Should we copy more values from the old Context?
			FGameplayEffectContextHandle ContextHandle = NewASC->MakeEffectContext();
			ContextHandle.AddSourceObject(ActiveEffect->Spec.GetEffectContext().GetSourceObject());

			const FActiveGameplayEffectHandle NewActiveHandle = NewASC->ApplyGameplayEffectToSelf(ActiveEffect->Spec.Def, ActiveEffect->Spec.GetLevel(), ContextHandle);
			ActiveEffectHandles.Add(NewActiveHandle);
		}

		// Remove it from the OldAbilitySystemComponent
		OldAbilitySystemComponent->RemoveActiveGameplayEffect(ActiveHandle);
	}


	// Transfer our Abilities
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandlesCopy = AbilitySpecHandles;
	AbilitySpecHandles.Empty();
	for (const FGameplayAbilitySpecHandle& SpecHandle : AbilitySpecHandlesCopy)
	{
		if (SpecHandle.IsValid() == false)
		{
			continue;
		}

		// Give it to the NewASC
		for (const FGameplayAbilitySpec& Spec : OldAbilitySystemComponent->GetActivatableAbilities())
		{
			check(Spec.Handle.IsValid());
			if (Spec.Handle == SpecHandle)
			{
				// TODO: Should we copy more values from the old Spec?
				const FGameplayAbilitySpecHandle NewSpecHandle = NewASC->GiveAbility(FGameplayAbilitySpec(Spec.Ability, Spec.Level, Spec.InputID, Spec.SourceObject));
				AbilitySpecHandles.Add(NewSpecHandle);
			}
		}

		// Clear it from the OldAbilitySystemComponent
		OldAbilitySystemComponent->ClearAbility(SpecHandle);
	}

	UE_LOG(LogAbilitySystemSetup, Verbose, TEXT("%s() Transfered from [%s] to [%s]"), ANSI_TO_TCHAR(__FUNCTION__), *(OldAbilitySystemComponent->GetName()), *(NewASC->GetName()));
}


///////////////////////////////////////
/// UAbilitySystemGrantSet
///////////////////////////////////////

UAbilitySystemGrantSet::UAbilitySystemGrantSet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UAbilitySystemGrantSet::GrantToAbilitySystemComponent(UAbilitySystemComponent* ASC, UObject* SourceObject, FAbilitySystemGrantHandles& OutGrantHandles) const
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
	OutGrantHandles.AbilitySystemComponent = ASC;

	// Grant our Attribute Sets
	int AttributeSetIndex = 0;
	for (const TSubclassOf<UAttributeSet> AttributeSetClass : GrantedAttributeSets)
	{
		if (!IsValid(AttributeSetClass))
		{
			UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() GrantedAttributeSets[%d] on grant set [%s] is not valid."), ANSI_TO_TCHAR(__FUNCTION__), AttributeSetIndex, *GetName());
			continue;
		}

		UAttributeSet* NewAttributeSet = NewObject<UAttributeSet>(SourceObject, AttributeSetClass);
		NewAttributeSet->Rename(nullptr, SourceObject);
		ASC->AddAttributeSetSubobject(NewAttributeSet);

		OutGrantHandles.GrantedAttributeSets.Add(NewAttributeSet);

		++AttributeSetIndex;
	}

	ASC->ForceReplication();

	// Grant our Gameplay Effects
	int EffectIndex = 0;
	for (const TSubclassOf<UGameplayEffect> EffectClass : GrantedEffects)
	{
		if (!IsValid(EffectClass))
		{
			UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() GrantedEffects[%d] on grant set [%s] is not valid."), ANSI_TO_TCHAR(__FUNCTION__), EffectIndex, *GetName());
			continue;
		}

		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		ContextHandle.AddSourceObject(SourceObject);
		const FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectToSelf(EffectClass.GetDefaultObject(), /*, GetLevel()*/1, ContextHandle);

		OutGrantHandles.ActiveEffectHandles.Add(ActiveHandle);

		++EffectIndex;
	}

	// Grant our Gameplay Abilities
	int AbilityIndex = 0;
	for (const TSubclassOf<UGameplayAbility> AbilityClass : GrantedAbilities)
	{
		if (!IsValid(AbilityClass))
		{
			UE_LOG(LogAbilitySystemSetup, Error, TEXT("%s() GrantedAbilities[%d] on grant set [%s] is not valid."), ANSI_TO_TCHAR(__FUNCTION__), AbilityIndex, *GetName());
			continue;
		}

		const FGameplayAbilitySpecHandle SpecHandle = ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, /*, GetLevel()*/1, INDEX_NONE, SourceObject));

		OutGrantHandles.AbilitySpecHandles.Add(SpecHandle);

		++AbilityIndex;
	}
}
