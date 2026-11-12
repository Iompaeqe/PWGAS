// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "GAS/ASC/PWASC_DataManagement.h"

#include "GameplayAbilitySpec.h"
#include "GAS/ASC/PWAbilitySystemComponent.h"
#include "GAS/Tags/GASCoreTags.h"

FGameplayTag FPWASC_DataManagement::GetInputFromSpec(const FGameplayAbilitySpec& Spec)
{
	if (Spec.Ability)
		for (auto Tag : Spec.GetDynamicSpecSourceTags())
			if (Tag.MatchesTag(PWTags::Input::Root))
				return Tag;

	return FGameplayTag();
}

FGameplayTag FPWASC_DataManagement::GetAbilityFromSpec(const FGameplayAbilitySpec& Spec)
{
	if (Spec.Ability)
		for (auto Tag : Spec.Ability.Get()->GetAssetTags())
			if (Tag.MatchesTag(PWTags::Ability::Skill::Root))
				return Tag;

	return FGameplayTag();
}

FGameplayAbilitySpec* FPWASC_DataManagement::GetSpecOfAbility(const FGameplayTag& AbilityTag) const
{
	FScopedAbilityListLock ActiveScopeLock(ASC);
	for (FGameplayAbilitySpec& Spec : ASC.GetActivatableAbilities())
		if (Spec.Ability.Get()->GetAssetTags().HasTag(AbilityTag))
			return &Spec;

	return nullptr;
}

FGameplayAbilitySpec* FPWASC_DataManagement::GetSpecOfInput(const FGameplayTag& InputTag) const
{
	FScopedAbilityListLock ActiveScopeLock(ASC);
	for (FGameplayAbilitySpec& Spec : ASC.GetActivatableAbilities())
		if (Spec.GetDynamicSpecSourceTags().HasTag(InputTag))
			return &Spec;

	return nullptr;
}

FGameplayTag FPWASC_DataManagement::GetInputOfAbility(const FGameplayTag& AbilityTag) const
{
	if (const FGameplayAbilitySpec* Spec = GetSpecOfAbility(AbilityTag))
		return GetInputFromSpec(*Spec);

	return FGameplayTag();
}

FGameplayTag FPWASC_DataManagement::GetAbilityOfInput(const FGameplayTag& InputTag) const
{
	if (const FGameplayAbilitySpec* Spec = GetSpecOfInput(InputTag))
		return GetAbilityFromSpec(*Spec);

	return FGameplayTag();
}

const FActiveGameplayEffect* FPWASC_DataManagement::GetSpecOfEffect(const FGameplayTag& EffectTag) const
{
	FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(EffectTag.GetSingleTagContainer());

	TArray<FActiveGameplayEffectHandle> EffectHandles = ASC.GetActiveEffects(Query);

	if (EffectHandles.Num() > 0 && EffectHandles[0].IsValid())
	{
		const FActiveGameplayEffect* Effect = ASC.GetActiveGameplayEffect(EffectHandles[0]);
		return Effect;
	}

	return nullptr;
}

FGameplayAbilitySpec* FPWASC_DataManagement::GetFirstActiveSpecWithTag(const FGameplayTagContainer& TagsToCheck, bool bMatchExact, bool bIncludeDynamicTags) const
{
	FScopedAbilityListLock Lock(ASC);
	for (FGameplayAbilitySpec& Spec : ASC.GetActivatableAbilities())
	{
		if (!Spec.IsActive()) continue;

		FGameplayTagContainer All = Spec.Ability->GetAssetTags();
		if (bIncludeDynamicTags) { All.AppendTags(Spec.GetDynamicSpecSourceTags()); }

		const bool bHas = bMatchExact ? All.HasAnyExact(TagsToCheck) : All.HasAny(TagsToCheck);
		if (bHas) return &Spec;
	}
	return nullptr;
}

FGameplayAbilitySpec* FPWASC_DataManagement::GetFirstActiveSpecWithTag(const FGameplayTag& TagToCheck, bool bMatchExact, bool bIncludeDynamicTags) const
{
	return GetFirstActiveSpecWithTag(TagToCheck.GetSingleTagContainer(), bMatchExact, bIncludeDynamicTags);
}

