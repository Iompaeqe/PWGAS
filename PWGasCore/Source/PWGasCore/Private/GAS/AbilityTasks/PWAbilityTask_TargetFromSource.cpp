// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/AbilityTasks/PWAbilityTask_TargetFromSource.h"

#include "AbilitySystemComponent.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "GAS/Abilities/Modules/ControlModules/PWAbilityModule_Targeting.h"
#include "Targeting/Data/PWTargetingData.h"
#include "PWGASCoreLogChannels.h"
#include "HAL/IConsoleManager.h"

// Logs each step of a cast's aim: what the caster sent, what the server received and where it spawned.
static TAutoConsoleVariable<int32> CVarPWDebugTargeting(TEXT("PW.Debug.Targeting"), 0, TEXT("Log the aim a caster sends, the aim the server receives and the spawn that follows."));

static FString DescribeHit(const FHitResult& Hit)
{
	return FString::Printf(TEXT("blocking=%d startPenetrating=%d actor=%s location=%s impact=%s traceStart=%s traceEnd=%s"), Hit.bBlockingHit, Hit.bStartPenetrating, *GetNameSafe(Hit.GetActor()), *Hit.Location.ToCompactString(), *Hit.ImpactPoint.ToCompactString(), *Hit.TraceStart.ToCompactString(), *Hit.TraceEnd.ToCompactString());
}

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
	const APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	if (!TargetingModule)
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

	if (CVarPWDebugTargeting.GetValueOnGameThread())
		UE_LOG(LogPWGASCoreAbilityModule, Warning, TEXT("[Targeting] %s SENT by %s: target=%s | %s"), *GetNameSafe(Ability), *GetNameSafe(GetAvatarActor()), *Result.Location.ToCompactString(), *DescribeHit(Result.Hit));

	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get());
	
	FGameplayAbilityTargetDataHandle Handle;
	Handle.Add(Data);

	// Only a player has a server to tell; an AI already runs there.
	if (PC)
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
	// When the aim arrived before this task did, DataHandle is the ability system's cached copy, and consuming clears it. Keep our own first.
	const FGameplayAbilityTargetDataHandle Data = DataHandle;
	AbilitySystemComponent->ConsumeClientReplicatedTargetData(
		GetAbilitySpecHandle(), GetActivationPredictionKey());

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		FPWTargetingResult Result;
		TargetingModule->MakeTargetingResultFromTargetData(Data, Result);

		if (CVarPWDebugTargeting.GetValueOnGameThread())
			UE_LOG(LogPWGASCoreAbilityModule, Warning, TEXT("[Targeting] %s RECEIVED for %s: target=%s | %s"), *GetNameSafe(Ability), *GetNameSafe(GetAvatarActor()), *Result.Location.ToCompactString(), *DescribeHit(Result.Hit));

		ValidData.Broadcast(Data, Result);
	}
}
