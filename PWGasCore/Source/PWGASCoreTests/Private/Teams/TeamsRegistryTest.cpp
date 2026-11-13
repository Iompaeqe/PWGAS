// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "Core/PWTeamsSubsystem.h"
#include "Core/PWTeamsSettings.h"
#include "Core/PWTeamSetup.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTeamsParticipantsTest, "PW.Teams.Registry.Participants", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTeamsParticipantsTest::RunTest(const FString& Parameters)
{
	const FPWTestWorld Test;
	UPWTeamsSubsystem* Teams = UPWTeamsSubsystem::Get(Test.World);
	if (!TestNotNull("Teams subsystem", Teams))
		return false;

	UPWTestTeamsListener* Listener = NewObject<UPWTestTeamsListener>();
	Teams->OnParticipantRegistered.AddDynamic(Listener, &UPWTestTeamsListener::OnRegistered);
	Teams->OnParticipantUnregistered.AddDynamic(Listener, &UPWTestTeamsListener::OnUnregistered);

	const FPWTeamParticipantHandle Red = Teams->RegisterParticipant("Red", EPWTeamParticipantType::Player);
	const FPWTeamParticipantHandle Creeps = Teams->RegisterParticipant("Creeps", EPWTeamParticipantType::Environment);
	TestTrue("A registered participant has a handle", Red.IsValid());
	TestTrue("Handles differ", Red != Creeps);
	TestEqual("Listeners hear each registration", Listener->Registered, 2);

	FPWTeamParticipant Data;
	TestTrue("It can be looked up", Teams->GetParticipant(Red, Data));
	TestEqual("With its name", Data.Name, FName("Red"));
	TestEqual("And its type", Data.Type, EPWTeamParticipantType::Player);
	TestTrue("It starts without a faction", Data.Faction.IsNone());
	TestFalse("An empty handle is nobody", Teams->GetParticipant(FPWTeamParticipantHandle(), Data));

	TestTrue("A participant can be removed", Teams->UnregisterParticipant(Creeps));
	TestFalse("Only once", Teams->UnregisterParticipant(Creeps));
	TestEqual("Listeners hear the removal", Listener->Unregistered, 1);
	TestEqual("One participant is left", Teams->GetParticipants().Num(), 1);

	const FPWTeamParticipantHandle Blue = Teams->RegisterParticipant("Blue", EPWTeamParticipantType::AI);
	TestTrue("A removed handle is not reused", Blue != Creeps);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTeamsFactionsTest, "PW.Teams.Registry.Factions", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTeamsFactionsTest::RunTest(const FString& Parameters)
{
	const FPWTestWorld Test;
	UPWTeamsSubsystem* Teams = UPWTeamsSubsystem::Get(Test.World);
	if (!TestNotNull("Teams subsystem", Teams))
		return false;

	UPWTestTeamsListener* Listener = NewObject<UPWTestTeamsListener>();
	Teams->OnFactionCreated.AddDynamic(Listener, &UPWTestTeamsListener::OnFactionCreated);
	Teams->OnFactionDestroyed.AddDynamic(Listener, &UPWTestTeamsListener::OnFactionDestroyed);
	Teams->OnParticipantFactionChanged.AddDynamic(Listener, &UPWTestTeamsListener::OnFactionChanged);

	TestTrue("A faction takes any name", Teams->CreateFaction("Radiant"));
	TestFalse("A name is used once", Teams->CreateFaction("Radiant"));
	TestFalse("A faction needs a name", Teams->CreateFaction(NAME_None));
	TestTrue("A neutral faction", Teams->CreateFaction("Wildlife", true));
	TestEqual("Listeners hear each creation", Listener->FactionsCreated, 2);

	FPWTeamFaction Faction;
	TestTrue("It can be looked up", Teams->GetFaction("Wildlife", Faction));
	TestTrue("With its neutral flag", Faction.bNeutral);
	TestFalse("An unknown faction is not found", Teams->GetFaction("Nobody", Faction));

	const FPWTeamParticipantHandle One = Teams->RegisterParticipant("One", EPWTeamParticipantType::Player, "Radiant");
	const FPWTeamParticipantHandle Two = Teams->RegisterParticipant("Two", EPWTeamParticipantType::Player);
	TestEqual("A participant can join while registering", Teams->GetParticipantFaction(One), FName("Radiant"));
	TestEqual("That counts as a change", Listener->FactionChanges, 1);

	TestTrue("A participant can be assigned later", Teams->AssignParticipantToFaction(Two, "Radiant"));
	TestEqual("A faction has no size limit to speak of", Teams->GetFactionParticipants("Radiant").Num(), 2);

	TestTrue("Assigning to a new name creates the faction", Teams->AssignParticipantToFaction(Two, "Dire"));
	TestTrue("It exists now", Teams->GetFaction("Dire", Faction));
	TestEqual("A participant is in one faction at a time", Teams->GetFactionParticipants("Radiant").Num(), 1);
	TestEqual("Listeners get the old faction", Listener->LastOldFaction, FName("Radiant"));
	TestEqual("And the new one", Listener->LastNewFaction, FName("Dire"));

	const int32 ChangesBefore = Listener->FactionChanges;
	Teams->AssignParticipantToFaction(Two, "Dire");
	TestEqual("Assigning the same faction again says nothing", Listener->FactionChanges, ChangesBefore);

	TestTrue("A participant can leave its faction", Teams->RemoveParticipantFromFaction(Two));
	TestTrue("And be without one", Teams->GetParticipantFaction(Two).IsNone());

	TestTrue("A faction can be destroyed", Teams->DestroyFaction("Radiant"));
	TestTrue("Its members are left without a faction", Teams->GetParticipantFaction(One).IsNone());
	TestEqual("Listeners hear the destruction", Listener->FactionsDestroyed, 1);
	TestFalse("It is gone", Teams->GetFaction("Radiant", Faction));
	TestEqual("The participant itself remains", Teams->GetParticipants().Num(), 2);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTeamsSetupTest, "PW.Teams.Registry.Setup", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTeamsSetupTest::RunTest(const FString& Parameters)
{
	UPWTeamSetup* Setup = NewObject<UPWTeamSetup>();
	Setup->AddToRoot();
	Setup->Factions.Add({"Radiant", false});
	Setup->Factions.Add({"Wild", true});
	{
		const FPWTestWorld Test(Setup);
		const UPWTeamsSubsystem* Teams = UPWTeamsSubsystem::Get(Test.World);

		TestEqual("The setup's names fill dropdowns", UPWTeamsSettings::GetFactionNames().Num(), 2);

		FPWTeamFaction Faction;
		TestTrue("The server creates the setup's factions when play begins", Teams->GetFaction("Radiant", Faction));
		TestTrue("With their neutral flags", Teams->GetFaction("Wild", Faction) && Faction.bNeutral);
		TestEqual("And only those", Teams->GetFactions().Num(), 2);
	}
	{
		const FPWTestWorld Test;
		TestEqual("Without a setup there are no factions to begin with", UPWTeamsSubsystem::Get(Test.World)->GetFactions().Num(), 0);
	}

	Setup->RemoveFromRoot();
	return true;
}

#endif
