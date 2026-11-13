// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "Core/PWTeamsSubsystem.h"
#include "Agents/PWTeamAgentComponent.h"
#include "Replication/PWTeamRegistry.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// Real networking cannot run here. This drives the two ends by hand: the server world fills its registry actor, and a second world receives that state the way a client does.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTeamsReplicatedStateTest, "PW.Teams.Replication.State", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTeamsReplicatedStateTest::RunTest(const FString& Parameters)
{
	const FPWTestWorld Server;
	const FPWTestWorld Client;
	UPWTeamsSubsystem* ServerTeams = UPWTeamsSubsystem::Get(Server.World);
	UPWTeamsSubsystem* ClientTeams = UPWTeamsSubsystem::Get(Client.World);

	ServerTeams->CreateFaction("Wild", true);
	const FPWTeamParticipantHandle Red = ServerTeams->RegisterParticipant("Red", EPWTeamParticipantType::Player, "Radiant");
	const FPWTeamParticipantHandle Blue = ServerTeams->RegisterParticipant("Blue", EPWTeamParticipantType::AI, "Dire");

	const APWTeamRegistry* Registry = ServerTeams->GetRegistry();
	if (!TestNotNull("The server spawns its registry by itself", Registry))
		return false;
	TestEqual("It holds the participants", Registry->Participants.Num(), 2);
	TestEqual("And the factions", Registry->Factions.Num(), 3);
	TestTrue("It reaches every client", Registry->bAlwaysRelevant && Registry->GetIsReplicated());

	// An agent that reached the client before the registry did.
	AActor* ClientActor = Client.SpawnASC()->GetOwner();
	UPWTeamAgentComponent* ClientAgent = NewObject<UPWTeamAgentComponent>(ClientActor);
	ClientAgent->RegisterComponent();
	ClientAgent->SetOwnerParticipant(Red);
	TestTrue("Before the registry arrives, an inherited faction is unknown", ClientAgent->GetFaction().IsNone());

	UPWTestTeamsListener* Listener = NewObject<UPWTestTeamsListener>();
	ClientTeams->OnParticipantRegistered.AddDynamic(Listener, &UPWTestTeamsListener::OnRegistered);
	ClientTeams->OnParticipantUnregistered.AddDynamic(Listener, &UPWTestTeamsListener::OnUnregistered);
	ClientTeams->OnFactionCreated.AddDynamic(Listener, &UPWTestTeamsListener::OnFactionCreated);
	ClientTeams->OnFactionDestroyed.AddDynamic(Listener, &UPWTestTeamsListener::OnFactionDestroyed);
	ClientTeams->OnParticipantFactionChanged.AddDynamic(Listener, &UPWTestTeamsListener::OnFactionChanged);
	ClientAgent->OnFactionChanged.AddDynamic(Listener, &UPWTestTeamsListener::OnAgentFactionChanged);

	ClientTeams->ApplyReplicatedState(*Registry);

	TestEqual("A late joiner receives every participant", ClientTeams->GetParticipants().Num(), 2);
	TestEqual("And every faction", ClientTeams->GetFactions().Num(), 3);
	TestEqual("Listeners hear the participants", Listener->Registered, 2);
	TestEqual("And the factions", Listener->FactionsCreated, 3);
	TestEqual("The agent that was waiting now has its faction", ClientAgent->GetFaction(), FName("Radiant"));
	TestEqual("And was told", Listener->LastAgentFaction, FName("Radiant"));

	FPWTeamFaction Faction;
	TestTrue("Neutral flags come across", ClientTeams->GetFaction("Wild", Faction) && Faction.bNeutral);
	FPWTeamParticipant Data;
	TestTrue("Participant details come across", ClientTeams->GetParticipant(Blue, Data) && Data.Type == EPWTeamParticipantType::AI && Data.Name == FName("Blue"));

	ClientTeams->ApplyReplicatedState(*Registry);
	TestEqual("Receiving the same state again says nothing", Listener->Registered, 2);

	// A change of sides on the server.
	ServerTeams->AssignParticipantToFaction(Red, "Dire");
	const int32 ChangesBefore = Listener->FactionChanges;
	ClientTeams->ApplyReplicatedState(*Registry);
	TestEqual("A faction change reaches the client once", Listener->FactionChanges, ChangesBefore + 1);
	TestEqual("With the new faction", Listener->LastNewFaction, FName("Dire"));
	TestEqual("And its agents follow", ClientAgent->GetFaction(), FName("Dire"));

	ServerTeams->SetDefaultAttitude(EPWTeamAttitude::Neutral);
	ClientTeams->ApplyReplicatedState(*Registry);
	TestEqual("The default attitude comes across", ClientTeams->DefaultAttitude, EPWTeamAttitude::Neutral);

	ServerTeams->UnregisterParticipant(Blue);
	ServerTeams->DestroyFaction("Wild");
	ClientTeams->ApplyReplicatedState(*Registry);
	TestEqual("A removed participant goes", Listener->Unregistered, 1);
	TestEqual("A destroyed faction goes", Listener->FactionsDestroyed, 1);
	TestFalse("And cannot be found", ClientTeams->GetParticipant(Blue, Data));

	return true;
}

#endif