bool FPWASC_DataManagement::DoesAbilityHaveTag(const FGameplayTag& AbilityTag, const FGameplayTagContainer& TagsToCheck, const bool bMatchExact, const bool bIncludeDynamicTags) const
{
	if (FGameplayAbilitySpec* Spec = GetSpecOfAbility(AbilityTag))
	{
		FGameplayTagContainer AllTags = Spec->Ability.Get()->GetAssetTags();
		if (bIncludeDynamicTags)
		{
			AllTags.AppendTags(Spec->GetDynamicSpecSourceTags());
		}

		return bMatchExact
			       ? AllTags.HasAnyExact(TagsToCheck)
			       : AllTags.HasAny(TagsToCheck);
	}
	return false;
}

bool FPWASC_DataManagement::DoesAbilityHaveTag(const FGameplayTag& AbilityTag, const FGameplayTag& TagToCheck, const bool bMatchExact, const bool bIncludeDynamicTags) const
{
	return DoesAbilityHaveTag(AbilityTag, TagToCheck.GetSingleTagContainer(), bMatchExact, bIncludeDynamicTags);
}

bool FPWASC_DataManagement::DoesAbilityHaveTag(const FGameplayAbilitySpecHandle& AbilitySpecHandle, const FGameplayTagContainer& TagsToCheck, const bool bMatchExact, const bool bIncludeDynamicTags) const
{
	if (!AbilitySpecHandle.IsValid()) return false;
	
	FGameplayAbilitySpec* FocusedSpec = ASC.FindAbilitySpecFromHandle(AbilitySpecHandle);
	if (!FocusedSpec) return false;

	FGameplayTagContainer AllTags = FocusedSpec->Ability.Get()->GetAssetTags();
	if (bIncludeDynamicTags)
	{
		AllTags.AppendTags(FocusedSpec->GetDynamicSpecSourceTags());
	}

	return bMatchExact
		       ? AllTags.HasAnyExact(TagsToCheck)
		       : AllTags.HasAny(TagsToCheck);
}

bool FPWASC_DataManagement::DoesAbilityHaveTag(const FGameplayAbilitySpecHandle& AbilitySpecHandle, const FGameplayTag& TagToCheck, const bool bMatchExact, const bool bIncludeDynamicTags) const
{
	return DoesAbilityHaveTag(AbilitySpecHandle, TagToCheck.GetSingleTagContainer(), bMatchExact, bIncludeDynamicTags);
}

TArray<FGameplayTag> FPWASC_DataManagement::GetAbilitiesWithTags(const FGameplayTagContainer& TagsToCheck, const bool bMatchExact, const bool bIncludeDynamicTags) const
{
	TArray<FGameplayTag> MatchingAbilities;

	FScopedAbilityListLock ActiveScopeLock(ASC);
	for (const FGameplayAbilitySpec& Spec : ASC.GetActivatableAbilities())
	{
		FGameplayTagContainer AllTags = Spec.Ability.Get()->GetAssetTags();
		if (bIncludeDynamicTags)
		{
			AllTags.AppendTags(Spec.GetDynamicSpecSourceTags());
		}

		bool bMatch = bMatchExact
			              ? AllTags.HasAnyExact(TagsToCheck)
			              : AllTags.HasAny(TagsToCheck);

		if (bMatch)
		{
			for (const FGameplayTag& Tag : Spec.Ability->GetAssetTags())
			{
				if (Tag.MatchesTag(PWTags::Ability::Skill::Root))
				{
					MatchingAbilities.Add(Tag);
				}
			}
		}
	}

	return MatchingAbilities;
}

TArray<FGameplayTag> FPWASC_DataManagement::GetAbilitiesWithTags(const FGameplayTag& TagsToCheck, const bool bMatchExact, const bool bIncludeDynamicTags) const
{
	return GetAbilitiesWithTags(TagsToCheck.GetSingleTagContainer(), bMatchExact, bIncludeDynamicTags);
}

