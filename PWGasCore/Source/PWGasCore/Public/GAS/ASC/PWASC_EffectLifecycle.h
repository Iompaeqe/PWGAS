// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once
#include "GameplayTagContainer.h"


class PWGASCORE_API FPWASC_EffectLifecycle 
{
public:
	friend class UPWAbilitySystemComponent;
	explicit FPWASC_EffectLifecycle(UPWAbilitySystemComponent& InASC) : ASC(InASC){}
	virtual ~FPWASC_EffectLifecycle() = default;
	UPWAbilitySystemComponent& ASC;

	void ApplyBuffTag(const FGameplayTag BuffTag) const;
	void RemoveEffectsByTag(const FGameplayTagContainer& TagsToRemove) const;

private:
};
