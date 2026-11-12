// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "PWGasCore/Public/GAS/Abilities/Modules/DataModules/PWAbilityModule_Effect.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "Ability/Affectable.h"
#include "Libraries/GASCoreFunctions.h"
#include "TeamFunctions.h"

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
	if (!UTeamFunctions::MatchesMask(SourceASC->GetAvatarActor(), TargetASC->GetAvatarActor(), TargetMask))
		return;

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

void UPWAbilityModule_Effect::ApplyEffectsToActor(UAbilitySystemComponent* SourceASC, AActor* Target)
{
	if (!SourceASC || !Target)
		return;

	if (UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target))
	{
		ApplyEffects(SourceASC, TargetASC);
		return;
	}

	UObject* Receiver = UGASCoreFunctions::FindAffectable(Target);
	if (!Receiver || !OwnerAbility || !UTeamFunctions::MatchesMask(SourceASC->GetAvatarActor(), Target, TargetMask))
		return;

	USpellParams* Params = OwnerAbility->MakeSpellParams();
	ExtendParams(Params);
	IAffectable::Execute_ApplyEffects(Receiver, Params);
}

void UPWAbilityModule_Effect::RemoveEffectsFromActor(UAbilitySystemComponent* SourceASC, AActor* Target, const bool bInfiniteOnly)
{
	if (!SourceASC || !Target)
		return;

	if (UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target))
	{
		RemoveEffects(SourceASC, TargetASC, bInfiniteOnly);
		return;
	}

	UObject* Receiver = UGASCoreFunctions::FindAffectable(Target);
	if (!Receiver || !OwnerAbility)
		return;

	USpellParams* Params = OwnerAbility->MakeSpellParams();
	ExtendParams(Params);
	IAffectable::Execute_RemoveEffects(Receiver, Params, bInfiniteOnly);
}
