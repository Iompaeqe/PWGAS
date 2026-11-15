// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "PWGameplayAbility_Charged.generated.h"

UCLASS()
class PWGASCORE_API UPWGameplayAbility_Charged : public UPWModularGameplayAbility
{
	GENERATED_BODY()

protected:
	// Starts charging on activation; turn off to call Start Charging from the graph.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Charge")
	bool bAutoStartCharge = true;

	// Seconds needed to reach a full charge.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Charge")
	float ChargeDuration = 2.0f;

	// Seconds between On Charge Tick events.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Charge")
	float ChargeTickInterval = 0.1f;

	// Also ticks the moment the charge starts, at no charge, before the timed ticks.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Charge")
	bool bTickAtStart = false;

	UPROPERTY(BlueprintReadOnly, Category="Charge")
	float CurrentChargeTime = 0.f;

	UPROPERTY(BlueprintReadOnly, Category="Charge")
	bool bIsCharging = false;

	FTimerHandle ChargeTickTimer;
	
public:
	// When the cost is paid. A cost due at the end that can't be paid ends the charge cancelled.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Commit")
	EPWCommitPolicy CostPolicy = EPWCommitPolicy::OnComplete;

	// When the cooldown starts.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Commit")
	EPWCommitPolicy CooldownPolicy = EPWCommitPolicy::OnComplete;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
								 const FGameplayAbilityActorInfo* ActorInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo,
								 const FGameplayEventData* TriggerEventData) override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;

	UFUNCTION(BlueprintCallable, Category="Charge")
	virtual void StartCharging();

	UFUNCTION(BlueprintCallable, Category="Charge")
	virtual void PauseCharging();

	UFUNCTION(BlueprintCallable, Category="Charge")
	virtual void StopCharging(bool bWasCanceled = false);
	
	UFUNCTION(BlueprintPure, Category="Charge")
	float GetChargePercentage() const;

protected:
	// Runs before the modules hear Started, so they use what it sets.
	UFUNCTION(BlueprintImplementableEvent, Category="Charge")
	void OnChargeStart();

	// Runs before the modules hear Tick, so they use what it sets.
	UFUNCTION(BlueprintImplementableEvent, Category="Charge")
	void OnChargeTick(float NormalizedCharge);

	// Runs after the modules hear Completed, which they get while the ability still runs.
	UFUNCTION(BlueprintImplementableEvent, Category="Charge")
	void OnChargeComplete();

	// Runs after the modules hear Cancelled, which they get while the ability still runs.
	UFUNCTION(BlueprintImplementableEvent, Category="Charge")
	void OnChargeCanceled();

	void HandleChargeTick();
	void RunChargeTick();
};
