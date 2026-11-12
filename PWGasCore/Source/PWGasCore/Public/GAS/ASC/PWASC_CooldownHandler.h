// copyright mkurtt96

#pragma once

#include "GameplayTagContainer.h"


DECLARE_MULTICAST_DELEGATE_TwoParams(FAbilityCooldownStarted, const FGameplayTag& /*AbilityTag*/, float /*Duration*/);

class PWGASCORE_API FPWASC_CooldownHandler
{
public:
	friend class UPWAbilitySystemComponent;
	explicit FPWASC_CooldownHandler(UPWAbilitySystemComponent& InASC) : ASC(InASC) {}
	virtual ~FPWASC_CooldownHandler() = default;
	UPWAbilitySystemComponent& ASC;

	FAbilityCooldownStarted CooldownStarted; //locally controlled only
	
	bool GetCooldownRemainingForTag(const FGameplayTag& CooldownTag, float& TimeRemaining, float& CooldownDuration) const;
	bool GetCooldownRemainingForAbilityTag(const FGameplayTag& AbilityTag, float& TimeRemaining, float& TotalDuration) const;
	int32 ClearCooldownByTag(const FGameplayTag& CooldownTag) const;
	int32 ClearCooldownForAbilityTag(const FGameplayTag& AbilityTag) const;
	bool AdjustCooldownForTag(const FGameplayTag& CooldownTag, float SecondsToReduce) const;
	bool AdjustCooldownForAbilityTag(const FGameplayTag& AbilityTag, float SecondsToReduce) const;

};
