// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ASSAbilitySystemComponent.h"

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
#include "AbilitySystem/Types/ASSAbilityInputID.h"
#include "ASSDeveloperSettings_AbilitySystemSetup.h"
#include "GameFramework/InputSettings.h"
#endif // !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
#include "AbilitySystem/ASSGameplayAbility.h"



UASSAbilitySystemComponent::UASSAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Some projects may want to use GAS' automatic input binding by default. Set these to false if you do		=@REVIEW MARKER@=
	bDoNotAutoActivateFromGASBindings = true;
	// If false, this one will automatially trigger confirmation / cancellation for target actors
	bDoNotAutoConfirmAndCancelFromGASBindings = true;


	/** The linked Anim Instance that this component will play montages in. Use NAME_None for the main anim instance. (Havn't explored this much yet) */
	AffectedAnimInstanceTag = NAME_None;
}

void UASSAbilitySystemComponent::InitializeComponent()
{
	Super::InitializeComponent();

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST) // editor only section to enforce good workflow
	const UASSDeveloperSettings_AbilitySystemSetup* AbilitySystemSetupDeveloperSettings = GetDefault<UASSDeveloperSettings_AbilitySystemSetup>();



	// Ensure this UENUM exists! (Ensure that the game has implemented their AbilityInputID enum that they specified in the plugin settings)
	const UEnum* AbilityInputIDEnum = FindObject<UEnum>(ANY_PACKAGE, *(AbilitySystemSetupDeveloperSettings->AbilityInputIDEnumName));
	if (AbilityInputIDEnum)
	{
		if (AbilityInputIDEnum->GetValueByIndex(0) != static_cast<uint8>(EASSAbilityInputID::MAX))
		{
			const FName MaxEnumeratorName = StaticEnum<EASSAbilityInputID>()->GetNameByIndex(StaticEnum<EASSAbilityInputID>()->NumEnums() - 1);
			UE_LOG(LogAbilitySystemInputEnumMappingsSafetyChecks, Error, TEXT("Your ``%s`` UEnum is not extending the base ``%s`` enum. Go to your %s definition and make sure your first enumerator's value starts at %s."), *(AbilitySystemSetupDeveloperSettings->AbilityInputIDEnumName), *(StaticEnum<EASSAbilityInputID>()->GetFName().ToString()), *(AbilitySystemSetupDeveloperSettings->AbilityInputIDEnumName), *(MaxEnumeratorName.ToString()));
			check(0);
		}
	}
	else
	{
		UE_LOG(LogAbilitySystemInputEnumMappingsSafetyChecks, Error, TEXT("The UEnum ``%s`` does not exist. Ensure correct spelling for the name of your AbilityInputID Enum and make sure it is a UENUM so we can find it!"), *(AbilitySystemSetupDeveloperSettings->AbilityInputIDEnumName));
		check(0);
	}




	// Our input action names (includes speech mappings)
	TArray<FName> ActionNames;
	UInputSettings::GetInputSettings()->GetActionNames(ActionNames);

	const FName& ConfirmTargetInputActionName = FName(AbilitySystemSetupDeveloperSettings->ConfirmTargetInputActionName);
	const FName& CancelTargetInputActionName = FName(AbilitySystemSetupDeveloperSettings->CancelTargetInputActionName);

	// Ensure that the Confirm and Cancel Target input actions exist! (Check to see if ConfirmTargetInputActionName and CancelTargetInputActionName in the plugin settings are real inputs)
	if (ActionNames.Contains(ConfirmTargetInputActionName) == false)
	{
		UE_LOG(LogAbilitySystemInputEnumMappingsSafetyChecks, Error, TEXT("The ``%s`` input action does not exist in your Action Mappings list in DefaultInput.ini - Ensure correct spelling for the name of your ConfirmTarget input action!"), *(AbilitySystemSetupDeveloperSettings->ConfirmTargetInputActionName));
		check(0);
	}
	if (ActionNames.Contains(CancelTargetInputActionName) == false)
	{
		UE_LOG(LogAbilitySystemInputEnumMappingsSafetyChecks, Error, TEXT("The ``%s`` input action does not exist in your Action Mappings list in DefaultInput.ini - Ensure correct spelling for the name of your CancelTarget input action!"), *(AbilitySystemSetupDeveloperSettings->CancelTargetInputActionName));
		check(0);
	}


	// Ensure that the enumerators exist in the Action Mappings!
	for (int32 i = 0; i < AbilityInputIDEnum->NumEnums(); ++i)
	{
		if (AbilityInputIDEnum->ContainsExistingMax() && i == AbilityInputIDEnum->NumEnums() - 1)
		{
			// Ignore the MAX enumerator
			continue;
		}

		const FString EnumeratorNameString = AbilityInputIDEnum->GetNameStringByIndex(i);
		if (ActionNames.Contains(FName(EnumeratorNameString)) == false)
		{
			UE_LOG(LogAbilitySystemInputEnumMappingsSafetyChecks, Error, TEXT("The ``%s`` input action does not exist in your Action Mappings list in DefaultInput.ini - Ensure correct spelling for the name of your input action!"), *EnumeratorNameString);
			check(0);
		}
	}
