// copyright mkurtt96

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

template <typename TAbility>
struct TCommitPolicyCase
{
	FPWTestWorld Test;
	FGameplayAbilitySpecHandle Handle;
	TAbility* Ability = nullptr;

	explicit TCommitPolicyCase(const EPWCommitPolicy Policy)
	{
		Handle = Test.ASC->GiveAbility(FGameplayAbilitySpec(TAbility::StaticClass(), 1));
		Ability = Cast<TAbility>(Test.ASC->FindAbilitySpecFromHandle(Handle)->GetPrimaryInstance());
		Ability->CommitPolicy = Policy;
	}

	int32 Commits() const { return Test.ASC->GetGameplayEffectCount(UPWTestDurationEffect::StaticClass(), nullptr); }
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChargedCommitPolicyTest, "PW.Abilities.Charged.CommitPolicy", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FChargedCommitPolicyTest::RunTest(const FString& Parameters)
{
	{
		TCommitPolicyCase<UPWTestChargedAbility> Case(EPWCommitPolicy::OnStart);
		Case.Test.ASC->TryActivateAbility(Case.Handle);
		TestEqual("On Start commits when the charge starts", Case.Commits(), 1);
		Case.Ability->StopCharging(false);
		TestEqual("On Start does not commit again", Case.Commits(), 1);
	}
	{
		TCommitPolicyCase<UPWTestChargedAbility> Case(EPWCommitPolicy::OnEnd);
		Case.Test.ASC->TryActivateAbility(Case.Handle);
		TestEqual("On End does not commit while charging", Case.Commits(), 0);
		for (int32 i = 0; i < 6; ++i) Case.Test.TickTimers(0.25f);
		TestEqual("On End commits when the charge completes", Case.Commits(), 1);
	}
	{
		TCommitPolicyCase<UPWTestChargedAbility> Case(EPWCommitPolicy::OnEnd);
		Case.Test.ASC->TryActivateAbility(Case.Handle);
		Case.Test.ASC->CancelAbilityHandle(Case.Handle);
		TestEqual("A cancelled charge is free", Case.Commits(), 0);
	}
	{
		TCommitPolicyCase<UPWTestChargedAbility> Case(EPWCommitPolicy::Manual);
		Case.Test.ASC->TryActivateAbility(Case.Handle);
		Case.Ability->StopCharging(false);
		TestEqual("Manual never commits", Case.Commits(), 0);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChannelingCommitPolicyTest, "PW.Abilities.Channeling.CommitPolicy", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FChannelingCommitPolicyTest::RunTest(const FString& Parameters)
{
	{
		TCommitPolicyCase<UPWTestChannelingAbility> Case(EPWCommitPolicy::OnStart);
		Case.Test.ASC->TryActivateAbility(Case.Handle);
		TestEqual("On Start commits when the channel starts", Case.Commits(), 1);
		Case.Ability->StopChanneling(false);
		TestEqual("On Start does not commit again", Case.Commits(), 1);
	}
	{
		TCommitPolicyCase<UPWTestChannelingAbility> Case(EPWCommitPolicy::OnEnd);
		Case.Test.ASC->TryActivateAbility(Case.Handle);
		TestEqual("On End does not commit while channeling", Case.Commits(), 0);
		for (int32 i = 0; i < 6; ++i) Case.Test.TickTimers(0.25f);
		TestEqual("On End commits when the channel finishes", Case.Commits(), 1);
	}
	{
		TCommitPolicyCase<UPWTestChannelingAbility> Case(EPWCommitPolicy::OnEnd);
		Case.Test.ASC->TryActivateAbility(Case.Handle);
		Case.Test.ASC->CancelAbilityHandle(Case.Handle);
		TestEqual("A cancelled channel is free", Case.Commits(), 0);
	}
	{
		TCommitPolicyCase<UPWTestChannelingAbility> Case(EPWCommitPolicy::Manual);
		Case.Test.ASC->TryActivateAbility(Case.Handle);
		Case.Ability->StopChanneling(false);
		TestEqual("Manual never commits", Case.Commits(), 0);
	}
	return true;
}

#endif
