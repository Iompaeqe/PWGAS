// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/Abilities/Derived/PWGameplayAbility_Channeling.h"

UPWGameplayAbility_Channeling::UPWGameplayAbility_Channeling()
{
	ActivationGroup = EPWActivationGroup::Exclusive_Replaceable;
}

void UPWGameplayAbility_Channeling::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (bAutoStartChannel)
		StartChanneling();
}

void UPWGameplayAbility_Channeling::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	StopChanneling(true);
	
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UPWGameplayAbility_Channeling::StartChanneling()
{
	if (bIsChanneling || !GetWorld()) return;

	if (!CommitAtStart(CostPolicy, CooldownPolicy))
	{
		K2_CancelAbility();
		return;
	}

	bIsChanneling = true;
	CurrentChannelTime = 0.f;

	OnChannelStart();
	if (!bIsChanneling || !IsActive())
		return;
	NotifyModulesOfPhase(EPWAbilityPhase::Started);

	if (bTickAtStart)
	{
		RunChannelTick();
		if (!bIsChanneling || !IsActive())
			return;
	}

	GetWorld()->GetTimerManager().SetTimer(
		ChannelTickTimer,
		this,
		&UPWGameplayAbility_Channeling::HandleChannelTick,
		ChannelTickInterval,
		true
	);
}

void UPWGameplayAbility_Channeling::HandleChannelTick()
{
	if (!bIsChanneling) return;

	CurrentChannelTime += ChannelTickInterval;
	RunChannelTick();
}

void UPWGameplayAbility_Channeling::RunChannelTick()
{
	OnChannelTick(CurrentChannelTime);
	if (!bIsChanneling || !IsActive())
		return;
	NotifyModulesOfPhase(EPWAbilityPhase::Tick);

	if (MaxChannelDuration > 0.f && CurrentChannelTime >= MaxChannelDuration)
	{
		StopChanneling(false);
	}
}

void UPWGameplayAbility_Channeling::PauseChanneling()
{
	if (!bIsChanneling) return;
	GetWorld()->GetTimerManager().PauseTimer(ChannelTickTimer);
}

void UPWGameplayAbility_Channeling::StopChanneling(const bool bWasCanceled)
{
	if (!bIsChanneling) return;

	bIsChanneling = false;
	GetWorld()->GetTimerManager().ClearTimer(ChannelTickTimer);

	if (CommitAtEnd(CostPolicy, CooldownPolicy, bWasCanceled))
	{
		NotifyModulesOfPhase(EPWAbilityPhase::Completed);
		OnChannelEnd();
		return;
	}

	NotifyModulesOfPhase(EPWAbilityPhase::Cancelled);
	OnChannelCancelled();
	if (!bWasCanceled)
		K2_CancelAbility();

	//EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, bWasCanceled);
}

float UPWGameplayAbility_Channeling::GetChannelPercentage() const
{
	if (MaxChannelDuration <= 0.f)
		return 1.f;

	return FMath::Clamp(CurrentChannelTime / MaxChannelDuration, 0.f, 1.f);
}
