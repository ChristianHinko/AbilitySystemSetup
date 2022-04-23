// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ASSAbilitySystemComponent.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystem/ASSGameplayAbility.h"
#include "GameplayCueManager.h"
#include "AbilitySystemSetup/Private/Utilities/ASSLogCategories.h"
#include "Abilities/GameplayAbilityTargetActor.h"

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
#include "DS_AbilitySystemSetup.h"
#include "GameFramework\InputSettings.h"
#endif // !(UE_BUILD_SHIPPING || UE_BUILD_TEST)



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
	const UDS_AbilitySystemSetup* AbilitySystemSetupDeveloperSettings = GetDefault<UDS_AbilitySystemSetup>();
	const UInputSettings* InputSettings = UInputSettings::GetInputSettings();



	// Ensure this UENUM exists! (Ensure that the game has implemented their AbilityInputID enum that they specified in the plugin settings)
	const UEnum* AbilityInputIDEnum = FindObject<UEnum>(ANY_PACKAGE, *(AbilitySystemSetupDeveloperSettings->AbilityInputIDEnumName));
	if (AbilityInputIDEnum)
	{
		// Ensure this UEnum has proper enum setup! (i.e. enum ``Unset`` and enum ``NoInput`` as the first 2)
		if (AbilityInputIDEnum->GetNameStringByValue(0) != TEXT("Unset") || AbilityInputIDEnum->GetNameStringByIndex(0) != TEXT("Unset"))
		{
			UE_LOG(LogAbilitySystemInputEnumMappingsSafetyChecks, Fatal, TEXT("Your ``%s`` UEnum is missing the ``Unset`` enum. Go to your %s definition and make sure you have ``Unset`` as the first enum (and make sure the value is 0). This is important for us to be able to detect when someone forgets to set an Ability's input ID (it's good to give all Abilities an input ID)"), *(AbilitySystemSetupDeveloperSettings->AbilityInputIDEnumName), *(AbilitySystemSetupDeveloperSettings->AbilityInputIDEnumName));
		}
		if (AbilityInputIDEnum->GetNameStringByValue(1) != TEXT("NoInput") || AbilityInputIDEnum->GetNameStringByIndex(1) != TEXT("NoInput"))
		{
			UE_LOG(LogAbilitySystemInputEnumMappingsSafetyChecks, Fatal, TEXT("Your ``%s`` UEnum is missing the ``NoInput`` enum. Go to your %s definition and make sure you have ``NoInput`` as the second enum (and make sure the value is 1). This enum allows you to state that an Ability does not use input binding"), *(AbilitySystemSetupDeveloperSettings->AbilityInputIDEnumName), *(AbilitySystemSetupDeveloperSettings->AbilityInputIDEnumName));
		}
	}
	else
	{
		UE_LOG(LogAbilitySystemInputEnumMappingsSafetyChecks, Fatal, TEXT("The UEnum ``%s`` does not exist. Ensure correct spelling for the name of your AbilityInputID Enum and make sure it is a UENUM so we can find it!"), *(AbilitySystemSetupDeveloperSettings->AbilityInputIDEnumName));
	}






	// Our input action names (includes speech mappings)
	TArray<FName> ActionNames;
	InputSettings->GetActionNames(ActionNames);

	const FName& ConfirmTargetInputActionName = FName(AbilitySystemSetupDeveloperSettings->ConfirmTargetInputActionName);
	const FName& CancelTargetInputActionName = FName(AbilitySystemSetupDeveloperSettings->CancelTargetInputActionName);

	// Ensure the Confirm and Cancel Target input actions exist! (Check to see if ConfirmTargetInputActionName and CancelTargetInputActionName in the plugin settings are real inputs)
	if (ActionNames.Contains(ConfirmTargetInputActionName) == false)
	{
		UE_LOG(LogAbilitySystemInputEnumMappingsSafetyChecks, Fatal, TEXT("The ``%s`` input action does not exist in your Action Mappings list in DefaultInput.ini - Ensure correct spelling for the name of your ConfirmTarget input action!"), *(AbilitySystemSetupDeveloperSettings->ConfirmTargetInputActionName));
	}
	if (ActionNames.Contains(CancelTargetInputActionName) == false)
	{
		UE_LOG(LogAbilitySystemInputEnumMappingsSafetyChecks, Fatal, TEXT("The ``%s`` input action does not exist in your Action Mappings list in DefaultInput.ini - Ensure correct spelling for the name of your CancelTarget input action!"), *(AbilitySystemSetupDeveloperSettings->CancelTargetInputActionName));
	}



	// Ensure the enum matches Action Mappings!
	{
		int32 ExpectedEnumIndex = 2; // what this current ActionName's index should be in the AbilityInputID UEnum
		for (const FName& ActionName : ActionNames)
		{
			if (ActionName == ConfirmTargetInputActionName || ActionName == CancelTargetInputActionName)
			{
				// Ignore our ConfirmTarget and CancelTarget actions - these are not part of the AbilityInputID UEnum
				continue;
			}


			if (AbilityInputIDEnum->GetIndexByName(ActionName) != ExpectedEnumIndex)
			{
				UE_LOG(LogAbilitySystemInputEnumMappingsSafetyChecks, Fatal, TEXT("Your %s UEnum is not matched up with your Action Mappings list in DefaultInput.ini - Expected ``%s`` enum at the %s spot in %s."), *(AbilitySystemSetupDeveloperSettings->AbilityInputIDEnumName), *(ActionName.ToString()), *FString::FromInt(ExpectedEnumIndex), *(AbilitySystemSetupDeveloperSettings->AbilityInputIDEnumName));
			}
			if (AbilityInputIDEnum->GetValueByName(ActionName) != ExpectedEnumIndex)
			{
				UE_LOG(LogAbilitySystemInputEnumMappingsSafetyChecks, Fatal, TEXT("%s::%s has defined a numeric value for itself. These enums should provide no numeric use and are purely to represent Action Mappings. Leave the enum at the default determined value. Even if you did do it this way, the Ability System's input events won't work that way"), *(AbilitySystemSetupDeveloperSettings->AbilityInputIDEnumName), *(ActionName.ToString()));
			}


			++ExpectedEnumIndex;
		}
	}
