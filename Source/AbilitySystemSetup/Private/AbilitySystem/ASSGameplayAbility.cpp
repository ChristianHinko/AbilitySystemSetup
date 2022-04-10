// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ASSGameplayAbility.h"

#include "AbilitySystemSetup/Private/Utilities/ASSLogCategories.h"



UASSGameplayAbility::UASSGameplayAbility()
{
	AbilityInputID = 0; // Unset
	bActivateOnGiveAbility = false;

	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bServerRespectsRemoteAbilityCancellation = false;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnlyTermination;
}


void UASSGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	TryCallOnAvatarSetOnPrimaryInstance
	Super::OnAvatarSet(ActorInfo, Spec);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (AbilityInputID == 0)
	{
		UE_LOG(LogGameplayAbilitySetup, Fatal, TEXT("%s() Ability implementor forgot to set an AbilityInputID in the Ability's constructor. Go back and set it so we get Ability input events"), ANSI_TO_TCHAR(__FUNCTION__));
	}
	if (AbilityTags.IsEmpty())
	{
		UE_LOG(LogGameplayAbilitySetup, Fatal, TEXT("%s() Ability implementor forgot to assign an Ability Tag to this ability. We try to enforce activating abilities by tag for organization reasons"), ANSI_TO_TCHAR(__FUNCTION__));
	}
#endif // !(UE_BUILD_SHIPPING || UE_BUILD_TEST)

	// Epic's comment: Projects may want to initiate passives or do other "BeginPlay" type of logic here.
}
void UASSGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (bActivateOnGiveAbility)
	{
		if (ActorInfo)
		{
			UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
			if (IsValid(ASC))
			{
				ASC->TryActivateAbility(Spec.Handle);
			}
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
			UE_LOG(LogGameplayAbilitySetup, Error, TEXT("%s() Ability activated but the client has no valid prediction key"), ANSI_TO_TCHAR(__FUNCTION__));
		}
	}


	


	//BEGIN Copied from Super (for Blueprint support)
	if (bHasBlueprintActivate)
	{
		// A Blueprinted ActivateAbility function must call CommitAbility somewhere in its execution chain.
		K2_ActivateAbility();
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
	}
	//END Copied from Super (for Blueprint support)
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
