// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/ASC/PWASC_AbilityLifecycle.h"

#include "GameplayAbilitySpec.h"
#include "PWGASCoreLogChannels.h"
#include "GAS/Abilities/PWAbilityInfo.h"
#include "GAS/Abilities/PWAbilitySet.h"
#include "GAS/ASC/PWAbilitySystemComponent.h"
#include "GAS/ASC/PWASC_DataManagement.h"
#include "GAS/Tags/GASCoreTags.h"
#include "Settings/PWGASCoreSettings.h"

FPWASC_AbilityLifecycle::FPWASC_AbilityLifecycle(UPWAbilitySystemComponent& InASC) : ASC(InASC)
{
	FMemory::Memset(ActivationGroupCounts, 0, sizeof(ActivationGroupCounts));
}

void FPWASC_AbilityLifecycle::AddAbilities(const FGameplayTagContainer& AbilityTags) const
{
	for (const FGameplayTag& Tag : AbilityTags)
	{
		AddAbility(Tag);
	}
	AbilitiesUpdated.Broadcast();
}

void FPWASC_AbilityLifecycle::AddAbilitySet(const UPWAbilitySet* AbilitySet) const
{
	if (!AbilitySet) return;

	for (const FPWAbilitySetEntry& Entry : AbilitySet->Abilities)
		AddAbility(Entry.AbilityTag, Entry.InputTag);
	AbilitiesUpdated.Broadcast();
}

void FPWASC_AbilityLifecycle::AddAbility(const FGameplayTag& AbilityTag, const FGameplayTag& InputTag) const
{
	const FPWAbilityInfo AbilityData = UPWGASCoreSettings::Get()->GetAbilityByTag(AbilityTag);
	if (!AbilityData.IsValid() || !AbilityData.AbilityClass)
	{
		UE_LOG(LogPWGASCoreAbilitySystem, Warning, TEXT("%s >> AbilityData is invalid!! Tag: %s"), *FString(__FUNCTION__), *AbilityTag.ToString());
		return;
	}

	FGameplayAbilitySpec Spec = FGameplayAbilitySpec(AbilityData.AbilityClass, 1);
	FGameplayAbilitySpecHandle SpecHandle = ASC.GiveAbility(Spec);

	if (InputTag.IsValid())
		ASC.Input().EquipAbility(AbilityTag, InputTag);

	bool Activated = ApplyActivationPolicies(SpecHandle, PWTags::Ability::Activation::OnGranted);

	AbilityAdded.Broadcast(AbilityTag);
}

void FPWASC_AbilityLifecycle::RemoveAbility(const FGameplayTag& AbilityTag) const
{
	FGameplayAbilitySpec* Spec = ASC.Data().GetSpecOfAbility(AbilityTag);
	if (!Spec) return;

	if (ApplyDeactivationPolicies(*Spec, PWTags::Ability::Deactivation::OnRemoved))
		AbilityEnded.Broadcast(AbilityTag);

	ASC.ClearAbility(Spec->Handle);

	AbilityRemoved.Broadcast(AbilityTag);
}

void FPWASC_AbilityLifecycle::UpgradeAbility(const FGameplayTag& AbilityTag)
{
	FGameplayAbilitySpec* Spec = ASC.Data().GetSpecOfAbility(AbilityTag);
	if (!Spec) return;

	Spec->Level++;

	AbilityUpdated.Broadcast(AbilityTag);
	ASC.MarkAbilitySpecDirty(*Spec);
}

void FPWASC_AbilityLifecycle::DowngradeAbility(const FGameplayTag& AbilityTag)
{
	FGameplayAbilitySpec* Spec = ASC.Data().GetSpecOfAbility(AbilityTag);
	if (!Spec) return;

	Spec->Level--;

	AbilityUpdated.Broadcast(AbilityTag);
	ASC.MarkAbilitySpecDirty(*Spec);
}

void FPWASC_AbilityLifecycle::RemoveGrantedAbility(const FGameplayAbilitySpecHandle& Handle) const
{
	if (!ASC.FindAbilitySpecFromHandle(Handle)) return;

	ASC.CancelAbilityHandle(Handle);
	ASC.MarkAbilitySpecDirty(*ASC.FindAbilitySpecFromHandle(Handle));
	ASC.ClearAbility(Handle);
}

bool FPWASC_AbilityLifecycle::HasAbility(const FGameplayTag& AbilityTag) const
{
	return ASC.Data().GetSpecOfAbility(AbilityTag) ? true : false;
}

