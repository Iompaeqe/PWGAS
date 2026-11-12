// copyright mkurtt96


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

	if (CommitPolicy == EPWCommitPolicy::OnStart && !CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		K2_CancelAbility();
		return;
	}

	bIsChanneling = true;
	CurrentChannelTime = 0.f;

	OnChannelStart();

	

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
	OnChannelTick(CurrentChannelTime);

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

	if (!bWasCanceled && CommitPolicy == EPWCommitPolicy::OnEnd && !CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		OnChannelCancelled();
		K2_CancelAbility();
		return;
	}

	if (bWasCanceled)
		OnChannelCancelled();
	else
		OnChannelEnd();

	//EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, bWasCanceled);
}

float UPWGameplayAbility_Channeling::GetChannelPercentage() const
{
	if (MaxChannelDuration <= 0.f)
		return 1.f;

	return FMath::Clamp(CurrentChannelTime / MaxChannelDuration, 0.f, 1.f);
}
