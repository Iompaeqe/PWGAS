// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once
#include "GameplayTagContainer.h"
#include "GAS/Abilities/PWGameplayAbilityBase.h"

class UGameplayAbility;
class UPWAbilitySet;
class UPWGameplayAbilityBase;
struct FGameplayAbilitySpec;
struct FGameplayAbilitySpecHandle;
struct FActiveGameplayEffect;

DECLARE_MULTICAST_DELEGATE(FAbilitiesEvent)
DECLARE_MULTICAST_DELEGATE_OneParam(FAbilityEvent, const FGameplayTag&);
DECLARE_DELEGATE_OneParam(FForEachAbility, const FGameplayAbilitySpec&);



class PWGASCORE_API FPWASC_AbilityLifecycle
{
public:
	friend class UPWAbilitySystemComponent;
	explicit FPWASC_AbilityLifecycle(UPWAbilitySystemComponent& InASC);
	virtual ~FPWASC_AbilityLifecycle() = default;
	UPWAbilitySystemComponent& ASC;
	
	FAbilitiesEvent AbilitiesUpdated; //server and client 
	FAbilityEvent AbilityAdded; //server only
	FAbilityEvent AbilityRemoved; //server only
	FAbilityEvent AbilityUpdated; //server only
	
	FAbilityEvent AbilityEnded; //server only
	
	/**
	 * Adds multiple abilities, broadcasts Abilities Updated.
	 * Intended to be used to grand startup abilities.
	 * @param AbilityTags 
	 */
	virtual void AddAbilities(const FGameplayTagContainer& AbilityTags) const;
	virtual void AddAbilitySet(const UPWAbilitySet* AbilitySet) const;
	virtual void AddAbility(const FGameplayTag& AbilityTag, const FGameplayTag& InputTag = FGameplayTag()) const;
	virtual void RemoveAbility(const FGameplayTag& AbilityTag) const;
	// False when the ability is missing or already at its max level.
	virtual bool UpgradeAbility(const FGameplayTag& AbilityTag);
	// False when the ability is missing or already at level 1.
	virtual bool DowngradeAbility(const FGameplayTag& AbilityTag);

	void RemoveGrantedAbility(const FGameplayAbilitySpecHandle& Handle) const;

	virtual bool HasAbility(const FGameplayTag& AbilityTag) const;
	virtual int32 GetAbilityLevel(const FGameplayTag& AbilityTag) const;
	virtual bool IsAbilityMaxLevel(const FGameplayTag& AbilityTag) const;

	virtual void ForEachAbility(const FForEachAbility& Delegate) const;
	
	virtual bool ApplyActivationPolicies(const FGameplayAbilitySpecHandle& SpecHandle, const FGameplayTag& EventTag) const;
	virtual bool ApplyActivationPolicies(const FGameplayAbilitySpec& AbilitySpec, const FGameplayTag& EventTag) const;
	virtual void ApplyActivationPolicies(const FGameplayTag& EventTag) const;
	virtual bool ApplyDeactivationPolicies(const FGameplayAbilitySpecHandle& SpecHandle, const FGameplayTag& EventTag) const;
	virtual bool ApplyDeactivationPolicies(const FGameplayAbilitySpec& AbilitySpec, const FGameplayTag& EventTag) const;
	virtual void ApplyDeactivationPolicies(const FGameplayTag& EventTag) const;
	
	// ==  Activation Group  == //
	// ======================== //
	typedef TFunctionRef<bool(const UPWGameplayAbilityBase* Ability, FGameplayAbilitySpecHandle Handle)> TShouldCancelAbilityFunc;
	void CancelAbilitiesByFunc(const TShouldCancelAbilityFunc& ShouldCancelFunc, bool bReplicateCancelAbility) const;
	
	bool IsActivationGroupBlocked(EPWActivationGroup Group) const;
	void AddAbilityToActivationGroup(EPWActivationGroup Group, const UPWGameplayAbilityBase* Ability);
	void RemoveAbilityFromActivationGroup(EPWActivationGroup Group, const UPWGameplayAbilityBase* Ability);
	void CancelActivationGroupAbilities(EPWActivationGroup Group, const UPWGameplayAbilityBase* IgnoreAbility, bool bReplicateCancelAbility) const;

private:
	TSet<TWeakObjectPtr<const UGameplayAbility>> ActiveReplaceable;
	TSet<TWeakObjectPtr<const UGameplayAbility>> ActiveBlocking;
	
	int32 ActivationGroupCounts[(uint8)EPWActivationGroup::MAX];
};
