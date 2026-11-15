// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

template <typename TAbility>
struct TPhaseCase
{
	FPWTestWorld Test;
	FGameplayAbilitySpecHandle Handle;
	TAbility* Ability = nullptr;
	UPWTestPhaseModule* Module = nullptr;

	explicit TPhaseCase(const bool bTickAtStart = false)
	{
		Handle = Test.ASC->GiveAbility(FGameplayAbilitySpec(TAbility::StaticClass(), 1));
		Ability = Cast<TAbility>(Test.ASC->FindAbilitySpecFromHandle(Handle)->GetPrimaryInstance());
		Ability->SetTickAtStart(bTickAtStart);
		Module = NewObject<UPWTestPhaseModule>(Ability);
		Ability->DataModules.Add(Module);
	}

	void Activate() const { Test.ASC->TryActivateAbility(Handle); }
	void Finish() const { for (int32 i = 0; i < 6; ++i) Test.TickTimers(0.25f); }
	void Cancel() const { Test.ASC->CancelAbilityHandle(Handle); }
	int32 Count(const EPWAbilityPhase Phase) const { return Module->Phases.FilterByPredicate([Phase](const EPWAbilityPhase P) { return P == Phase; }).Num(); }
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChannelingPhaseTest, "PW.Abilities.Channeling.ModulePhases", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FChannelingPhaseTest::RunTest(const FString& Parameters)
{
	{
		const TPhaseCase<UPWTestChannelingAbility> Case;
		Case.Activate();
		TestEqual("Modules hear the channel start", Case.Count(EPWAbilityPhase::Started), 1);
		Case.Finish();
		TestEqual("every tick as a pulse", Case.Count(EPWAbilityPhase::Pulse), 4);
		TestEqual("and the finish", Case.Count(EPWAbilityPhase::Completed), 1);
		TestEqual("with no cancel", Case.Count(EPWAbilityPhase::Cancelled), 0);
	}
	{
		const TPhaseCase<UPWTestChannelingAbility> Case;
		Case.Activate();
		Case.Cancel();
		TestEqual("A cancelled channel reports the cancel", Case.Count(EPWAbilityPhase::Cancelled), 1);
		TestEqual("and no finish", Case.Count(EPWAbilityPhase::Completed), 0);
	}
	{
		const TPhaseCase<UPWTestChannelingAbility> Case(true);
		Case.Activate();
		TestEqual("Tick At Start pulses as the channel starts", Case.Count(EPWAbilityPhase::Pulse), 1);
		TestEqual("at time 0", Case.Ability->GetChannelTime(), 0.f);
		Case.Finish();
		TestEqual("then every timed tick", Case.Count(EPWAbilityPhase::Pulse), 5);
		TestEqual("and still finishes once", Case.Count(EPWAbilityPhase::Completed), 1);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChargedPhaseTest, "PW.Abilities.Charged.ModulePhases", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FChargedPhaseTest::RunTest(const FString& Parameters)
{
	{
		const TPhaseCase<UPWTestChargedAbility> Case;
		Case.Activate();
		TestEqual("Modules hear the charge start", Case.Count(EPWAbilityPhase::Started), 1);
		Case.Finish();
		TestEqual("and the release", Case.Count(EPWAbilityPhase::Completed), 1);
		TestEqual("every charge tick as a pulse", Case.Count(EPWAbilityPhase::Pulse), 4);
	}
	{
		const TPhaseCase<UPWTestChargedAbility> Case;
		Case.Activate();
		Case.Cancel();
		TestEqual("A cancelled charge reports the cancel", Case.Count(EPWAbilityPhase::Cancelled), 1);
	}
	{
		const TPhaseCase<UPWTestChargedAbility> Case(true);
		Case.Activate();
		TestEqual("Tick At Start pulses as the charge starts", Case.Count(EPWAbilityPhase::Pulse), 1);
		Case.Finish();
		TestEqual("then every timed tick", Case.Count(EPWAbilityPhase::Pulse), 5);
		TestEqual("and still releases once", Case.Count(EPWAbilityPhase::Completed), 1);
	}
	return true;
}

#endif
