// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "PWGasCore/Public/GAS/Abilities/Modules/DataModules/PWAbilityModule_Effect.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "Ability/PWAffectable.h"
#include "Libraries/PWGasFunctions.h"
#include "Core/PWTeamsSubsystem.h"
#include "Libraries/PWGasFunctions.h"

namespace
{
	FActiveGameplayEffectHandle ApplyEntry(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC, const FPWEffectEntry& Entry, const int32 Level)
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

void UPWEffectSpellParams::ApplyEffectsToTarget(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC, const int32 AbilityLevel) const
{
	if (!SourceASC || !TargetASC)
		return;

	// The params know the caster even when it is gone and SourceASC is only a stand-in.
	const UPWSpellParams* Params = GetTypedOuter<UPWSpellParams>();
	const UPWTeamsSubsystem* Teams = UPWTeamsSubsystem::Get(TargetASC);
	const FPWTeamIdentity Source = Params ? Params->GetSourceIdentity() : UPWTeamsSubsystem::GetIdentity(SourceASC->GetAvatarActor());
	const FPWTeamIdentity Target = UPWTeamsSubsystem::GetIdentity(TargetASC->GetAvatarActor());

	for (const FPWEffectGroup& Group : Groups)
	{
		if (Teams && !Teams->MatchesFilter(Source, Target, Group.TargetFilter))
			continue;
		for (const FPWEffectEntry& Entry : Group.Effects)
			ApplyEntry(SourceASC, TargetASC, Entry, AbilityLevel);
	}
}

void UPWAbilityModule_Effect::PostLoad()
{
	Super::PostLoad();

	// Abilities saved before effect groups kept a single mask and list.
	if (!Effects_DEPRECATED.IsEmpty())
	{
		FPWEffectGroup& Group = EffectGroups.AddDefaulted_GetRef();
		Group.TargetMask_DEPRECATED = TargetMask_DEPRECATED;
		Group.Effects = MoveTemp(Effects_DEPRECATED);
		Effects_DEPRECATED.Reset();
	}

	// Abilities saved before tag filters kept a number per group.
	for (FPWEffectGroup& Group : EffectGroups)
	{
		if (Group.TargetMask_DEPRECATED == 15)
			continue;
		if (Group.TargetFilter.IsEmpty())
			Group.TargetFilter = UPWGasFunctions::FilterFromLegacyMask(Group.TargetMask_DEPRECATED);
		Group.TargetMask_DEPRECATED = 15;
	}
}

bool UPWAbilityModule_Effect::AppliesTo(const AActor* Source, const AActor* Target) const
{
	for (const FPWEffectGroup& Group : EffectGroups)
		if (UPWTeamsSubsystem::ActorMatchesFilter(Source, Target, Group.TargetFilter))
			return true;
	return false;
}

void UPWAbilityModule_Effect::ExtendParams(UPWSpellParams* Params) const
{
	if (!Params)
		return;

	UPWEffectSpellParams* Extension = Params->AddExtension<UPWEffectSpellParams>();
	Extension->Groups = EffectGroups;
}

void UPWAbilityModule_Effect::ApplyEffects(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC)
{
	if (!SourceASC || !TargetASC)
		return;

	for (const FPWEffectGroup& Group : EffectGroups)
	{
		if (!UPWTeamsSubsystem::ActorMatchesFilter(SourceASC->GetAvatarActor(), TargetASC->GetAvatarActor(), Group.TargetFilter))
			continue;

		for (const FPWEffectEntry& Entry : Group.Effects)
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

	UObject* Receiver = IPWAffectable::Find(Target);
	if (!Receiver || !OwnerAbility || !AppliesTo(SourceASC->GetAvatarActor(), Target))
		return;

	UPWSpellParams* Params = OwnerAbility->MakeSpellParams();
	ExtendParams(Params);
	IPWAffectable::Execute_ApplyEffects(Receiver, Params);
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

	UObject* Receiver = IPWAffectable::Find(Target);
	if (!Receiver || !OwnerAbility)
		return;

	UPWSpellParams* Params = OwnerAbility->MakeSpellParams();
	ExtendParams(Params);
	IPWAffectable::Execute_RemoveEffects(Receiver, Params, bInfiniteOnly);
}
