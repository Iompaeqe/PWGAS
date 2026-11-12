// copyright mkurtt96

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

#endif
