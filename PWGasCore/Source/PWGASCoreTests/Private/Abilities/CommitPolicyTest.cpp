// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// The test abilities pay their cost with UPWTestEffect and cool down with UPWTestDurationEffect, so each can be counted.
template <typename TAbility>
struct TCommitPolicyCase
{
	FPWTestWorld Test;
	FGameplayAbilitySpecHandle Handle;
	TAbility* Ability = nullptr;

	TCommitPolicyCase()
	{
		Handle = Test.ASC->GiveAbility(FGameplayAbilitySpec(TAbility::StaticClass(), 1));
		Ability = Cast<TAbility>(Test.ASC->FindAbilitySpecFromHandle(Handle)->GetPrimaryInstance());
	}

	TCommitPolicyCase(const EPWCommitPolicy CostPolicy, const EPWCommitPolicy CooldownPolicy) : TCommitPolicyCase()
	{
		Ability->CostPolicy = CostPolicy;
		Ability->CooldownPolicy = CooldownPolicy;
	}

	void Activate() const { Test.ASC->TryActivateAbility(Handle); }
	void Finish() const { for (int32 i = 0; i < 6; ++i) Test.TickTimers(0.25f); }
	void Cancel() const { Test.ASC->CancelAbilityHandle(Handle); }
	int32 Costs() const { return Test.ASC->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr); }
	int32 Cooldowns() const { return Test.ASC->GetGameplayEffectCount(UPWTestDurationEffect::StaticClass(), nullptr); }
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChargedCommitPolicyTest, "PW.Abilities.Charged.CommitPolicy", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FChargedCommitPolicyTest::RunTest(const FString& Parameters)
{
	{
		const TCommitPolicyCase<UPWTestChargedAbility> Case;
		Case.Activate();
		TestEqual("By default nothing is paid while charging", Case.Costs() + Case.Cooldowns(), 0);
		Case.Finish();
		TestEqual("By default the cost is paid when the charge completes", Case.Costs(), 1);
		TestEqual("and the cooldown starts", Case.Cooldowns(), 1);
	}
	{
		const TCommitPolicyCase<UPWTestChargedAbility> Case;
		Case.Activate();
		Case.Cancel();
		TestEqual("By default a cancelled charge is free", Case.Costs() + Case.Cooldowns(), 0);
	}
	{
		const TCommitPolicyCase<UPWTestChargedAbility> Case(EPWCommitPolicy::OnStart, EPWCommitPolicy::OnStart);
		Case.Activate();
		TestEqual("On Start pays when the charge starts", Case.Costs(), 1);
		TestEqual("On Start cools down when the charge starts", Case.Cooldowns(), 1);
		Case.Ability->StopCharging(false);
		TestEqual("On Start does not commit again", Case.Costs() + Case.Cooldowns(), 2);
	}
	{
		const TCommitPolicyCase<UPWTestChargedAbility> Case(EPWCommitPolicy::Manual, EPWCommitPolicy::Manual);
		Case.Activate();
		Case.Ability->StopCharging(false);
		TestEqual("Manual never commits", Case.Costs() + Case.Cooldowns(), 0);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChannelingCommitPolicyTest, "PW.Abilities.Channeling.CommitPolicy", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FChannelingCommitPolicyTest::RunTest(const FString& Parameters)
{
	{
		const TCommitPolicyCase<UPWTestChannelingAbility> Case;
		Case.Activate();
		TestEqual("By default the cost is paid when the channel starts", Case.Costs(), 1);
		TestEqual("and the cooldown waits", Case.Cooldowns(), 0);
		Case.Finish();
		TestEqual("The cooldown starts when the channel finishes", Case.Cooldowns(), 1);
		TestEqual("and the cost is not paid again", Case.Costs(), 1);
	}
	{
		const TCommitPolicyCase<UPWTestChannelingAbility> Case;
		Case.Activate();
		Case.Cancel();
		TestEqual("A cancelled channel still starts its cooldown", Case.Cooldowns(), 1);
	}
	{
		const TCommitPolicyCase<UPWTestChannelingAbility> Case(EPWCommitPolicy::OnStart, EPWCommitPolicy::OnComplete);
		Case.Activate();
		Case.Cancel();
		TestEqual("On Complete skips the cooldown of a cancelled channel", Case.Cooldowns(), 0);
	}
	{
		const TCommitPolicyCase<UPWTestChannelingAbility> Case(EPWCommitPolicy::OnEnd, EPWCommitPolicy::OnEnd);
		Case.Activate();
		TestEqual("On End pays nothing while channeling", Case.Costs() + Case.Cooldowns(), 0);
		Case.Cancel();
		TestEqual("On End pays when the channel is cancelled", Case.Costs(), 1);
		TestEqual("and cools down", Case.Cooldowns(), 1);
	}
	{
		const TCommitPolicyCase<UPWTestChannelingAbility> Case(EPWCommitPolicy::Manual, EPWCommitPolicy::Manual);
		Case.Activate();
		Case.Ability->StopChanneling(false);
		TestEqual("Manual never commits", Case.Costs() + Case.Cooldowns(), 0);
	}
	return true;
}

#endif
