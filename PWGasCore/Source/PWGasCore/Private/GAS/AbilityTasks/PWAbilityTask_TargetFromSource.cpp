// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/AbilityTasks/PWAbilityTask_TargetFromSource.h"

#include "AbilitySystemComponent.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "GAS/Abilities/Modules/ControlModules/PWAbilityModule_Targeting.h"
#include "Targeting/Data/PWTargetingData.h"

UPWAbilityTask_TargetFromSource* UPWAbilityTask_TargetFromSource::TargetFromSource(UPWModularGameplayAbility* OwningAbility)
{
	auto* Task = NewAbilityTask<UPWAbilityTask_TargetFromSource>(OwningAbility);
	Task->TargetingModule = OwningAbility->GetTargetingModule();
	return Task;
}

void UPWAbilityTask_TargetFromSource::Activate()
{
	const bool bIsLocal = Ability->GetCurrentActorInfo()->IsLocallyControlled();

	if (bIsLocal)
	{
		SendTargetDataLocal();
	}
	else
	{
		FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
		FPredictionKey ActivationKey = GetActivationPredictionKey();

		AbilitySystemComponent->AbilityTargetDataSetDelegate(SpecHandle, ActivationKey).AddUObject(this, &ThisClass::OnTargetDataReplicatedCallback);

		if (!AbilitySystemComponent->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationKey))
		{
			SetWaitingOnRemotePlayerData();
		}
	}
}

void UPWAbilityTask_TargetFromSource::SendTargetDataLocal()
{
	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	if (!PC || !TargetingModule)
	{
		EndTask();
		return;
	}
	
	FPWTargetingResult Result;
	if (!TargetingModule->ComputeTarget(Result))
	{
		EndTask();
		return;
	}

	FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit();
	Data->HitResult = Result.Hit;

	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get());
	
	FGameplayAbilityTargetDataHandle Handle;
	Handle.Add(Data);

	AbilitySystemComponent->ServerSetReplicatedTargetData(
		GetAbilitySpecHandle(),
		GetActivationPredictionKey(),
		Handle,
		FGameplayTag(),
		AbilitySystemComponent->ScopedPredictionKey);

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(Handle, Result);
	}
}

void UPWAbilityTask_TargetFromSource::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag /*ActivationTag*/) const
{
	AbilitySystemComponent->ConsumeClientReplicatedTargetData(
		GetAbilitySpecHandle(), GetActivationPredictionKey());

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		FPWTargetingResult Result;
		TargetingModule->MakeTargetingResultFromTargetData(DataHandle, Result);

		ValidData.Broadcast(DataHandle, Result);
	}
}