int32 FPWASC_AbilityLifecycle::GetAbilityLevel(const FGameplayTag& AbilityTag) const
{
	if (const FGameplayAbilitySpec* Spec = ASC.Data().GetSpecOfAbility(AbilityTag))
		return Spec->Level;
	return 0;
}

bool FPWASC_AbilityLifecycle::IsAbilityMaxLevel(const FGameplayTag& AbilityTag) const
{
	const FPWAbilityInfo AbilityData = UPWGASCoreSettings::Get()->GetAbilityByTag(AbilityTag);
	const FGameplayAbilitySpec* Spec = ASC.Data().GetSpecOfAbility(AbilityTag);
	if (!Spec) return false;
	if (Spec->Level == AbilityData.MaxLevel) return true;
	return false;
}

void FPWASC_AbilityLifecycle::ForEachAbility(const FForEachAbility& Delegate) const
{
	FScopedAbilityListLock ActiveScopeLock(ASC);
	for (const FGameplayAbilitySpec& AbilitySpec : ASC.GetActivatableAbilities())
		if (!Delegate.ExecuteIfBound(AbilitySpec))
			UE_LOG(LogPWGASCoreAbilitySystem, Warning, TEXT("Failed to execute delegate"));
}

bool FPWASC_AbilityLifecycle::ApplyActivationPolicies(const FGameplayAbilitySpecHandle& SpecHandle, const FGameplayTag& EventTag) const
{
	FGameplayAbilitySpec* AbilitySpec = ASC.FindAbilitySpecFromHandle(SpecHandle);
	if (AbilitySpec && AbilitySpec->Ability && AbilitySpec->Ability->GetAssetTags().HasAnyExact(EventTag.GetSingleTagContainer()))
	{
		if (!AbilitySpec->IsActive())
		{
			ASC.TryActivateAbility(SpecHandle);
			return true;
		}
	}
	return false;
}

bool FPWASC_AbilityLifecycle::ApplyActivationPolicies(const FGameplayAbilitySpec& AbilitySpec, const FGameplayTag& EventTag) const
{
	if (AbilitySpec.Ability && AbilitySpec.Ability->GetAssetTags().HasAnyExact(EventTag.GetSingleTagContainer()))
	{
		if (!AbilitySpec.IsActive())
		{
			ASC.TryActivateAbility(AbilitySpec.Handle);
			return true;
		}
	}
	return false;
}

void FPWASC_AbilityLifecycle::ApplyActivationPolicies(const FGameplayTag& EventTag) const
{
	for (FGameplayAbilitySpec* Spec : ASC.Data().GetSpecsWithTags(EventTag.GetSingleTagContainer(), /*exact*/true))
	{
		if (!Spec->IsActive())
		{
			ASC.TryActivateAbility(Spec->Handle);
		}
	}
}

bool FPWASC_AbilityLifecycle::ApplyDeactivationPolicies(const FGameplayAbilitySpecHandle& SpecHandle, const FGameplayTag& EventTag) const
{
	FGameplayAbilitySpec* AbilitySpec = ASC.FindAbilitySpecFromHandle(SpecHandle);
	if (AbilitySpec && AbilitySpec->Ability && AbilitySpec->Ability->GetAssetTags().HasAnyExact(EventTag.GetSingleTagContainer()))
	{
		if (AbilitySpec->IsActive())
		{
			ASC.CancelAbilityHandle(SpecHandle);
			return true;
		}
	}
	return false;
}

bool FPWASC_AbilityLifecycle::ApplyDeactivationPolicies(const FGameplayAbilitySpec& AbilitySpec, const FGameplayTag& EventTag) const
{
	if (AbilitySpec.Ability && AbilitySpec.Ability->GetAssetTags().HasAnyExact(EventTag.GetSingleTagContainer()))
	{
		if (AbilitySpec.IsActive())
		{
			ASC.CancelAbilityHandle(AbilitySpec.Handle);
			return true;
		}
	}
	return false;
}

void FPWASC_AbilityLifecycle::ApplyDeactivationPolicies(const FGameplayTag& EventTag) const
{
	// end abilities that have this deactivation policy
	for (FGameplayAbilitySpec* Spec : ASC.Data().GetSpecsWithTags(EventTag.GetSingleTagContainer(), /*exact*/true))
	{
		if (Spec->IsActive())
		{
			ASC.CancelAbilityHandle(Spec->Handle);
		}
	}
}

