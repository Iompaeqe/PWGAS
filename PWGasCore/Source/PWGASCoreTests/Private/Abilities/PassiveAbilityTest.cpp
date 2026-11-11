// copyright mkurtt96

#include "PWTestWorld.h"
#include "GAS/Abilities/Derived/PWGameplayAbility_Passive.h"
#include "GAS/Tags/GASCoreTags.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPassiveAbilityTest, "PW.Abilities.Passive", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPassiveAbilityTest::RunTest(const FString& Parameters)
{
	FPWTestWorld Test;
	const FPWASC_AbilityLifecycle& Abilities = Test.ASC->Abilities();

	const FGameplayAbilitySpecHandle Handle = Test.ASC->GiveAbility(FGameplayAbilitySpec(UPWGameplayAbility_Passive::StaticClass(), 1));
	const FGameplayAbilitySpec* Spec = Test.ASC->FindAbilitySpecFromHandle(Handle);

	TestTrue("Activated on granted", Abilities.ApplyActivationPolicies(Handle, PWTags::Ability::Activation::OnGranted));
	TestTrue("Active", Spec->IsActive());

	TestTrue("Deactivated on removed", Abilities.ApplyDeactivationPolicies(Handle, PWTags::Ability::Deactivation::OnRemoved));
	TestFalse("Not active", Spec->IsActive());

	return true;
}

#endif
