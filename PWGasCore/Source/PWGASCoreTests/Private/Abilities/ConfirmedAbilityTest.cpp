// copyright mkurtt96

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "GAS/Tags/GASCoreTags.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConfirmedAbilityNoGraphTest, "PW.Abilities.Confirmed.NoGraph", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FConfirmedAbilityNoGraphTest::RunTest(const FString& Parameters)
{
	FPWTestWorld Test;

	const FGameplayAbilitySpecHandle Handle = Test.ASC->GiveAbility(FGameplayAbilitySpec(UPWTestConfirmedAbility::StaticClass(), 1));
	const auto IsAwaiting = [&]() { return Test.ASC->FindAbilitySpecFromHandle(Handle)->GetDynamicSpecSourceTags().HasTagExact(PWTags::Ability::State::AwaitingConfirmation); };
	const auto IsActive = [&]() { return Test.ASC->FindAbilitySpecFromHandle(Handle)->IsActive(); };
	const auto Effects = [&]() { return Test.ASC->GetGameplayEffectCount(UPWTestDurationEffect::StaticClass(), nullptr); };

	TestTrue("Activated", Test.ASC->TryActivateAbility(Handle));
	TestTrue("Waits for confirmation", IsActive() && IsAwaiting());
	TestEqual("Nothing happens before the confirm", Effects(), 0);

	Test.ASC->SendInputEvent(PWTags::Input::Root, EInputEventType::Pressed);
	TestTrue("Other inputs do not confirm", IsActive() && IsAwaiting());

	Test.ASC->SendInputEvent(PWTags::Input::GlobalCancel, EInputEventType::Released);
	TestTrue("The wrong event type does not confirm", IsActive() && IsAwaiting());

	Test.ASC->SendInputEvent(PWTags::Input::GlobalCancel, EInputEventType::Pressed);
	TestEqual("Confirm runs the action", Effects(), 1);
	TestFalse("Ended by itself", IsActive());
	TestFalse("No longer awaiting", IsAwaiting());

	TestTrue("Can be cast again", Test.ASC->TryActivateAbility(Handle));
	Test.ASC->CancelAbilityHandle(Handle);
	TestFalse("Cancel ends it", IsActive());
	TestFalse("Cancel clears the awaiting state", IsAwaiting());
	TestEqual("Cancel does not run the action", Effects(), 1);

	Test.ASC->SendInputEvent(PWTags::Input::GlobalCancel, EInputEventType::Pressed);
	TestEqual("A confirm after cancel does nothing", Effects(), 1);

	return true;
}

#endif
