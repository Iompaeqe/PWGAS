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

namespace PWEffectModuleDebug
{
	static FString EffectName(const UClass* EffectClass)
	{
		FString Name = GetNameSafe(EffectClass);
		Name.RemoveFromEnd(TEXT("_C"));
		return Name;
	}

	static FString ActorName(const AActor* Actor)
	{
		return Actor ? Actor->GetActorNameOrLabel() : FString(TEXT("none"));
	}

	static FVector AboveActor(const AActor* Actor)
	{
		return Actor->GetActorLocation() + FVector(0.f, 0.f, Actor->GetSimpleCollisionHalfHeight() + 40.f);
	}

	static FString DescribeEffects(const FPWEffectGroup& Group)
	{
		TArray<FString> Names;
		for (const FPWEffectEntry& Entry : Group.Effects)
			Names.Add(FString::Printf(TEXT("%s x%d"), *EffectName(Entry.EffectClass), Entry.StackCount));
		return Names.IsEmpty() ? FString(TEXT("no effects")) : FString::Join(Names, TEXT(", "));
	}

	static FString DescribeFilter(const FPWTeamTargetFilter& Filter)
	{
		if (Filter.IsEmpty())
			return TEXT("everyone");

		TArray<FString> Parts;
		if (!Filter.AnyOf.IsEmpty())
			Parts.Add(TEXT("any of ") + Filter.AnyOf.ToStringSimple());
		if (!Filter.AllOf.IsEmpty())
			Parts.Add(TEXT("all of ") + Filter.AllOf.ToStringSimple());
		if (!Filter.NoneOf.IsEmpty())
			Parts.Add(TEXT("none of ") + Filter.NoneOf.ToStringSimple());
		return FString::Join(Parts, TEXT(", "));
	}

