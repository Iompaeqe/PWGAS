// copyright mkurtt96


#include "Libraries/GASCoreFunctions.h"

#include "AbilitySystemGlobals.h"
#include "Ability/SpellParams.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Effect.h"
#include "GAS/ASC/PWAbilitySystemComponent.h"

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

TArray<FEffectModifierValue> UGASCoreFunctions::GetModifierValuesFromEffects(const USpellParams* SpellParams, const FGameplayAttribute& IncomingDamageAttr)
{
	TArray<FEffectModifierValue> Values;
	if (!SpellParams) return Values;
	UEffectSpellParamsExtension* EffectParams = SpellParams->FindExtension<UEffectSpellParamsExtension>();
	if (!EffectParams) return Values;
	
	for (FEffectEntry Effect : EffectParams->Effects)
	{
		UGameplayEffect* GE = Effect.EffectClass.GetDefaultObject();
		FEffectModifierValue Emv = FEffectModifierValue();
		
		if (GE->DurationPolicy != EGameplayEffectDurationType::Instant)
		{
			GE->DurationMagnitude.GetStaticMagnitudeIfPossible(SpellParams->AbilityLevel, Emv.Value);
			Emv.Period = GE->Period.GetValueAtLevel(SpellParams->AbilityLevel);
		}
		for (FGameplayModifierInfo Modifier : GE->Modifiers)
		{
			if (Modifier.Attribute == IncomingDamageAttr)
			{
				Modifier.ModifierMagnitude.GetStaticMagnitudeIfPossible(SpellParams->AbilityLevel, Emv.Value);
				break;
			}
		}
		
		Values.Add(Emv);
	}
	
	return Values;
}