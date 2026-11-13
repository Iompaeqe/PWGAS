// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "GAS/Abilities/Derived/PWGameplayAbility_Confirmed.h"

#include "Abilities/Tasks/AbilityTask.h"
#include "GAS/Abilities/Modules/ActionModules/PWActionModule.h"
#include "GAS/AbilityTasks/PWAbilityTask_WaitInputEvent.h"
#include "GAS/ASC/PWASC_DataManagement.h"
#include "GAS/Tags/GASCoreTags.h"

void UPWGameplayAbility_Confirmed::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	SetAwaitingConfirmation(true);
	bConfirmed = false;

	const FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec();
	const FGameplayTagContainer* PressedInputs = ConfirmInputs.Find(EPWInputEventType::Pressed);
	bSkipOwnPress = Spec && PressedInputs && PressedInputs->HasTagExact(FPWASC_DataManagement::GetInputFromSpec(*Spec));

	if (!HasGraph())
		if (UPWAbilityModule_Precast* Precast = GetPrecastModule())
			Precast->StartPrecast();

	for (const TPair<EPWInputEventType, FGameplayTagContainer>& ConfirmInput : ConfirmInputs)
		for (const FGameplayTag& InputTag : ConfirmInput.Value)
			WaitForConfirm(InputTag, ConfirmInput.Key);
}

void UPWGameplayAbility_Confirmed::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (bAwaitingConfirmation)
		SetAwaitingConfirmation(false);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPWGameplayAbility_Confirmed::WaitForConfirm(const FGameplayTag& InputTag, const EPWInputEventType EventType)
{
	if (!InputTag.IsValid())
		return;

	UPWAbilityTask_WaitInputEvent* WaitTask = UPWAbilityTask_WaitInputEvent::WaitInputEvent(this, InputTag, EventType);
	if (!WaitTask)
		return;

	WaitTask->EventReceived.AddDynamic(this, &UPWGameplayAbility_Confirmed::HandleConfirmInput);
	WaitTask->ReadyForActivation();
}

void UPWGameplayAbility_Confirmed::SetAwaitingConfirmation(bool bCond)
{
	bAwaitingConfirmation = bCond;

	FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec();
	if (!Spec)
		return;

	if (bAwaitingConfirmation)
		Spec->GetDynamicSpecSourceTags().AddTag(PWTags::Ability::State::AwaitingConfirmation);
	else
		Spec->GetDynamicSpecSourceTags().RemoveTag(PWTags::Ability::State::AwaitingConfirmation);
}

bool UPWGameplayAbility_Confirmed::IsConfirmInput(const FGameplayTag& InputTag) const
{
	for (const TPair<EPWInputEventType, FGameplayTagContainer>& ConfirmInput : ConfirmInputs)
		if (ConfirmInput.Value.HasTagExact(InputTag))
			return true;

	return false;
}

bool UPWGameplayAbility_Confirmed::HasGraph() const
{
	return bHasBlueprintActivate || bHasBlueprintActivateFromEvent || GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UPWGameplayAbility_Confirmed, OnConfirmed));
}

void UPWGameplayAbility_Confirmed::HandleConfirmInput(const FGameplayTag& InputTag)
{
	if (!bAwaitingConfirmation)
		return;

	const FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec();
	if (bSkipOwnPress && Spec && InputTag == FPWASC_DataManagement::GetInputFromSpec(*Spec))
	{
		bSkipOwnPress = false;
		WaitForConfirm(InputTag, EPWInputEventType::Pressed);
		return;
	}

	bConfirmed = true;
	SetAwaitingConfirmation(false);

	if (HasGraph())
	{
		OnConfirmed(InputTag);
		return;
	}

	if (UPWAbilityModule_Precast* Precast = GetPrecastModule())
		Precast->StopPrecast();

	if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		K2_CancelAbility();
		return;
	}

	if (ActionModule)
	{
		ActionModule->OnActionFinished.RemoveAll(this);
		ActionModule->OnActionFinished.AddUObject(this, &ThisClass::OnActionFinished);

		bExecuting = true;
		ActionModule->ExecuteAction();
		bExecuting = false;
	}

	if (IsActive() && (!ActionModule || !ActionModule->IsActionPending()))
		K2_EndAbility();
}

void UPWGameplayAbility_Confirmed::OnActionFinished()
{
	if (!bExecuting && IsActive())
		K2_EndAbility();
}
