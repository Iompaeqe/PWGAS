// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "Libraries/PWGasFunctions.h"

#include "AbilitySystemGlobals.h"
#include "Ability/PWAffectable.h"
#include "Ability/PWEffectReceiver.h"
#include "Ability/PWSpellParams.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Effect.h"
#include "GAS/ASC/PWAbilitySystemComponent.h"
#include "TeamFunctions.h"

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

	for (const FPWEffectGroup& Group : EffectParams->Groups)
	{
		if (Target && !UTeamFunctions::MatchesMask(SpellParams->SourceAvatar, Target, Group.TargetMask))
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
