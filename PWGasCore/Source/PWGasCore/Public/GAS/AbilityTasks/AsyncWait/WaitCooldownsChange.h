// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "WaitCooldownsChange.generated.h"

struct FGameplayEffectSpec;
class UAbilitySystemComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCooldownsChangeDelegate, FGameplayTag, GameplayTag, float, TimeRemaining, float, Cooldown);

/**
 * 
 */
UCLASS(BlueprintType, meta=(ExposedAsyncProxy = "AsyncTask"))
class PWGASCORE_API UWaitCooldownsChange : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FCooldownsChangeDelegate CooldownStarted;
	UPROPERTY(BlueprintAssignable)
	FCooldownsChangeDelegate CooldownEnded;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = true))
	static UWaitCooldownsChange* WaitForCooldownsChange(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTagContainer InCooldownTags);

	UFUNCTION(BlueprintCallable)
	void EndTask();

protected:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;

	FGameplayTagContainer CooldownTags;

	void CooldownTagChanged(FGameplayTag GameplayTag, int NewCount) const;
	void CooldownEffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle) const;
};
