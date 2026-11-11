// copyright mkurtt96

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Targeting/Data/PWTargetingData.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

struct FTargetDataReadyParams
{
	FGameplayAbilityTargetDataHandle DataHandle;
	FPWTargetingResult Result;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInstantEffectModuleTest, "PW.Modules.InstantEffect", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInstantEffectModuleTest::RunTest(const FString& Parameters)
{
	FPWTestWorld Test;
	UPWAbilitySystemComponent* TargetASC = Test.SpawnASC();

	const FGameplayAbilitySpecHandle Handle = Test.ASC->GiveAbility(FGameplayAbilitySpec(UPWTestInstantEffectAbility::StaticClass(), 1));
	TestTrue("Activated", Test.ASC->TryActivateAbility(Handle));

	const UPWModularGameplayAbility* Ability = Cast<UPWModularGameplayAbility>(Test.ASC->FindAbilitySpecFromHandle(Handle)->GetPrimaryInstance());
	UPWAbilityModule_InstantEffect* Module = Ability ? Ability->GetInstantEffectModule() : nullptr;
	if (!TestNotNull("Instant effect module", Module))
		return false;

	Module->ApplyEffects();
	TestEqual("Applied to self without targeting", Test.ASC->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 1);

	Module->RemoveEffects();
	TestEqual("Removed from self", Test.ASC->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 0);

	FTargetDataReadyParams Params;
	Params.Result.HitActors.Add(TargetASC->GetOwner());
	Module->ProcessEvent(Module->FindFunctionChecked("OnTargetDataReady"), &Params);
	TestEqual("Applied to target", TargetASC->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 1);

	Module->RemoveEffects();
	TestEqual("Removed from target", TargetASC->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 0);

	return true;
}

#endif