	// Instant effects report success without a handle, so only the others can be removed later.
	static FString DescribeHandle(const UAbilitySystemComponent* TargetASC, const FActiveGameplayEffectHandle& Handle)
	{
		if (const FActiveGameplayEffect* Active = Handle.IsValid() ? TargetASC->GetActiveGameplayEffect(Handle) : nullptr)
			return Active->GetDuration() == FGameplayEffectConstants::INFINITE_DURATION
				? FString(TEXT("applied, infinite, handle kept for removal"))
				: FString::Printf(TEXT("applied for %.1f s, handle kept for removal"), Active->GetDuration());
		if (Handle.IsValid())
			return TEXT("applied, handle kept for removal");
		return Handle.WasSuccessfullyApplied() ? TEXT("executed, instant so no handle") : TEXT("NOT applied: immune, blocked or not predicted here");
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

void UPWAbilityModule_Effect::OnAbilityActivated_Implementation()
{
	Super::OnAbilityActivated_Implementation();
	if (!IsDebugging())
		return;

	TArray<FString> Groups;
	for (int32 GroupIndex = 0; GroupIndex < EffectGroups.Num(); ++GroupIndex)
		Groups.Add(FString::Printf(TEXT("%d [%s] for %s"), GroupIndex, *PWEffectModuleDebug::DescribeEffects(EffectGroups[GroupIndex]), *PWEffectModuleDebug::DescribeFilter(EffectGroups[GroupIndex].TargetFilter)));
	DebugPrint(FString::Printf(TEXT("%d effect group(s) at level %d%s%s"), EffectGroups.Num(), OwnerAbility->GetAbilityLevel(), Groups.IsEmpty() ? TEXT("") : TEXT(": "), *FString::Join(Groups, TEXT("; "))));
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

	const bool bDebug = IsDebugging();
	TArray<FString> DebugApplied;
	for (int32 GroupIndex = 0; GroupIndex < EffectGroups.Num(); ++GroupIndex)
	{
		const FPWEffectGroup& Group = EffectGroups[GroupIndex];
		if (!UPWTeamsSubsystem::ActorMatchesFilter(SourceASC->GetAvatarActor(), TargetASC->GetAvatarActor(), Group.TargetFilter))
		{
			if (bDebug)
				DebugPrint(FString::Printf(TEXT("skipped %s: group %d filter (%s)"), *PWEffectModuleDebug::ActorName(TargetASC->GetAvatarActor()), GroupIndex, *PWEffectModuleDebug::DescribeFilter(Group.TargetFilter)), FLinearColor::Gray);
			continue;
		}

		for (const FPWEffectEntry& Entry : Group.Effects)
		{
			const int32 Level = OwnerAbility->GetAbilityLevel();
			const FActiveGameplayEffectHandle Handle = ApplyEntry(SourceASC, TargetASC, Entry, Level);
			if (Handle.IsValid())
				ActiveEffectHandles.FindOrAdd(TargetASC).Add(Handle);
			if (!bDebug)
				continue;

			DebugPrint(FString::Printf(TEXT("%s x%d at level %d -> %s (group %d, ability system): %s"), *PWEffectModuleDebug::EffectName(Entry.EffectClass), Entry.StackCount, Level,
				*PWEffectModuleDebug::ActorName(TargetASC->GetAvatarActor()), GroupIndex, Entry.EffectClass ? *PWEffectModuleDebug::DescribeHandle(TargetASC, Handle) : TEXT("no effect class set")),
				Handle.WasSuccessfullyApplied() ? FLinearColor::Green : FLinearColor::Red);
			if (Handle.WasSuccessfullyApplied())
				DebugApplied.Add(PWEffectModuleDebug::EffectName(Entry.EffectClass));
		}
	}

	if (bDebug && !DebugApplied.IsEmpty() && TargetASC->GetAvatarActor())
		DebugText(PWEffectModuleDebug::AboveActor(TargetASC->GetAvatarActor()), FString::Join(DebugApplied, TEXT(", ")), PWModuleDebug::Target);
}

void UPWAbilityModule_Effect::RemoveEffects(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC, const bool bInfiniteOnly)
{
	if (!SourceASC || !TargetASC)
		return;

	const bool bDebug = IsDebugging();
	TArray<FActiveGameplayEffectHandle>* Handles = ActiveEffectHandles.Find(TargetASC);
	if (!Handles)
	{
		if (bDebug)
			DebugPrint(FString::Printf(TEXT("nothing to remove from %s: no effects tracked on it"), *PWEffectModuleDebug::ActorName(TargetASC->GetAvatarActor())), FLinearColor::Gray);
		return;
	}

	const int32 Tracked = Handles->Num();
	TArray<FString> DebugRemoved;
	int32 DebugLeft = 0;
	for (const FActiveGameplayEffectHandle& Handle : *Handles)
	{
		const FActiveGameplayEffect* Active = TargetASC->GetActiveGameplayEffect(Handle);
		if (bInfiniteOnly && Active && Active->GetDuration() != FGameplayEffectConstants::INFINITE_DURATION)
		{
			++DebugLeft;
			continue;
		}

		FString DebugName;
		if (bDebug && Active)
			DebugName = PWEffectModuleDebug::EffectName(Active->Spec.Def ? Active->Spec.Def->GetClass() : nullptr);
		if (TargetASC->RemoveActiveGameplayEffect(Handle) && bDebug)
			DebugRemoved.Add(DebugName);
	}
	ActiveEffectHandles.Remove(TargetASC);

	if (!bDebug)
		return;

	FString Message = FString::Printf(TEXT("removed %d of %d tracked effect(s) from %s (%s)"), DebugRemoved.Num(), Tracked, *PWEffectModuleDebug::ActorName(TargetASC->GetAvatarActor()), bInfiniteOnly ? TEXT("infinite only") : TEXT("all"));
	if (!DebugRemoved.IsEmpty())
		Message += TEXT(": ") + FString::Join(DebugRemoved, TEXT(", "));
	if (DebugLeft > 0)
		Message += FString::Printf(TEXT(", %d timed left to run out"), DebugLeft);
	if (const int32 Gone = Tracked - DebugRemoved.Num() - DebugLeft; Gone > 0)
		Message += FString::Printf(TEXT(", %d already gone"), Gone);
	DebugPrint(Message);

	if (!DebugRemoved.IsEmpty() && TargetASC->GetAvatarActor())
		DebugText(PWEffectModuleDebug::AboveActor(TargetASC->GetAvatarActor()), TEXT("removed ") + FString::Join(DebugRemoved, TEXT(", ")), PWModuleDebug::Text);
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
	{
		if (IsDebugging())
			DebugPrint(FString::Printf(TEXT("skipped %s: %s"), *PWEffectModuleDebug::ActorName(Target), Receiver ? TEXT("no group's filter accepts it") : TEXT("no ability system or IPWAffectable receiver")), FLinearColor::Gray);
		return;
	}

	UPWSpellParams* Params = OwnerAbility->MakeSpellParams();
	ExtendParams(Params);
	const bool bApplied = IPWAffectable::Execute_ApplyEffects(Receiver, Params);
	if (!IsDebugging())
		return;

	TArray<FString> Groups;
	TArray<FString> Passed;
	for (int32 GroupIndex = 0; GroupIndex < EffectGroups.Num(); ++GroupIndex)
	{
		const FPWEffectGroup& Group = EffectGroups[GroupIndex];
		if (!UPWTeamsSubsystem::ActorMatchesFilter(SourceASC->GetAvatarActor(), Target, Group.TargetFilter))
		{
			Groups.Add(FString::Printf(TEXT("skipped group %d filter"), GroupIndex));
			continue;
		}
		Groups.Add(FString::Printf(TEXT("group %d [%s]"), GroupIndex, *PWEffectModuleDebug::DescribeEffects(Group)));
		Passed.Add(PWEffectModuleDebug::DescribeEffects(Group));
	}
	DebugPrint(FString::Printf(TEXT("%s at level %d -> %s (IPWAffectable receiver %s): %s"), *FString::Join(Groups, TEXT("; ")), OwnerAbility->GetAbilityLevel(), *PWEffectModuleDebug::ActorName(Target),
		*GetNameSafe(Receiver), bApplied ? TEXT("receiver applied them") : TEXT("receiver applied nothing")), bApplied ? FLinearColor::Green : FLinearColor::Red);
	if (bApplied)
		DebugText(PWEffectModuleDebug::AboveActor(Target), FString::Join(Passed, TEXT(", ")), PWModuleDebug::Target);
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
	{
		if (IsDebugging())
			DebugPrint(FString::Printf(TEXT("nothing to remove from %s: no ability system or IPWAffectable receiver"), *PWEffectModuleDebug::ActorName(Target)), FLinearColor::Gray);
		return;
	}

	UPWSpellParams* Params = OwnerAbility->MakeSpellParams();
	ExtendParams(Params);
	IPWAffectable::Execute_RemoveEffects(Receiver, Params, bInfiniteOnly);
	if (IsDebugging())
		DebugPrint(FString::Printf(TEXT("asked %s's IPWAffectable receiver %s to remove %s"), *PWEffectModuleDebug::ActorName(Target), *GetNameSafe(Receiver), bInfiniteOnly ? TEXT("the infinite effects") : TEXT("all effects")));
}
