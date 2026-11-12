// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWGameplayAbility_Channeling.h"
#include "PWGameplayAbility_Passive.generated.h"

UCLASS()
class PWGASCORE_API UPWGameplayAbility_Passive : public UPWModularGameplayAbility
{
	GENERATED_BODY()

protected:
	UPWGameplayAbility_Passive();
	
	// Fires On Passive Tick repeatedly while the passive is active.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Passive|Tick")
	bool bEnablePassiveTick = false;

	// Seconds between On Passive Tick events.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Passive|Tick")
	float PassiveTickInterval = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category="Passive")
	bool bIsActivePassive = false;

	FTimerHandle PassiveTickTimer;

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
								 const FGameplayAbilityActorInfo* ActorInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo,
								 const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
							const FGameplayAbilityActorInfo* ActorInfo,
							const FGameplayAbilityActivationInfo ActivationInfo,
							bool bReplicateEndAbility,
							bool bWasCancelled) override;

	UFUNCTION(BlueprintPure, Category="Passive")
	bool IsPassiveActive() const { return bIsActivePassive; }

protected:
	void HandlePassiveTick();

	UFUNCTION(BlueprintImplementableEvent, Category="Passive")
	void OnPassiveTick();
};
