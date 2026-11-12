// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInstantAbilityNoGraphTest, "PW.Abilities.Instant.NoGraph", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInstantAbilityNoGraphTest::RunTest(const FString& Parameters)
{
	FPWTestWorld Test;

	const FGameplayAbilitySpecHandle Handle = Test.ASC->GiveAbility(FGameplayAbilitySpec(UPWTestInstantBuffAbility::StaticClass(), 1));
	TestTrue("Activated", Test.ASC->TryActivateAbility(Handle));

	TestFalse("Ended by itself", Test.ASC->FindAbilitySpecFromHandle(Handle)->IsActive());
	TestEqual("Timed effect stays", Test.ASC->GetGameplayEffectCount(UPWTestDurationEffect::StaticClass(), nullptr), 1);
	TestEqual("Infinite effect ends with the ability", Test.ASC->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 0);

	TestTrue("Can be cast again", Test.ASC->TryActivateAbility(Handle));

	return true;
}

#endif
