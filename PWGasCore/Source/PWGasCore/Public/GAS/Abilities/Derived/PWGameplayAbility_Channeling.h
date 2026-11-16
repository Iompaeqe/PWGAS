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

	// Ends the ability when the channel stops, finished or cancelled; turn off to end it from the graph, for example after an ending animation.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Channel")
	bool bAutoEndAbility = true;

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

	// On while the channel stops because it reached Max Channel Duration, rather than being stopped or cancelled early.
	bool bRanFullLength = false;

	// On while the ability itself is ending or being cancelled, so the channel stopping does not end it a second time.
	bool bEndingAbility = false;

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

	// Runs after the modules hear Completed, which they get while the ability still runs. With Auto End Ability the ability ends right after it.
	UFUNCTION(BlueprintImplementableEvent, Category="Channel")
	void OnChannelEnd();

	// Runs after the modules hear Cancelled, which they get while the ability still runs. With Auto End Ability the ability is cancelled right after it.
	UFUNCTION(BlueprintImplementableEvent, Category="Channel")
	void OnChannelCancelled();

	void HandleChannelTick();
	void RunChannelTick();

	// A predicting client that ran the whole channel ends without telling the server, whose copy runs its own to the end, so the client finishing a moment earlier cannot cut it short;
	// that holds for Auto End Ability and for End Ability called from On Channel End.
	// Any other end that comes mid channel stops the channel first, so its cooldown and end events still run and the next cast can channel again;
	// if the other copy ended first and this one is a tick from the end, that tick runs here too.
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};