#endif // !(UE_BUILD_SHIPPING || UE_BUILD_TEST)

}


void UASSAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!IsValid(AbilitySpec.SourceObject))
	{
		UE_LOG(LogAbilitySystemComponentSetup, Fatal, TEXT("%s() SourceObject was not valid when Ability was given. Someone must have forgotten to set it when giving the Ability"), ANSI_TO_TCHAR(__FUNCTION__));
	}
#endif // !(UE_BUILD_SHIPPING || UE_BUILD_TEST)

	// NOTE: I want to do this in UGameplayAbility::OnGiveAbility() instead but the given Spec is const there
	UASSGameplayAbility* ASSAbility = Cast<UASSGameplayAbility>(AbilitySpec.Ability);
	if (IsValid(ASSAbility))
	{
		//if (ASSAbility->AbilityInputID != 1)
		//{
			// Take the configured InputID from the Ability
			AbilitySpec.InputID = static_cast<int32>(ASSAbility->AbilityInputID);
		//}
		//else
		//{
		//	// We are EAbilityInputID::NoInput
		//	AbilitySpec.InputID = INDEX_NONE;
		//}
	}
	else
	{
		UE_LOG(LogAbilitySystemComponentSetup, Warning, TEXT("%s() no meaningful AbilityInputID for given Ability because the UASSGameplayAbility was null"), ANSI_TO_TCHAR(__FUNCTION__));
	}


	Super::OnGiveAbility(AbilitySpec);
}

