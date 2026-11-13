// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "PWGasCore/Public/GAS/Abilities/Modules/DataModules/PWAbilityModule_Effect.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "Ability/Affectable.h"
#include "Libraries/GASCoreFunctions.h"
#include "TeamFunctions.h"

namespace
{
	FActiveGameplayEffectHandle ApplyEntry(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC, const FEffectEntry& Entry, const int32 Level)
	{
		if (!Entry.EffectClass)
			return FActiveGameplayEffectHandle();

		FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
		Context.AddSourceObject(SourceASC->GetAvatarActor());

		const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(Entry.EffectClass, Level, Context);
		if (!SpecHandle.IsValid())
			return FActiveGameplayEffectHandle();

		if (Entry.StackCount > 1)
			SpecHandle.Data->SetStackCount(Entry.StackCount);

		return SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}
}

void UEffectSpellParamsExtension::ApplyEffectsToTarget(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC, const int32 AbilityLevel) const
{
	if (!SourceASC || !TargetASC)
		return;

	for (const FPWEffectGroup& Group : Groups)
	{
		if (!UTeamFunctions::MatchesMask(SourceASC->GetAvatarActor(), TargetASC->GetAvatarActor(), Group.TargetMask))
			continue;
		for (const FEffectEntry& Entry : Group.Effects)
			ApplyEntry(SourceASC, TargetASC, Entry, AbilityLevel);
	}
}

void UPWAbilityModule_Effect::PostLoad()
{
	Super::PostLoad();

	// Abilities saved before effect groups kept a single mask and list.
	if (Effects_DEPRECATED.IsEmpty())
		return;

	FPWEffectGroup& Group = EffectGroups.AddDefaulted_GetRef();
	Group.TargetMask = TargetMask_DEPRECATED;
	Group.Effects = MoveTemp(Effects_DEPRECATED);
	Effects_DEPRECATED.Reset();
}

int32 UPWAbilityModule_Effect::GetCombinedMask() const
{
	int32 Mask = 0;
	for (const FPWEffectGroup& Group : EffectGroups)
		Mask |= Group.TargetMask;
	return Mask;
}

void UPWAbilityModule_Effect::ExtendParams(USpellParams* Params) const
{
	if (!Params)
		return;

	UEffectSpellParamsExtension* Extension = Params->AddExtension<UEffectSpellParamsExtension>();
	Extension->Groups = EffectGroups;
	Params->TargetEffectMask = GetCombinedMask();
}

void UPWAbilityModule_Effect::ApplyEffects(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC)
{
	if (!SourceASC || !TargetASC)
		return;

	for (const FPWEffectGroup& Group : EffectGroups)
	{
		if (!UTeamFunctions::MatchesMask(SourceASC->GetAvatarActor(), TargetASC->GetAvatarActor(), Group.TargetMask))
			continue;

		for (const FEffectEntry& Entry : Group.Effects)
			if (const FActiveGameplayEffectHandle Handle = ApplyEntry(SourceASC, TargetASC, Entry, OwnerAbility->GetAbilityLevel()); Handle.IsValid())
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
	if (!Receiver || !OwnerAbility || !UTeamFunctions::MatchesMask(SourceASC->GetAvatarActor(), Target, GetCombinedMask()))
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
