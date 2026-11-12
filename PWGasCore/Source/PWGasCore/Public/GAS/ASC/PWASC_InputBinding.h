// copyright mkurtt96

#pragma once
#include "GameplayTagContainer.h"

class UGameplayAbility;
struct FGameplayAbilitySpec;

DECLARE_MULTICAST_DELEGATE(FAbilitiesEvent)
DECLARE_MULTICAST_DELEGATE_OneParam(FAbilityEvent, const FGameplayTag&);

class PWGASCORE_API FPWASC_InputBinding
{
public:
	
	friend class UPWAbilitySystemComponent;
	explicit FPWASC_InputBinding(UPWAbilitySystemComponent& InASC);
	virtual ~FPWASC_InputBinding() = default;
	UPWAbilitySystemComponent& ASC;

	FAbilitiesEvent AbilityInputsUpdated;
	FAbilityEvent AbilityInputUpdated;

	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag) const;

	void EquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& InputTag) const;
	auto SetOrSwapInputOfAbility(const FGameplayTag& FirstAbilityTag, const FGameplayTag& SecondInputTag) const -> void;
	void AssignInputToSpec(FGameplayAbilitySpec& Spec, const FGameplayTag& InputTag) const;
	void ClearInputFromSpec(FGameplayAbilitySpec* Spec) const;
	bool IsInputUsed(const FGameplayTag& InputTag) const;
	
protected:
	bool HandleAwaitingAbility(const FGameplayTag& InputTag, const FGameplayAbilitySpec& PressedSpec) const;
	
};
