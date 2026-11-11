// copyright mkurtt96

#include "PWTestWorld.h"
#include "GAS/Effects/PWGameplayEffect_BuffTag.h"
#include "GAS/Tags/GASCoreTags.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEffectLifecycleBuffTagTest, "PW.ASC.EffectLifecycle.BuffTag", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEffectLifecycleBuffTagTest::RunTest(const FString& Parameters)
{
	FPWTestWorld Test;
	const FPWASC_EffectLifecycle& Effects = Test.ASC->Effects();
	const FGameplayTag BuffTag = PWTags::Ability::State::AwaitingConfirmation;

	Effects.ApplyBuffTag(BuffTag);
	Effects.ApplyBuffTag(BuffTag);
	TestTrue("Tag granted", Test.ASC->HasMatchingGameplayTag(BuffTag));
	TestEqual("One effect per call", Test.ASC->GetGameplayEffectCount(UPWGameplayEffect_BuffTag::StaticClass(), nullptr), 2);

	Effects.RemoveEffectsByTag(BuffTag.GetSingleTagContainer());
	TestFalse("Tag removed", Test.ASC->HasMatchingGameplayTag(BuffTag));
	TestEqual("Effects removed", Test.ASC->GetGameplayEffectCount(UPWGameplayEffect_BuffTag::StaticClass(), nullptr), 0);

	return true;
}

#endif
