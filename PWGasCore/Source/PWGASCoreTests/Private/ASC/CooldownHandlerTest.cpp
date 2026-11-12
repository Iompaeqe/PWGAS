// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestWorld.h"
#include "GameplayEffect.h"
#include "GAS/Tags/GASCoreTags.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#define TAG_Test_Cooldown PWTags::Ability::SetByCaller::Cooldown
#define TAG_Test_Unused PWTags::Ability::State::AwaitingConfirmation

static void ApplyCooldown(UPWAbilitySystemComponent* ASC, const FGameplayTag& CooldownTag, const float Duration)
{
	UGameplayEffect* Effect = NewObject<UGameplayEffect>(GetTransientPackage());
	Effect->DurationPolicy = EGameplayEffectDurationType::HasDuration;
	Effect->DurationMagnitude = FScalableFloat(Duration);

	FGameplayEffectSpec Spec(Effect, ASC->MakeEffectContext(), 1);
	Spec.DynamicGrantedTags.AddTag(CooldownTag);
	ASC->ApplyGameplayEffectSpecToSelf(Spec);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCooldownHandlerTest, "PW.ASC.CooldownHandler", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCooldownHandlerTest::RunTest(const FString& Parameters)
{
	FPWTestWorld Test;
	const FPWASC_CooldownHandler& Cooldown = Test.ASC->Cooldown();
	float Remaining = 0, Total = 0;

	TestFalse("No cooldown yet", Cooldown.GetCooldownRemainingForTag(TAG_Test_Cooldown, Remaining, Total));

	ApplyCooldown(Test.ASC, TAG_Test_Cooldown, 10.f);
	TestTrue("Cooldown found", Cooldown.GetCooldownRemainingForTag(TAG_Test_Cooldown, Remaining, Total));
	TestEqual("Remaining", Remaining, 10.f, 0.01f);
	TestEqual("Total", Total, 10.f, 0.01f);

	TestTrue("Adjust", Cooldown.AdjustCooldownForTag(TAG_Test_Cooldown, 3.f));
	Cooldown.GetCooldownRemainingForTag(TAG_Test_Cooldown, Remaining, Total);
	TestEqual("Remaining after adjust", Remaining, 7.f, 0.01f);
	TestEqual("Total after adjust", Total, 10.f, 0.01f);

	TestFalse("Adjust without cooldown", Cooldown.AdjustCooldownForTag(TAG_Test_Unused, 3.f));

	TestEqual("Cleared", Cooldown.ClearCooldownByTag(TAG_Test_Cooldown), 1);
	TestFalse("No cooldown after clear", Cooldown.GetCooldownRemainingForTag(TAG_Test_Cooldown, Remaining, Total));

	ApplyCooldown(Test.ASC, TAG_Test_Cooldown, 10.f);
	TestTrue("Adjust past the end", Cooldown.AdjustCooldownForTag(TAG_Test_Cooldown, 15.f));
	TestFalse("Cooldown expired", Cooldown.GetCooldownRemainingForTag(TAG_Test_Cooldown, Remaining, Total));

	return true;
}

#endif
