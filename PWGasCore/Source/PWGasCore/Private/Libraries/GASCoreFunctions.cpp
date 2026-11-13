// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "Libraries/GASCoreFunctions.h"

#include "AbilitySystemGlobals.h"
#include "Ability/Affectable.h"
#include "Ability/EffectReceiver.h"
#include "Ability/SpellParams.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Effect.h"
#include "GAS/ASC/PWAbilitySystemComponent.h"
#include "TeamFunctions.h"

bool UGASCoreFunctions::ApplyEffects(UPWAbilitySystemComponent* ASC, const USpellParams* SpellParams)
{
	if (!ASC) return false;
	if (!SpellParams) return false;
	
	UEffectSpellParamsExtension* EffectParams = SpellParams->FindExtension<UEffectSpellParamsExtension>();
	if (!EffectParams) return false;
	
	UAbilitySystemComponent* SourceASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(SpellParams->SourceAvatar);
	EffectParams->ApplyEffectsToTarget(SourceASC ? SourceASC : ASC, ASC, SpellParams->AbilityLevel);
	return true;
}

UObject* UGASCoreFunctions::FindAffectable(AActor* Target)
{
	if (!Target) return nullptr;

	UObject* Receiver = Target->Implements<UEffectReceiver>() ? IEffectReceiver::Execute_GetEffectReceiver(Target) : Target;
	return Receiver && Receiver->Implements<UAffectable>() ? Receiver : nullptr;
}

TArray<FEffectModifierValue> UGASCoreFunctions::GetModifierValuesFromEffects(const USpellParams* SpellParams, const FGameplayAttribute& Attribute, const AActor* Target)
{
	TArray<FEffectModifierValue> Values;
	if (!SpellParams) return Values;

	const UEffectSpellParamsExtension* EffectParams = SpellParams->FindExtension<UEffectSpellParamsExtension>();
	if (!EffectParams) return Values;

	for (const FPWEffectGroup& Group : EffectParams->Groups)
	{
		if (Target && !UTeamFunctions::MatchesMask(SpellParams->SourceAvatar, Target, Group.TargetMask))
			continue;

		for (const FEffectEntry& Effect : Group.Effects)
		{
			const UGameplayEffect* GE = Effect.EffectClass ? Effect.EffectClass.GetDefaultObject() : nullptr;
			if (!GE) continue;

			for (const FGameplayModifierInfo& Modifier : GE->Modifiers)
			{
				if (Modifier.Attribute != Attribute)
					continue;

				FEffectModifierValue Emv;
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
