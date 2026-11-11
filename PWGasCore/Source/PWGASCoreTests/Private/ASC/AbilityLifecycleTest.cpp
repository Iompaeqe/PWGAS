// copyright mkurtt96

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "GAS/Abilities/PWAbilitySet.h"
#include "GAS/Abilities/Derived/PWGameplayAbility_Channeling.h"
#include "GAS/Tags/GASCoreTags.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAbilityLifecycleInvalidInputTest, "PW.ASC.AbilityLifecycle.InvalidInput", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAbilityLifecycleInvalidInputTest::RunTest(const FString& Parameters)
{
	FPWTestWorld Test;
	const FPWASC_AbilityLifecycle& Abilities = Test.ASC->Abilities();

	Abilities.AddAbility(PWTags::Ability::State::AwaitingConfirmation);
	TestEqual("Unknown tag grants nothing", Test.ASC->GetActivatableAbilities().Num(), 0);

	UPWAbilitySet* AbilitySet = NewObject<UPWAbilitySet>();
	AbilitySet->Abilities.Add({PWTags::Ability::State::AwaitingConfirmation, PWTags::Input::GlobalCancel});
	Abilities.AddAbilitySet(AbilitySet);
	Abilities.AddAbilitySet(nullptr);
	TestEqual("Unknown set entry grants nothing", Test.ASC->GetActivatableAbilities().Num(), 0);

	const FGameplayAbilitySpecHandle Handle = Test.ASC->GiveAbility(FGameplayAbilitySpec(UPWTestPlainAbility::StaticClass(), 1));
	Test.ASC->ClearAbility(Handle);

	Abilities.RemoveGrantedAbility(Handle);
	TestFalse("Deactivation on stale handle", Abilities.ApplyDeactivationPolicies(Handle, PWTags::Ability::Deactivation::OnRemoved));
	TestFalse("Tag check on stale handle", Test.ASC->Data().DoesAbilityHaveTag(Handle, PWTags::Ability::Skill::Root));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAbilityLifecycleForeignAbilityTest, "PW.ASC.AbilityLifecycle.ForeignAbility", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAbilityLifecycleForeignAbilityTest::RunTest(const FString& Parameters)
{
	FPWTestWorld Test;

	const FGameplayAbilitySpecHandle Plain = Test.ASC->GiveAbility(FGameplayAbilitySpec(UPWTestPlainAbility::StaticClass(), 1));
	const FGameplayAbilitySpecHandle Exclusive = Test.ASC->GiveAbility(FGameplayAbilitySpec(UPWGameplayAbility_Channeling::StaticClass(), 1));

	TestTrue("Plain ability activates", Test.ASC->TryActivateAbility(Plain));
	TestTrue("Exclusive ability activates beside it", Test.ASC->TryActivateAbility(Exclusive));
	TestTrue("Plain ability still active", Test.ASC->FindAbilitySpecFromHandle(Plain)->IsActive());

	Test.ASC->CancelAbilityHandle(Plain);
	TestFalse("Plain ability ended", Test.ASC->FindAbilitySpecFromHandle(Plain)->IsActive());

	return true;
}

#endif