void UASSAbilitySystemComponent::TargetConfirmByAbility(UGameplayAbility* AbilityToConfirmTargetOn)
{
	// Callbacks may modify the spawned target actor array so iterate over a copy instead
	TArray<AGameplayAbilityTargetActor*> LocalTargetActors = SpawnedTargetActors;
	SpawnedTargetActors.Reset();
	for (AGameplayAbilityTargetActor* TargetActor : LocalTargetActors)
	{
		if (TargetActor)
		{
			if (TargetActor->IsConfirmTargetingAllowed())
			{
				if (TargetActor->OwningAbility == AbilityToConfirmTargetOn) // =@OVERRIDED CODE MARKER@= wrapped in this if statement
				{
					//TODO: There might not be any cases where this bool is false
					if (!TargetActor->bDestroyOnConfirmation)
					{
						SpawnedTargetActors.Add(TargetActor);
					}
					TargetActor->ConfirmTargeting();
				}
			}
			else
			{
				SpawnedTargetActors.Add(TargetActor);
			}
		}
	}
}

void UASSAbilitySystemComponent::TargetCancelByAbility(UGameplayAbility* AbilityToCancelTargetOn)
{
	// Callbacks may modify the spawned target actor array so iterate over a copy instead
	TArray<AGameplayAbilityTargetActor*> LocalTargetActors = SpawnedTargetActors;
	SpawnedTargetActors.Reset();
	for (AGameplayAbilityTargetActor* TargetActor : LocalTargetActors)
	{
		if (TargetActor)
		{
			if (TargetActor->OwningAbility == AbilityToCancelTargetOn) // =@OVERRIDED CODE MARKER@= wrapped in this if statement
			{
				TargetActor->CancelTargeting();
			}
			else // =@OVERRIDED CODE MARKER@= add this else statement
			{
				SpawnedTargetActors.Add(TargetActor);
			}
		}
	}
}

#pragma region Input Binding
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
#pragma endregion

void UASSAbilitySystemComponent::GetActiveAbilitiesWithTags(const FGameplayTagContainer& GameplayTagContainer, TArray<UGameplayAbility*>& ActiveAbilities)
{
	TArray<FGameplayAbilitySpec*> AbilitiesToActivate;
	GetActivatableGameplayAbilitySpecsByAllMatchingTags(GameplayTagContainer, AbilitiesToActivate, false);

	// Iterate the list of all ability specs
	for (FGameplayAbilitySpec* Spec : AbilitiesToActivate)
	{
		// Iterate all instances on this ability spec
		TArray<UGameplayAbility*> AbilityInstances = Spec->GetAbilityInstances();

		for (UGameplayAbility* ActiveAbility : AbilityInstances)
		{
			ActiveAbilities.Add(Cast<UGameplayAbility>(ActiveAbility));
		}
	}
}

FGameplayAbilitySpecHandle UASSAbilitySystemComponent::FindAbilitySpecHandleFromClass(TSubclassOf<UGameplayAbility> AbilityClass, UObject* OptionalSourceObject)
{
	ABILITYLIST_SCOPE_LOCK();
	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		TSubclassOf<UGameplayAbility> SpecAbilityClass = Spec.Ability->GetClass();
		if (SpecAbilityClass == AbilityClass)
		{
			if (!OptionalSourceObject || (OptionalSourceObject && Spec.SourceObject == OptionalSourceObject))
			{
				return Spec.Handle;
			}
		}
	}

	return FGameplayAbilitySpecHandle();
}

void UASSAbilitySystemComponent::ExecuteGameplayCueLocal(const FGameplayTag& GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters)
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::Executed, GameplayCueParameters);
}
void UASSAbilitySystemComponent::AddGameplayCueLocal(const FGameplayTag& GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters)
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::OnActive, GameplayCueParameters);
}
void UASSAbilitySystemComponent::RemoveGameplayCueLocal(const FGameplayTag& GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters)
{
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::Removed, GameplayCueParameters);
}



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
