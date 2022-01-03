// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ASSGameplayAbility.h"

#include "Utilities/ASSLogCategories.h"



UASSGameplayAbility::UASSGameplayAbility()
{
	AbilityInputID = 0;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bServerRespectsRemoteAbilityCancellation = false;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnlyTermination;
}


void UASSGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	if (AbilityInputID == 0)
	{
		UE_LOG(LogGameplayAbilitySetup, Fatal, TEXT("%s()  Ability implementor forgot to set an AbilityInputID in the ability's constructor. Go back and set it so our grant ability calls know what input id to give it"), *FString(__FUNCTION__));
	}

	TryCallOnAvatarSetOnPrimaryInstance
	Super::OnAvatarSet(ActorInfo, Spec);

	if (ActivateAbilityOnGrant && ActorInfo)
	{
		if (ActorInfo->AbilitySystemComponent.Get())
		{
			bool ActivatedAbilitySucessfully = ActorInfo->AbilitySystemComponent.Get()->TryActivateAbility(Spec.Handle);
		}
	}
}

bool UASSGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return true;
}

void UASSGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalPredicted)
	{
		// Const cast is a red flag. 
		FPredictionKey* Key = const_cast<FPredictionKey*>(&ActivationInfo.GetActivationPredictionKey());
		Key->NewRejectedDelegate().BindUObject(this, &UASSGameplayAbility::OnActivationPredictionKeyRejected);

		if (!HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))	// If we are a client without a valid prediction key
		{
			UE_LOG(LogGameplayAbilitySetup, Error, TEXT("%s() Ability activated but the client has no valid prediction key"), *FString(__FUNCTION__));
		}
	}


	


#pragma region Copied from Super (Blueprint Support)
	if (bHasBlueprintActivate)
	{
		// A Blueprinted ActivateAbility function must call CommitAbility somewhere in its execution chain.
		K2_ActivateAbility();
		return;
	}
	else if (bHasBlueprintActivateFromEvent)
	{
		if (TriggerEventData)
		{
			// A Blueprinted ActivateAbility function must call CommitAbility somewhere in its execution chain.
			K2_ActivateAbilityFromEvent(*TriggerEventData);
		}
		else
		{
			UE_LOG(LogAbilitySystem, Warning, TEXT("Ability %s expects event data but none is being supplied. Use Activate Ability instead of Activate Ability From Event."), *GetName());
			bool bReplicateEndAbility = false;
			bool bWasCancelled = true;
			EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
		}
		return;
	}
#pragma endregion
}

void UASSGameplayAbility::ExternalEndAbility()
{
	check(CurrentActorInfo);

	const bool bReplicateEndAbility = true;
	const bool bWasCancelled = false;
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), bReplicateEndAbility, bWasCancelled);
}

//void UASSGameplayAbility::OnCurrentAbilityPredictionKeyRejected()
//{
//	/*UKismetSystemLibrary::PrintString(this, "Prediction Key rejected ", true, false, FLinearColor::Red);
//
//	if (PKey == CurrentActivationInfo.GetActivationPredictionKey())
//	{
//		OnActivationPredictionKeyRejected();
//	}*/
//}

void UASSGameplayAbility::OnActivationPredictionKeyRejected()
{

}
