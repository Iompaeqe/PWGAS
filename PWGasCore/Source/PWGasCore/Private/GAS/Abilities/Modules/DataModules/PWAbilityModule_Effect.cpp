// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "PWGasCore/Public/GAS/Abilities/Modules/DataModules/PWAbilityModule_Effect.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#if WITH_PWTEAMS
#include "TeamFunctions.h"
#endif

void UEffectSpellParamsExtension::ApplyEffectsToTarget(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC, const int32 AbilityLevel) const
{
	if (!SourceASC || !TargetASC)
		return;

	for (const FEffectEntry& Entry : Effects)
	{
		if (!Entry.EffectClass)
			continue;

		FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
		Context.AddSourceObject(SourceASC->GetAvatarActor());

		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(Entry.EffectClass, AbilityLevel, Context);
		if (!SpecHandle.IsValid())
			continue;

		if (FGameplayEffectSpec* Spec = SpecHandle.Data.Get())
		{
			if (Entry.StackCount > 1)
				Spec->SetStackCount(Entry.StackCount);
		}

		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}
}

void UPWAbilityModule_Effect::ExtendParams(USpellParams* Params) const
{
	if (!Params)
		return;

	UEffectSpellParamsExtension* Extension = Params->AddExtension<UEffectSpellParamsExtension>();
	Extension->Effects = Effects;
	Params->TargetEffectMask = TargetMask;
}

void UPWAbilityModule_Effect::ApplyEffects(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC)
{
	if (!SourceASC || !TargetASC)
		return;
#if WITH_PWTEAMS
	if (!UTeamFunctions::MatchesMask(SourceASC->GetAvatarActor(), TargetASC->GetAvatarActor(), TargetMask))
		return;
#endif

	for (const FEffectEntry& Entry : Effects)
	{
		if (!Entry.EffectClass)
			continue;

		FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
		Context.AddSourceObject(SourceASC->GetAvatarActor());

		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(Entry.EffectClass, OwnerAbility->GetAbilityLevel(), Context);
		if (!SpecHandle.IsValid())
			continue;

		if (FGameplayEffectSpec* Spec = SpecHandle.Data.Get())
		{
			if (Entry.StackCount > 1)
				Spec->SetStackCount(Entry.StackCount);
		}

		FActiveGameplayEffectHandle Handle = SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);

		if (Handle.IsValid())
			ActiveEffectHandles.FindOrAdd(TargetASC).Add(Handle);
	}
}

void UPWAbilityModule_Effect::RemoveEffects(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC, const bool bInfiniteOnly)
{
	if (!SourceASC || !TargetASC)
		return;

	if (TArray<FActiveGameplayEffectHandle>* Handles = ActiveEffectHandles.Find(TargetASC))
	{
		for (const FActiveGameplayEffectHandle& Handle : *Handles)
		{
			const FActiveGameplayEffect* Active = TargetASC->GetActiveGameplayEffect(Handle);
			if (bInfiniteOnly && Active && Active->GetDuration() != FGameplayEffectConstants::INFINITE_DURATION)
				continue;

			TargetASC->RemoveActiveGameplayEffect(Handle);
		}
		ActiveEffectHandles.Remove(TargetASC);
	}
}