#endif // !(UE_BUILD_SHIPPING || UE_BUILD_TEST)

}


void UASSAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!IsValid(AbilitySpec.SourceObject))
	{
		UE_LOG(LogASSAbilitySystemComponentSetup, Error, TEXT("%s() SourceObject was not valid when Ability was given. Someone must have forgotten to set it when giving the Ability"), ANSI_TO_TCHAR(__FUNCTION__));
		check(0);
	}
#endif // !(UE_BUILD_SHIPPING || UE_BUILD_TEST)

	// NOTE: I want to do this in UGameplayAbility::OnGiveAbility() instead but the given Spec is const there
	UASSGameplayAbility* ASSAbility = Cast<UASSGameplayAbility>(AbilitySpec.Ability);
	if (IsValid(ASSAbility))
	{
		//if (ASSAbility->AbilityInputID != static_cast<uint8>(EASSAbilityInputID::NoInput))
		//{
			// Take the configured InputID from the Ability
			AbilitySpec.InputID = static_cast<int32>(ASSAbility->AbilityInputID);
		//}
		//else
		//{
		//	// We are EASSAbilityInputID::NoInput
		//	AbilitySpec.InputID = INDEX_NONE;
		//}
	}
	else
	{
		UE_LOG(LogASSAbilitySystemComponentSetup, Warning, TEXT("%s() no meaningful AbilityInputID for given Ability because the UASSGameplayAbility was null"), ANSI_TO_TCHAR(__FUNCTION__));
	}


	Super::OnGiveAbility(AbilitySpec);
}

//  BEGIN Input Binding
/*
	this is the same thing as the Super exept it doesn't force the Confirm/Cancel binding to confirm/cancel the target actor directly.
	We added a bool (bDoNotAutoConfirmAndCancelFromGASBindings) - set this to false to if you want Confirm/Cancel bindings to directly confirm / cancel the target actors.
	If you set this bool to false, make sure you get rid of the LocalInputConfirm / Cancel in the OnConfirmTargetPressed / Released and OnCancelTargetPressed / Released implementations in
	the AbilitySystemCharacter so that it doesn't double fire LocalInputConfirm and LocalInputCancel
*/
void UASSAbilitySystemComponent::BindAbilityActivationToInputComponent(UInputComponent* InputComponent, FGameplayAbilityInputBinds BindInfo)
{
	UEnum* EnumBinds = BindInfo.GetBindEnum();

	SetBlockAbilityBindingsArray(BindInfo);

	for (int32 idx = 0; idx < EnumBinds->NumEnums(); ++idx)
	{
		const FString FullStr = EnumBinds->GetNameStringByIndex(idx);

		// Pressed event
		{
			FInputActionBinding AB(FName(*FullStr), IE_Pressed);
			AB.ActionDelegate.GetDelegateForManualSet().BindUObject(this, &UAbilitySystemComponent::AbilityLocalInputPressed, idx);
			InputComponent->AddActionBinding(AB);
		}

		// Released event
		{
			FInputActionBinding AB(FName(*FullStr), IE_Released);
			AB.ActionDelegate.GetDelegateForManualSet().BindUObject(this, &UAbilitySystemComponent::AbilityLocalInputReleased, idx);
			InputComponent->AddActionBinding(AB);
		}
	}

	// Bind Confirm/Cancel. Note: these have to come last!
	if (!bDoNotAutoConfirmAndCancelFromGASBindings) // =@OVERRIDED CODE MARKER@= wrapped in this if statement because original implemntation didn't have this and forced a binding to confirm / cancel
	{
		if (BindInfo.ConfirmTargetCommand.IsEmpty() == false)
		{
			FInputActionBinding AB(FName(*BindInfo.ConfirmTargetCommand), IE_Pressed);
			AB.ActionDelegate.GetDelegateForManualSet().BindUObject(this, &UAbilitySystemComponent::LocalInputConfirm);
			InputComponent->AddActionBinding(AB);
		}

		if (BindInfo.CancelTargetCommand.IsEmpty() == false)
		{
			FInputActionBinding AB(FName(*BindInfo.CancelTargetCommand), IE_Pressed);
			AB.ActionDelegate.GetDelegateForManualSet().BindUObject(this, &UAbilitySystemComponent::LocalInputCancel);
			InputComponent->AddActionBinding(AB);
		}
	}

	if (BindInfo.CancelTargetInputID >= 0)
	{
		GenericCancelInputID = BindInfo.CancelTargetInputID;
	}
	if (BindInfo.ConfirmTargetInputID >= 0)
	{
		GenericConfirmInputID = BindInfo.ConfirmTargetInputID;
	}
}

