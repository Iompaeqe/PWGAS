// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWGameplayAbility_Charged.h"
#include "PWGameplayAbility_Channeling.generated.h"

UCLASS()
class PWGASCORE_API UPWGameplayAbility_Channeling : public UPWModularGameplayAbility
{
	GENERATED_BODY()

protected:
	UPWGameplayAbility_Channeling();

	// Starts channeling on activation; turn off to call Start Channeling from the graph.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Channel")
	bool bAutoStartChannel = true;

	// Seconds between On Channel Tick events.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Channel")
	float ChannelTickInterval = 0.5f;

	// Seconds until the channel ends by itself; 0 channels until stopped.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Channel")
	float MaxChannelDuration = 0.f; // 0 = infinite

	// Also ticks the moment the channel starts, at time 0, before the timed ticks.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Channel")
	bool bTickAtStart = false;

	UPROPERTY(BlueprintReadOnly, Category="Channel")
	bool bIsChanneling = false;

	UPROPERTY(BlueprintReadOnly, Category="Channel")
	float CurrentChannelTime = 0.f;

	FTimerHandle ChannelTickTimer;

public:
	// When the cost is paid. A cost due at the end that can't be paid ends the channel cancelled.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Commit")
	EPWCommitPolicy CostPolicy = EPWCommitPolicy::OnStart;

	// When the cooldown starts.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Commit")
	EPWCommitPolicy CooldownPolicy = EPWCommitPolicy::OnEnd;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;

	UFUNCTION(BlueprintCallable, Category="Channel")
	virtual void StartChanneling();

	UFUNCTION(BlueprintCallable, Category="Channel")
	virtual void PauseChanneling();

	UFUNCTION(BlueprintCallable, Category="Channel")
	virtual void StopChanneling(bool bWasCanceled = false);

	UFUNCTION(BlueprintPure, Category="Channel")
	float GetChannelPercentage() const;

protected:
	// Runs before the modules hear Started, so they use what it sets.
	UFUNCTION(BlueprintImplementableEvent, Category="Channel")
	void OnChannelStart();

	// Runs before the modules hear Tick, so they use what it sets.
	UFUNCTION(BlueprintImplementableEvent, Category="Channel")
	void OnChannelTick(float ElapsedTime);

	// Runs after the modules hear Completed, which they get while the ability still runs.
	UFUNCTION(BlueprintImplementableEvent, Category="Channel")
	void OnChannelEnd();

	// Runs after the modules hear Cancelled, which they get while the ability still runs.
	UFUNCTION(BlueprintImplementableEvent, Category="Channel")
	void OnChannelCancelled();

	void HandleChannelTick();
	void RunChannelTick();
};
