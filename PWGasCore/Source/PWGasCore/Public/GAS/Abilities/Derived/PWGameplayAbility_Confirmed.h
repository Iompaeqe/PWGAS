// copyright mkurtt96

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "GAS/Data/GASCoreEnums.h"
#include "PWGameplayAbility_Confirmed.generated.h"

UCLASS()
class PWGASCORE_API UPWGameplayAbility_Confirmed : public UPWModularGameplayAbility
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly)
	bool bAwaitingConfirmation = false;

	UPROPERTY(BlueprintReadOnly)
	bool bConfirmed = false;

	UPROPERTY(EditDefaultsOnly, meta=(GameplayTagFilter = "Input"))
	TMap<EPWInputEventType, FGameplayTagContainer> ConfirmInputs;

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual bool DefersAction() const override { return true; }

	UFUNCTION(BlueprintImplementableEvent, Category="Ability|Confirm")
	void OnConfirmed(FGameplayTag InputTag);
	UFUNCTION()
	void HandleConfirmInput(const FGameplayTag& InputTag);

	UFUNCTION(BlueprintPure, Category="Ability|Confirm")
	bool IsConfirmInput(const FGameplayTag& InputTag) const;

	void SetAwaitingConfirmation(bool bCond);

private:
	bool HasGraph() const;
	void WaitForConfirm(const FGameplayTag& InputTag, EPWInputEventType EventType);
	void OnActionFinished();

	bool bSkipOwnPress = false;
	bool bExecuting = false;
};
