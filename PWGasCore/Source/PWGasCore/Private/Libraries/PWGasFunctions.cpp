// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "Libraries/PWGasFunctions.h"

#include "AbilitySystemGlobals.h"
#include "Ability/PWAffectable.h"
#include "Ability/PWEffectReceiver.h"
#include "Ability/PWSpellParams.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Effect.h"
#include "GAS/ASC/PWAbilitySystemComponent.h"
#include "Core/PWTeamsSubsystem.h"
#include "Core/PWTeamsTags.h"
#include "Components/SkeletalMeshComponent.h"

bool UPWGasFunctions::ApplyEffects(UPWAbilitySystemComponent* ASC, const UPWSpellParams* SpellParams)
{
	if (!ASC) return false;
	if (!SpellParams) return false;
	
	UPWEffectSpellParams* EffectParams = SpellParams->FindExtension<UPWEffectSpellParams>();
	if (!EffectParams) return false;
	
	UAbilitySystemComponent* SourceASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(SpellParams->SourceAvatar);
	EffectParams->ApplyEffectsToTarget(SourceASC ? SourceASC : ASC, ASC, SpellParams->AbilityLevel);
	return true;
}

TArray<FPWEffectModifierValue> UPWGasFunctions::GetModifierValuesFromEffects(const UPWSpellParams* SpellParams, const FGameplayAttribute& Attribute, const AActor* Target)
{
	TArray<FPWEffectModifierValue> Values;
	if (!SpellParams) return Values;

	const UPWEffectSpellParams* EffectParams = SpellParams->FindExtension<UPWEffectSpellParams>();
	if (!EffectParams) return Values;

	const UPWTeamsSubsystem* Teams = UPWTeamsSubsystem::Get(Target);
	const FPWTeamIdentity SourceIdentity = SpellParams->GetSourceIdentity();
	const FPWTeamIdentity TargetIdentity = UPWTeamsSubsystem::GetIdentity(Target);

	for (const FPWEffectGroup& Group : EffectParams->Groups)
	{
		if (Teams && !Teams->MatchesFilter(SourceIdentity, TargetIdentity, Group.TargetFilter))
			continue;

		for (const FPWEffectEntry& Effect : Group.Effects)
		{
			const UGameplayEffect* GE = Effect.EffectClass ? Effect.EffectClass.GetDefaultObject() : nullptr;
			if (!GE) continue;

			for (const FGameplayModifierInfo& Modifier : GE->Modifiers)
			{
				if (Modifier.Attribute != Attribute)
					continue;

				FPWEffectModifierValue Emv;
				Emv.EffectClass = Effect.EffectClass;
				Emv.DurationType = GE->DurationPolicy;
				Emv.Period = GE->Period.GetValueAtLevel(SpellParams->AbilityLevel);
				if (GE->DurationPolicy == EGameplayEffectDurationType::HasDuration)
					GE->DurationMagnitude.GetStaticMagnitudeIfPossible(SpellParams->AbilityLevel, Emv.Duration);
				Modifier.ModifierMagnitude.GetStaticMagnitudeIfPossible(SpellParams->AbilityLevel, Emv.Value);
				Values.Add(Emv);
				break;
			}
		}
	}
	return Values;
}

FPWTeamTargetFilter UPWGasFunctions::FilterFromLegacyMask(const int32 Mask)
{
	FPWTeamTargetFilter Filter;
	if ((Mask & 15) == 15)
		return Filter;

	if (Mask & 1) Filter.AnyOf.AddTag(PWTeamTags::Relation_Self);
	if (Mask & 2) Filter.AnyOf.AddTag(PWTeamTags::Relation_Ally);
	if (Mask & 4) Filter.AnyOf.AddTag(PWTeamTags::Relation_Enemy);
	if (Mask & 8)
	{
		Filter.AnyOf.AddTag(PWTeamTags::Relation_Neutral);
		Filter.AnyOf.AddTag(PWTeamTags::Relation_Unaffiliated);
	}

	// A mask with nothing ticked reached nobody; an empty filter would reach everybody.
	if (Filter.AnyOf.IsEmpty())
		Filter.NoneOf.AddTag(PWTeamTags::Relation);
	return Filter;
}

UMeshComponent* UPWGasFunctions::FindSocketMesh(const AActor* Actor, const FName Socket)
{
	if (!Actor || Socket.IsNone())
		return nullptr;

	TInlineComponentArray<UMeshComponent*> Meshes(Actor);
	Meshes.StableSort([](const UMeshComponent& A, const UMeshComponent& B) { return A.IsA<USkeletalMeshComponent>() && !B.IsA<USkeletalMeshComponent>(); });
	for (UMeshComponent* Mesh : Meshes)
		if (Mesh->DoesSocketExist(Socket))
			return Mesh;
	return nullptr;
}