FGameplayTag FPWASC_DataManagement::GetFirstAbilityWithTag(const FGameplayTagContainer& TagsToCheck, const bool bMatchExact, const bool bIncludeDynamicTags) const
{
	FScopedAbilityListLock ActiveScopeLock(ASC);
	for (const FGameplayAbilitySpec& Spec : ASC.GetActivatableAbilities())
	{
		FGameplayTagContainer AllTags = Spec.Ability.Get()->GetAssetTags();
		if (bIncludeDynamicTags)
		{
			AllTags.AppendTags(Spec.GetDynamicSpecSourceTags());
		}

		bool bMatch = bMatchExact
			              ? AllTags.HasAnyExact(TagsToCheck)
			              : AllTags.HasAny(TagsToCheck);

		if (bMatch)
		{
			for (const FGameplayTag& Tag : Spec.Ability->GetAssetTags())
			{
				if (Tag.MatchesTag(PWTags::Ability::Skill::Root))
				{
					return Tag;
				}
			}
		}
	}
	return FGameplayTag();
}

FGameplayTag FPWASC_DataManagement::GetFirstAbilityWithTag(const FGameplayTag& TagsToCheck, const bool bMatchExact, const bool bIncludeDynamicTags) const
{
	return GetFirstAbilityWithTag(TagsToCheck.GetSingleTagContainer(), bMatchExact, bIncludeDynamicTags);
}

TArray<FGameplayAbilitySpec*> FPWASC_DataManagement::GetSpecsWithTags(const FGameplayTagContainer& TagsToCheck, const bool bMatchExact, const bool bIncludeDynamicTags) const
{
	TArray<FGameplayAbilitySpec*> MatchingSpecs;

	FScopedAbilityListLock ActiveScopeLock(ASC);
	for (FGameplayAbilitySpec& Spec : ASC.GetActivatableAbilities())
	{
		FGameplayTagContainer AllTags = Spec.Ability.Get()->GetAssetTags();
		if (bIncludeDynamicTags)
		{
			AllTags.AppendTags(Spec.GetDynamicSpecSourceTags());
		}

		bool bMatch = bMatchExact
			              ? AllTags.HasAnyExact(TagsToCheck)
			              : AllTags.HasAny(TagsToCheck);

		if (bMatch)
		{
			MatchingSpecs.Add(&Spec);
		}
	}

	return MatchingSpecs;
}

TArray<FGameplayAbilitySpec*> FPWASC_DataManagement::GetSpecsWithTags(const FGameplayTag& TagToCheck, const bool bMatchExact, const bool bIncludeDynamicTags) const
{
	return GetSpecsWithTags(TagToCheck.GetSingleTagContainer(), bMatchExact, bIncludeDynamicTags);
}

FGameplayAbilitySpec* FPWASC_DataManagement::GetFirstSpecWithTag(const FGameplayTagContainer& TagsToCheck, const bool bMatchExact, const bool bIncludeDynamicTags) const
{
	FScopedAbilityListLock ActiveScopeLock(ASC);
	for (FGameplayAbilitySpec& Spec : ASC.GetActivatableAbilities())
	{
		FGameplayTagContainer AllTags = Spec.Ability.Get()->GetAssetTags();
		if (bIncludeDynamicTags)
		{
			AllTags.AppendTags(Spec.GetDynamicSpecSourceTags());
		}

		bool bMatch = bMatchExact
			              ? AllTags.HasAnyExact(TagsToCheck)
			              : AllTags.HasAny(TagsToCheck);

		if (bMatch)
		{
			return &Spec;
		}
	}
	return nullptr;
}

FGameplayAbilitySpec* FPWASC_DataManagement::GetFirstSpecWithTag(const FGameplayTag& TagToCheck, const bool bMatchExact, const bool bIncludeDynamicTags) const
{
	return GetFirstSpecWithTag(TagToCheck.GetSingleTagContainer(), bMatchExact, bIncludeDynamicTags);
}