/*
										--- Our thought process here that took us few hours to decide what to do ---
													relates to AAbilitySystemCharacter::BindASCInput()
	Idk why this what the pourpose is for the first 2 if statements because they never passed even when you pass in the last 2 optional parameters for
	the FGameplayAbilityInputBinds struct for the BindAbilityActivationToInputComponent() function and put confirm and cancel inputs in the Enum we made.
	Decided to just not have this function run for confirm and cancel since nothing happens in this function for them. We did this by not including
	confirm and cancel in the GAS input enum and not setting the optional parameters for the confirm and cancel input IDs for
	BindAbilityActivationToInputComponent(). We figure this is the right way to setup the ability system since LocalInputConfirm() and LocalInputCancel()
	still get called and it prevents an extra call to this function which would have don't nothing anyways. Technically there may be importance to this
	function running for the confirm and cancel inputs since there seems to be some kind of logic in the beginning for them, but Dart does it our way
	so at least were not the only ones. Dan doesn't do it this way though but we think he should have.
*/
/*
	this is the same thing as the Super exept it doesn't force the try activate ability when a bound input is pressed.
	We added a bool (bDoNotAutoActivateFromGASBindings) - set this to false if you want to make bound input directly call try activate ability
*/
void UASSAbilitySystemComponent::AbilityLocalInputPressed(int32 InputID)
{
	// Consume the input if this InputID is overloaded with GenericConfirm/Cancel and the GenericConfim/Cancel callback is bound
	if (IsGenericConfirmInputBound(InputID))
	{
		LocalInputConfirm();
		return;
	}

	if (IsGenericCancelInputBound(InputID))
	{
		LocalInputCancel();
		return;
	}

	// ---------------------------------------------------------

	ABILITYLIST_SCOPE_LOCK();
	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.InputID == InputID)
		{
			if (Spec.Ability)
			{
				Spec.InputPressed = true;
				if (Spec.IsActive())
				{
					if (Spec.Ability->bReplicateInputDirectly && IsOwnerActorAuthoritative() == false)
					{
						ServerSetInputPressed(Spec.Handle);
					}

					AbilitySpecInputPressed(Spec);

					// Invoke the InputPressed event. This is not replicated here. If someone is listening, they may replicate the InputPressed event to the server.
					InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, Spec.ActivationInfo.GetActivationPredictionKey());
				}
				else
				{
					if (!bDoNotAutoActivateFromGASBindings) // =@OVERRIDED CODE MARKER@= wrapped in this if statement because original implemntation would just TryActivateAbility() in this else statement
					{
						// Ability is not active, so try to activate it
						TryActivateAbility(Spec.Handle);
					}
				}
			}
		}
	}
}
//  END Input Binding


void UASSAbilitySystemComponent::FullReset()
{
	//	Stop ASC from doing things
	DestroyActiveState();


	if (IsOwnerActorAuthoritative())
	{
		//	Ungive abilities. Will remove all abilitity tags/blocked bindings as well
		ClearAllAbilities();

		//	Clear Effects. Will remove all given tags and cues as well
		for (const FActiveGameplayEffect& Effect : &ActiveGameplayEffects)
		{
			RemoveActiveGameplayEffect(Effect.Handle);
		}

		//	Remove Attribute Sets
		GetSpawnedAttributes_Mutable().Empty();
	}


	//	If cue still exists because it was not from an effect
	RemoveAllGameplayCues();

	//	Now clean up any loose gameplay tags
	ResetTagMap();
	GetMinimalReplicationTags_Mutable().RemoveAllTags();		//	This line may not be necessary

	//	Give clients changes ASAP
	ForceReplication();
}
