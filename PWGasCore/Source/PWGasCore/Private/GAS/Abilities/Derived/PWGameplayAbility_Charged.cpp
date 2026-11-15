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

	if (!CommitAtStart(CostPolicy, CooldownPolicy))
	{
		K2_CancelAbility();
		return;
	}

	bIsCharging = true;
	CurrentChargeTime = 0.f;

	OnChargeStart();
	if (!bIsCharging || !IsActive())
		return;
	NotifyModulesOfPhase(EPWAbilityPhase::Started);

	if (bTickAtStart)
	{
		RunChargeTick();
		if (!bIsCharging || !IsActive())
			return;
	}

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

	if (CommitAtEnd(CostPolicy, CooldownPolicy, bWasCanceled))
	{
		NotifyModulesOfPhase(EPWAbilityPhase::Completed);
		OnChargeComplete();
		return;
	}

	NotifyModulesOfPhase(EPWAbilityPhase::Cancelled);
	OnChargeCanceled();
	if (!bWasCanceled)
		K2_CancelAbility();

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
	RunChargeTick();
}

void UPWGameplayAbility_Charged::RunChargeTick()
{
	OnChargeTick(GetChargePercentage());
	if (!bIsCharging || !IsActive())
		return;
	NotifyModulesOfPhase(EPWAbilityPhase::Pulse);

	if (CurrentChargeTime >= ChargeDuration)
	{
		StopCharging(false);
	}
}
