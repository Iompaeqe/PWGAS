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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Charge")
	bool bAutoStartCharge = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Charge")
	float ChargeDuration = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Charge")
	float ChargeTickInterval = 0.1f;

	UPROPERTY(BlueprintReadOnly, Category="Charge")
	float CurrentChargeTime = 0.f;

	UPROPERTY(BlueprintReadOnly, Category="Charge")
	bool bIsCharging = false;

	FTimerHandle ChargeTickTimer;
	
public:
	// On End commits when the charge completes, a cancelled charge is free.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Commit")
	EPWCommitPolicy CommitPolicy = EPWCommitPolicy::Manual;

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
	UFUNCTION(BlueprintImplementableEvent, Category="Charge")
	void OnChargeStart();

	UFUNCTION(BlueprintImplementableEvent, Category="Charge")
	void OnChargeTick(float NormalizedCharge);

	UFUNCTION(BlueprintImplementableEvent, Category="Charge")
	void OnChargeComplete();

	UFUNCTION(BlueprintImplementableEvent, Category="Charge")
	void OnChargeCanceled();

	void HandleChargeTick();
};
