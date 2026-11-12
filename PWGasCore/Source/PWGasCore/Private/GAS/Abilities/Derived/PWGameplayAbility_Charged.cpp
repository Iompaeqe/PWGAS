// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/Abilities/Derived/PWGameplayAbility_Charged.h"

void UPWGameplayAbility_Charged::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (bAutoStartCharge)
		StartCharging();
}

void UPWGameplayAbility_Charged::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	StopCharging(true);
	
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UPWGameplayAbility_Charged::StartCharging()
{
	if (bIsCharging || !GetWorld()) return;

	if (CommitPolicy == EPWCommitPolicy::OnStart && !CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		K2_CancelAbility();
		return;
	}

	bIsCharging = true;
	CurrentChargeTime = 0.f;

	OnChargeStart();

	GetWorld()->GetTimerManager().SetTimer(
		ChargeTickTimer,
		this,
		&UPWGameplayAbility_Charged::HandleChargeTick,
		ChargeTickInterval,
		true
	);
}

void UPWGameplayAbility_Charged::PauseCharging()
{
	if (!bIsCharging) return;
	GetWorld()->GetTimerManager().PauseTimer(ChargeTickTimer);
}

void UPWGameplayAbility_Charged::StopCharging(bool bWasCanceled)
{
	if (!bIsCharging) return;
	bIsCharging = false;

	GetWorld()->GetTimerManager().ClearTimer(ChargeTickTimer);

	if (!bWasCanceled && CommitPolicy == EPWCommitPolicy::OnEnd && !CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		OnChargeCanceled();
		K2_CancelAbility();
		return;
	}

	if (bWasCanceled)
		OnChargeCanceled();
	else
		OnChargeComplete();

	//EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, bWasCanceled);
}

float UPWGameplayAbility_Charged::GetChargePercentage() const
{
	if (ChargeDuration <= 0.f)
		return 1.f;
	
	return FMath::Clamp(CurrentChargeTime / ChargeDuration, 0.f, 1.f);
}

void UPWGameplayAbility_Charged::HandleChargeTick()
{
	if (!bIsCharging) return;

	CurrentChargeTime += ChargeTickInterval;
	const float Normalized = FMath::Clamp(CurrentChargeTime / ChargeDuration, 0.f, 1.f);

	OnChargeTick(Normalized);

	if (CurrentChargeTime >= ChargeDuration)
	{
		StopCharging(false);
	}
}