void FPWASC_AbilityLifecycle::CancelAbilitiesByFunc(const TShouldCancelAbilityFunc& ShouldCancelFunc, const bool bReplicateCancelAbility) const
{
	FScopedAbilityListLock ActiveScopeLock(ASC);
	for (const FGameplayAbilitySpec& AbilitySpec : ASC.GetActivatableAbilities())
	{
		if (!AbilitySpec.IsActive())
		{
			continue;
		}

		if (!AbilitySpec.Ability || !AbilitySpec.Ability->IsA<UPWGameplayAbilityBase>())
			continue;

		TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
		for (UGameplayAbility* Instance : Instances)
		{
			UPWGameplayAbilityBase* Ability = CastChecked<UPWGameplayAbilityBase>(Instance);
			if (!ShouldCancelFunc(Ability, AbilitySpec.Handle))
				continue;

			if (Ability->CanBeCanceled())
				Ability->CancelAbility(AbilitySpec.Handle, ASC.AbilityActorInfo.Get(), Ability->GetCurrentActivationInfo(), bReplicateCancelAbility);
			else
				UE_LOG(LogPWGASCoreAbilitySystem, Error, TEXT("CancelAbilitiesByFunc: Can't cancel ability [%s] because CanBeCanceled is false."), *Ability->GetName());
		}
	}
}

bool FPWASC_AbilityLifecycle::IsActivationGroupBlocked(EPWActivationGroup Group) const
{
	bool bBlocked = false;

	switch (Group)
	{
	case EPWActivationGroup::Independent:
		// Independent abilities are never blocked.
		bBlocked = false;
		break;

	case EPWActivationGroup::Exclusive_Replaceable:
	case EPWActivationGroup::Exclusive_Blocking:
		// Exclusive abilities can activate if nothing is blocking.
		bBlocked = (ActivationGroupCounts[(uint8)EPWActivationGroup::Exclusive_Blocking] > 0);
		break;

	default:
		checkf(false, TEXT("IsActivationGroupBlocked: Invalid ActivationGroup [%d]\n"), (uint8)Group);
		break;
	}

	return bBlocked;
}

void FPWASC_AbilityLifecycle::AddAbilityToActivationGroup(EPWActivationGroup Group, const UPWGameplayAbilityBase* Ability)
{
	check(Ability);
	check(ActivationGroupCounts[(uint8)Group] < INT32_MAX);

	ActivationGroupCounts[(uint8)Group]++;

	const bool bReplicateCancelAbility = false;

	switch (Group)
	{
	case EPWActivationGroup::Independent:
		// Independent abilities do not cancel any other abilities.
		break;

	case EPWActivationGroup::Exclusive_Replaceable:
	case EPWActivationGroup::Exclusive_Blocking:
		CancelActivationGroupAbilities(EPWActivationGroup::Exclusive_Replaceable, Ability, bReplicateCancelAbility);
		break;

	default:
		checkf(false, TEXT("AddAbilityToActivationGroup: Invalid ActivationGroup [%d]\n"), (uint8)Group);
		break;
	}

	const int32 ExclusiveCount = ActivationGroupCounts[(uint8)EPWActivationGroup::Exclusive_Replaceable] + ActivationGroupCounts[(uint8)EPWActivationGroup::Exclusive_Blocking];
	if (!ensure(ExclusiveCount <= 1))
	{
		UE_LOG(LogPWGASCoreAbilitySystem, Error, TEXT("AddAbilityToActivationGroup: Multiple exclusive abilities are running."));
	}
}

void FPWASC_AbilityLifecycle::RemoveAbilityFromActivationGroup(EPWActivationGroup Group, const UPWGameplayAbilityBase* Ability)
{
	check(Ability);
	check(ActivationGroupCounts[(uint8)Group] > 0);

	ActivationGroupCounts[(uint8)Group]--;
}

void FPWASC_AbilityLifecycle::CancelActivationGroupAbilities(EPWActivationGroup Group, const UPWGameplayAbilityBase* IgnoreAbility, bool bReplicateCancelAbility) const
{
	auto ShouldCancelFunc = [this, Group, IgnoreAbility](const UPWGameplayAbilityBase* Ability, FGameplayAbilitySpecHandle Handle)
	{
		return ((Ability->GetActivationGroup() == Group) && (Ability != IgnoreAbility));
	};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}