// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/ASC/PWASC_CooldownHandler.h"

#include "GameplayEffect.h"
#include "GAS/ASC/PWAbilitySystemComponent.h"
#include "GAS/Tags/GASCoreTags.h"

bool FPWASC_CooldownHandler::GetCooldownRemainingForTag(const FGameplayTag& CooldownTag, float& TimeRemaining, float& CooldownDuration) const
{
	FGameplayEffectQuery const Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTag.GetSingleTagContainer());
	TArray<TPair<float, float>> DurationAndTimeRemaining = ASC.GetActiveEffectsTimeRemainingAndDuration(Query);
	if (DurationAndTimeRemaining.Num() > 0)
	{
		int32 BestIdx = 0;
		float LongestTime = DurationAndTimeRemaining[0].Key;
		for (int32 Idx = 1; Idx < DurationAndTimeRemaining.Num(); ++Idx)
		{
			if (DurationAndTimeRemaining[Idx].Key > LongestTime)
			{
				LongestTime = DurationAndTimeRemaining[Idx].Key;
				BestIdx = Idx;
			}
		}

		TimeRemaining = DurationAndTimeRemaining[BestIdx].Key;
		CooldownDuration = DurationAndTimeRemaining[BestIdx].Value;
		return true;
	}
	return false;
}

bool FPWASC_CooldownHandler::GetCooldownRemainingForAbilityTag(const FGameplayTag& AbilityTag, float& TimeRemaining, float& TotalDuration) const
{
	const FGameplayTag CooldownTag = PWTags::Ability::Skill::GetCooldownTag(AbilityTag);
	return CooldownTag.IsValid() ? GetCooldownRemainingForTag(CooldownTag, TimeRemaining, TotalDuration) : false;
}

int32 FPWASC_CooldownHandler::ClearCooldownByTag(const FGameplayTag& CooldownTag) const
{
	FGameplayTagContainer Tags;
	Tags.AddTag(CooldownTag);
	return ASC.RemoveActiveEffectsWithGrantedTags(Tags);
}

int32 FPWASC_CooldownHandler::ClearCooldownForAbilityTag(const FGameplayTag& AbilityTag) const
{
	const FGameplayTag CooldownTag = PWTags::Ability::Skill::GetCooldownTag(AbilityTag);
	return CooldownTag.IsValid() ? ClearCooldownByTag(CooldownTag) : 0;
}

bool FPWASC_CooldownHandler::AdjustCooldownForTag(const FGameplayTag& CooldownTag, float SecondsToReduce) const
{
	float Remaining = 0, Total = 0;
	if (!GetCooldownRemainingForTag(CooldownTag, Remaining, Total) || Remaining <= 0.f) return false;

	const FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTag.GetSingleTagContainer());
	for (const FActiveGameplayEffectHandle& Handle : ASC.GetActiveEffects(Query))
		ASC.ModifyActiveEffectStartTime(Handle, -SecondsToReduce);
	return true;
}

bool FPWASC_CooldownHandler::AdjustCooldownForAbilityTag(const FGameplayTag& AbilityTag, float SecondsToReduce) const
{
	const FGameplayTag CooldownTag = PWTags::Ability::Skill::GetCooldownTag(AbilityTag);
	return CooldownTag.IsValid() ? AdjustCooldownForTag(CooldownTag, SecondsToReduce) : false;
}
