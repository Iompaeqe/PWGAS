// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/Tags/PWTagFunctions.h"

#include "AbilitySystemComponent.h"
#include "GAS/Tags/GASCoreTags.h"

FGameplayTag UPWTagFunctions::GetAbilityCooldownTag(const FGameplayTag& AbilityTag)
{
	if (AbilityTag.MatchesTag(PWTags::Ability::Skill::Root))
		return PWTags::Ability::Skill::GetCooldownTag(AbilityTag);
	return FGameplayTag();
}

FGameplayTag UPWTagFunctions::GetAbilityTagWithSuffix(const FGameplayTag& AbilityTag, const FString& Suffix)
{
	if (AbilityTag.MatchesTag(PWTags::Ability::Skill::Root))
	{
		FString FullString = AbilityTag.ToString() + TEXT(".") + Suffix;
		return FGameplayTag::RequestGameplayTag(*FullString, false);
	}
	return FGameplayTag();
}

FGameplayTag UPWTagFunctions::FindTagWithSuffix(const FGameplayTagContainer& Container, const FString& Suffix)
{
	for (const FGameplayTag& Tag : Container)
		if (Tag.ToString().EndsWith(Suffix))
			return Tag;
	return FGameplayTag();
}

FGameplayAbilitySpec* UPWTagFunctions::GetCurrentSpec(const UGameplayAbility* Ability, UAbilitySystemComponent*& OutASC)
{
	OutASC = nullptr;
	if (!Ability) return nullptr;

	const FGameplayAbilityActorInfo* Info = Ability->GetCurrentActorInfo();
	if (!Info) return nullptr;

	OutASC = Info->AbilitySystemComponent.Get();
	if (!OutASC) return nullptr;

	const FGameplayAbilitySpecHandle Handle = Ability->GetCurrentAbilitySpecHandle();
	if (!Handle.IsValid()) return nullptr;

	return OutASC->FindAbilitySpecFromHandle(Handle);
}
