// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "Ability/SpellParams.h"
#include "Libraries/GASCoreFunctions.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

static UAbilitySystemComponent* GetEffectInstigator(const UPWAbilitySystemComponent* ASC)
{
	FGameplayEffectQuery Query;
	Query.EffectDefinition = UPWTestEffect::StaticClass();
	const TArray<FActiveGameplayEffectHandle> Handles = ASC->GetActiveEffects(Query);
	if (Handles.Num() == 0)
		return nullptr;
	return ASC->GetActiveGameplayEffect(Handles[0])->Spec.GetContext().GetOriginalInstigatorAbilitySystemComponent();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGASCoreFunctionsApplyEffectsTest, "PW.Libraries.GASCoreFunctions.ApplyEffects", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGASCoreFunctionsApplyEffectsTest::RunTest(const FString& Parameters)
{
	FPWTestWorld Test;
	UPWAbilitySystemComponent* Target = Test.SpawnASC();
	UPWAbilitySystemComponent* Other = Test.SpawnASC();

	USpellParams* Params = NewObject<USpellParams>();
	Params->AbilityLevel = 1;
	Params->AddExtension<UEffectSpellParamsExtension>()->Effects.Add({UPWTestEffect::StaticClass(), 1});

	Params->SourceAvatar = Test.Actor;
	TestTrue("Applied", UGASCoreFunctions::ApplyEffects(Target, Params));
	TestEqual("Effect on target", Target->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 1);
	TestTrue("Caster is the instigator", GetEffectInstigator(Target) == Test.ASC);

	Params->SourceAvatar = nullptr;
	TestTrue("Applied without a source", UGASCoreFunctions::ApplyEffects(Other, Params));
	TestTrue("Target is its own instigator", GetEffectInstigator(Other) == Other);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGASCoreFunctionsModifierValuesTest, "PW.Libraries.GASCoreFunctions.ModifierValues", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGASCoreFunctionsModifierValuesTest::RunTest(const FString& Parameters)
{
	USpellParams* Params = NewObject<USpellParams>();
	Params->AbilityLevel = 1;
	UEffectSpellParamsExtension* Effects = Params->AddExtension<UEffectSpellParamsExtension>();
	Effects->Effects.Add({UPWTestPeriodicDamageEffect::StaticClass(), 1});
	Effects->Effects.Add({UPWTestEffect::StaticClass(), 1});
	Effects->Effects.Add({nullptr, 1});

	const TArray<FEffectModifierValue> Values = UGASCoreFunctions::GetModifierValuesFromEffects(Params, UPWTestAttributeSet::GetHealthAttribute());
	if (!TestEqual("Only effects that change the attribute are returned", Values.Num(), 1))
		return false;

	TestEqual("Duration type", Values[0].DurationType, EGameplayEffectDurationType::Infinite);
	TestEqual("Period", Values[0].Period, 0.25f);
	TestEqual("Value", Values[0].Value, -10.f);
	TestEqual("An infinite effect has no duration", Values[0].Duration, 0.f);

	TestEqual("Another attribute gives nothing", UGASCoreFunctions::GetModifierValuesFromEffects(Params, UPWTestAttributeSet::GetHealthMaxAttribute()).Num(), 0);
	return true;
}

#endif
