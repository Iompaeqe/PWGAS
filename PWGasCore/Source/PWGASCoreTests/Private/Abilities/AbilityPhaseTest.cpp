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
	int32 Cooldowns() const { return Test.ASC->GetGameplayEffectCount(UPWTestDurationEffect::StaticClass(), nullptr); }

	// Ends the ability as it completes, the way a graph's On Channel End or On Charge Complete calls End Ability.
	void EndOnCompleted() const { Module->OnCompleted = [this] { Ability->EndFromGraph(); }; }
	// Sets bOutSent when the ability ends: whether that end goes to the other machine.
	void WatchEnd(bool& bOutSent) const { Ability->OnGameplayAbilityEndedWithData.AddLambda([&bOutSent](const FAbilityEndedData& Data) { bOutSent = Data.bReplicateEndAbility; }); }
	// Runs Body with the owner as a client's copy, as on the machine that predicts the cast.
	template <typename TBody>
	void AsClient(TBody&& Body) const
	{
		Test.Actor->SetRole(ROLE_AutonomousProxy);
		Body();
		Test.Actor->SetRole(ROLE_Authority);
	}
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChannelingPhaseTest, "PW.Abilities.Channeling.ModulePhases", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FChannelingPhaseTest::RunTest(const FString& Parameters)
{
	{
		const TPhaseCase<UPWTestChannelingAbility> Case;
		Case.Activate();
		TestEqual("Modules hear the channel start", Case.Count(EPWAbilityPhase::Started), 1);
		Case.Finish();
		TestEqual("every tick", Case.Count(EPWAbilityPhase::Tick), 4);
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
		TestEqual("Tick At Start ticks as the channel starts", Case.Count(EPWAbilityPhase::Tick), 1);
		TestEqual("at time 0", Case.Ability->GetChannelTime(), 0.f);
		Case.Finish();
		TestEqual("then every timed tick", Case.Count(EPWAbilityPhase::Tick), 5);
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
		TestEqual("every charge tick", Case.Count(EPWAbilityPhase::Tick), 4);
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
		TestEqual("Tick At Start ticks as the charge starts", Case.Count(EPWAbilityPhase::Tick), 1);
		Case.Finish();
		TestEqual("then every timed tick", Case.Count(EPWAbilityPhase::Tick), 5);
		TestEqual("and still releases once", Case.Count(EPWAbilityPhase::Completed), 1);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChannelingEndMidwayTest, "PW.Abilities.Channeling.EndMidway", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FChannelingEndMidwayTest::RunTest(const FString& Parameters)
{
	{
		const TPhaseCase<UPWTestChannelingAbility> Case;
		Case.Activate();
		Case.Ability->RunTicks(2);
		Case.Ability->EndFromGraph();
		TestFalse("Ending mid channel stops the channel", Case.Ability->IsChanneling());
		TestEqual("as finished", Case.Count(EPWAbilityPhase::Completed), 1);
		TestEqual("with its cooldown", Case.Cooldowns(), 1);
		Case.Activate();
		TestEqual("The next cast channels again", Case.Count(EPWAbilityPhase::Started), 2);
		Case.Finish();
		TestEqual("to its end", Case.Count(EPWAbilityPhase::Completed), 2);
	}
	{
		const TPhaseCase<UPWTestChannelingAbility> Case;
		Case.Activate();
		Case.Ability->RunTicks(3);
		Case.Ability->EndAsRemoteEnded();
		TestEqual("When the other copy ended after its last tick, this copy runs that tick too", Case.Count(EPWAbilityPhase::Tick), 4);
		TestEqual("and finishes", Case.Count(EPWAbilityPhase::Completed), 1);
		TestFalse("and ends", Case.Ability->IsActive());
	}
	{
		const TPhaseCase<UPWTestChannelingAbility> Case;
		Case.Activate();
		Case.Ability->RunTicks(1);
		Case.Ability->EndAsRemoteEnded();
		TestEqual("It skips ticks the other copy ended well before", Case.Count(EPWAbilityPhase::Tick), 1);
		TestEqual("and still finishes", Case.Count(EPWAbilityPhase::Completed), 1);
	}
	{
		const TPhaseCase<UPWTestChannelingAbility> Case;
		Case.Activate();
		Case.Ability->RunTicks(3);
		Case.Ability->SetRemoteInstanceHasEnded();
		Case.Cancel();
		TestEqual("A cancel runs no last tick", Case.Count(EPWAbilityPhase::Tick), 3);
		TestEqual("and reports the cancel", Case.Count(EPWAbilityPhase::Cancelled), 1);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChargedEndMidwayTest, "PW.Abilities.Charged.EndMidway", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FChargedEndMidwayTest::RunTest(const FString& Parameters)
{
	{
		const TPhaseCase<UPWTestChargedAbility> Case;
		Case.Activate();
		Case.Ability->RunTicks(2);
		Case.Ability->EndFromGraph();
		TestFalse("Ending mid charge stops the charge", Case.Ability->IsCharging());
		TestEqual("as released", Case.Count(EPWAbilityPhase::Completed), 1);
		TestEqual("with its cooldown", Case.Cooldowns(), 1);
		Case.Activate();
		TestEqual("The next cast charges again", Case.Count(EPWAbilityPhase::Started), 2);
		Case.Finish();
		TestEqual("to full", Case.Count(EPWAbilityPhase::Completed), 2);
	}
	{
		const TPhaseCase<UPWTestChargedAbility> Case;
		Case.Activate();
		Case.Ability->RunTicks(3);
		Case.Ability->EndAsRemoteEnded();
		TestEqual("When the other copy ended fully charged, this copy reaches full charge too", Case.Count(EPWAbilityPhase::Tick), 4);
		TestEqual("and releases", Case.Count(EPWAbilityPhase::Completed), 1);
	}
	{
		const TPhaseCase<UPWTestChargedAbility> Case;
		Case.Activate();
		Case.Ability->RunTicks(1);
		Case.Ability->EndAsRemoteEnded();
		TestEqual("It releases where it is when the other copy let go early", Case.Count(EPWAbilityPhase::Tick), 1);
		TestEqual("and still releases", Case.Count(EPWAbilityPhase::Completed), 1);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChannelingAutoEndTest, "PW.Abilities.Channeling.AutoEnd", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FChannelingAutoEndTest::RunTest(const FString& Parameters)
{
	{
		const TPhaseCase<UPWTestChannelingAbility> Case;
		Case.Activate();
		Case.Finish();
		TestFalse("The ability ends when its channel runs out", Case.Ability->IsActive());
	}
	{
		const TPhaseCase<UPWTestChannelingAbility> Case;
		Case.Activate();
		Case.Ability->StopChanneling(true);
		TestFalse("and when the graph stops the channel as cancelled", Case.Ability->IsActive());
		TestEqual("which reports one cancel", Case.Count(EPWAbilityPhase::Cancelled), 1);
	}
	{
		const TPhaseCase<UPWTestChannelingAbility> Case;
		Case.Ability->SetAutoEndAbility(false);
		Case.Activate();
		Case.Finish();
		TestTrue("With Auto End Ability off it stays active for the graph to end", Case.Ability->IsActive());
	}
	{
		const TPhaseCase<UPWTestChannelingAbility> Case;
		bool bSent = true;
		Case.Activate();
		Case.WatchEnd(bSent);
		Case.AsClient([&Case] { Case.Ability->RunTicks(4); });
		TestFalse("A client that ran the whole channel ends by itself", Case.Ability->IsActive());
		TestFalse("without telling the server", bSent);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChargedAutoEndTest, "PW.Abilities.Charged.AutoEnd", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FChargedAutoEndTest::RunTest(const FString& Parameters)
{
	{
		const TPhaseCase<UPWTestChargedAbility> Case;
		Case.Activate();
		Case.Finish();
		TestFalse("The ability ends when its charge is full", Case.Ability->IsActive());
	}
	{
		const TPhaseCase<UPWTestChargedAbility> Case;
		Case.Activate();
		Case.Ability->StopCharging(false);
		TestFalse("and when the graph releases it early", Case.Ability->IsActive());
		TestEqual("which reports one release", Case.Count(EPWAbilityPhase::Completed), 1);
	}
	{
		const TPhaseCase<UPWTestChargedAbility> Case;
		Case.Ability->SetAutoEndAbility(false);
		Case.Activate();
		Case.Finish();
		TestTrue("With Auto End Ability off it stays active for the graph to end", Case.Ability->IsActive());
	}
	{
		const TPhaseCase<UPWTestChargedAbility> Case;
		bool bSent = true;
		Case.Activate();
		Case.WatchEnd(bSent);
		Case.AsClient([&Case] { Case.Ability->RunTicks(4); });
		TestFalse("A client whose charge filled by itself ends by itself", Case.Ability->IsActive());
		TestFalse("without telling the server", bSent);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChannelingClientEndTest, "PW.Abilities.Channeling.ClientEnd", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FChannelingClientEndTest::RunTest(const FString& Parameters)
{
	{
		const TPhaseCase<UPWTestChannelingAbility> Case;
		bool bSent = true;
		Case.Activate();
		Case.EndOnCompleted();
		Case.WatchEnd(bSent);
		Case.AsClient([&Case] { Case.Ability->RunTicks(4); });
		TestFalse("A client that ran the whole channel ends", Case.Ability->IsActive());
		TestFalse("without telling the server", bSent);
	}
	{
		const TPhaseCase<UPWTestChannelingAbility> Case;
		bool bSent = false;
		Case.Activate();
		Case.WatchEnd(bSent);
		Case.Ability->RunTicks(2);
		Case.AsClient([&Case] { Case.Ability->EndFromGraph(); });
		TestTrue("A client that stops early tells the server", bSent);
	}
	{
		const TPhaseCase<UPWTestChannelingAbility> Case;
		bool bSent = false;
		Case.Activate();
		Case.EndOnCompleted();
		Case.WatchEnd(bSent);
		Case.Ability->RunTicks(4);
		TestTrue("The server's own end is still sent", bSent);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChargedClientEndTest, "PW.Abilities.Charged.ClientEnd", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FChargedClientEndTest::RunTest(const FString& Parameters)
{
	{
		const TPhaseCase<UPWTestChargedAbility> Case;
		bool bSent = true;
		Case.Activate();
		Case.EndOnCompleted();
		Case.WatchEnd(bSent);
		Case.AsClient([&Case] { Case.Ability->RunTicks(4); });
		TestFalse("A client whose charge filled by itself ends", Case.Ability->IsActive());
		TestFalse("without telling the server", bSent);
	}
	{
		const TPhaseCase<UPWTestChargedAbility> Case;
		bool bSent = false;
		Case.Activate();
		Case.WatchEnd(bSent);
		Case.Ability->RunTicks(2);
		Case.AsClient([&Case] { Case.Ability->EndFromGraph(); });
		TestTrue("A client that releases early tells the server", bSent);
	}
	{
		const TPhaseCase<UPWTestChargedAbility> Case;
		bool bSent = false;
		Case.Activate();
		Case.EndOnCompleted();
		Case.WatchEnd(bSent);
		Case.Ability->RunTicks(4);
		TestTrue("The server's own end is still sent", bSent);
	}
	return true;
}

#endif
