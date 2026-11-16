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

	// Ends the ability when the charge stops, released or cancelled; turn off to end it from the graph, for example after a release animation.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Charge")
	bool bAutoEndAbility = true;

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

	// On while the charge stops because it reached Charge Duration, rather than being released or cancelled early.
	bool bRanFullLength = false;

	// On while the ability itself is ending or being cancelled, so the charge stopping does not end it a second time.
	bool bEndingAbility = false;

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

	// Runs after the modules hear Completed, which they get while the ability still runs. With Auto End Ability the ability ends right after it.
	UFUNCTION(BlueprintImplementableEvent, Category="Charge")
	void OnChargeComplete();

	// Runs after the modules hear Cancelled, which they get while the ability still runs. With Auto End Ability the ability is cancelled right after it.
	UFUNCTION(BlueprintImplementableEvent, Category="Charge")
	void OnChargeCanceled();

	void HandleChargeTick();
	void RunChargeTick();

	// A predicting client whose charge filled by itself ends without telling the server, whose copy fills its own, so the client finishing a moment earlier cannot cut it short;
	// that holds for Auto End Ability and for End Ability called from On Charge Complete.
	// Any other end that comes mid charge stops the charge first, so its cooldown and end events still run and the next cast can charge again;
	// if the other copy ended first and this one is a tick from full, that tick runs here too.
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};
