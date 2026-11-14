// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "Core/PWTeamsSubsystem.h"
#include "Agents/PWTeamAgentComponent.h"
#include "GameFramework/PlayerState.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

static UPWTeamAgentComponent* AddAgent(AActor* Actor)
{
	UPWTeamAgentComponent* Agent = NewObject<UPWTeamAgentComponent>(Actor);
	Agent->RegisterComponent();
	return Agent;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTeamsAgentAffiliationTest, "PW.Teams.Agents.Affiliation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTeamsAgentAffiliationTest::RunTest(const FString& Parameters)
{
	const FPWTestWorld Test;
	UPWTeamsSubsystem* Teams = UPWTeamsSubsystem::Get(Test.World);
	const FPWTeamParticipantHandle Player = Teams->RegisterParticipant("Player", EPWTeamParticipantType::Player, "Radiant");

	AActor* Bare = Test.SpawnASC()->GetOwner();
	TestNull("An actor without an agent has none", UPWTeamsSubsystem::FindAgent(Bare));
	TestTrue("And no faction", UPWTeamsSubsystem::GetAgentFaction(Bare).IsNone());

	AActor* Loose = Test.SpawnASC()->GetOwner();
	UPWTeamAgentComponent* LooseAgent = AddAgent(Loose);
	TestFalse("An agent can have neither owner", LooseAgent->GetOwnerParticipant().IsValid());
	TestEqual("An agent inherits its owner's faction unless told otherwise", LooseAgent->GetFactionSource(), EPWTeamFactionSource::InheritFromOwner);
	TestTrue("Nor faction", LooseAgent->GetFaction().IsNone());

	AActor* Summon = Test.SpawnASC()->GetOwner();
	UPWTeamAgentComponent* SummonAgent = AddAgent(Summon);
	TestTrue("An owner can be set", UPWTeamsSubsystem::SetAgentOwner(Summon, Player));
	TestEqual("An owned agent inherits its owner's faction", SummonAgent->GetFaction(), FName("Radiant"));

	AActor* Minion = Test.SpawnASC()->GetOwner();
	UPWTeamAgentComponent* MinionAgent = AddAgent(Minion);
	TestTrue("A faction can be set with no owner", UPWTeamsSubsystem::SetAgentFaction(Minion, "Dire"));
	TestFalse("It has no owner", MinionAgent->GetOwnerParticipant().IsValid());
	TestEqual("And its own faction", MinionAgent->GetFaction(), FName("Dire"));

	const FPWTeamParticipantHandle NoFaction = Teams->RegisterParticipant("Loner", EPWTeamParticipantType::Player);
	AActor* Pet = Test.SpawnASC()->GetOwner();
	UPWTeamAgentComponent* PetAgent = AddAgent(Pet);
	PetAgent->SetOwnerParticipant(NoFaction);
	TestTrue("An owner without a faction gives none", PetAgent->GetFaction().IsNone());
	TestEqual("The owner is still known", UPWTeamsSubsystem::GetAgentOwner(Pet), NoFaction);

	AActor* Turret = Test.SpawnASC()->GetOwner();
	UPWTeamAgentComponent* TurretAgent = AddAgent(Turret);
	TurretAgent->SetOwnerParticipant(Player);
	TurretAgent->SetFaction("Dire");
	TestEqual("An explicit faction wins over the owner's", TurretAgent->GetFaction(), FName("Dire"));
	TestEqual("A faction name reads as explicit", TurretAgent->GetFactionSource(), EPWTeamFactionSource::Explicit);

	Teams->AssignParticipantToFaction(Player, "Dire");
	TestEqual("An inherited faction follows the owner", SummonAgent->GetFaction(), FName("Dire"));
	Teams->AssignParticipantToFaction(Player, "Radiant");
	TestEqual("An explicit faction does not", TurretAgent->GetFaction(), FName("Dire"));

	SummonAgent->SetFaction(NAME_None);
	TestTrue("A faction source of None gives no faction", SummonAgent->GetFaction().IsNone());
	TestEqual("None reads as no faction source", SummonAgent->GetFactionSource(), EPWTeamFactionSource::None);
	TestEqual("Ownership is untouched by that", SummonAgent->GetOwnerParticipant(), Player);
	SummonAgent->InheritFactionFromOwner();

	TestEqual("A participant's agents can be listed", Teams->GetAgentsOwnedBy(Player).Num(), 2);
	TestEqual("So can a faction's", Teams->GetAgentsInFaction("Dire").Num(), 2);

	Teams->UnregisterParticipant(Player);
	TestTrue("When the owner leaves, its faction no longer applies", SummonAgent->GetFaction().IsNone());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTeamsAgentEventsTest, "PW.Teams.Agents.Events", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTeamsAgentEventsTest::RunTest(const FString& Parameters)
{
	const FPWTestWorld Test;
	UPWTeamsSubsystem* Teams = UPWTeamsSubsystem::Get(Test.World);
	const FPWTeamParticipantHandle Player = Teams->RegisterParticipant("Player", EPWTeamParticipantType::Player, "Radiant");

	UPWTeamAgentComponent* Agent = AddAgent(Test.SpawnASC()->GetOwner());
	UPWTestTeamsListener* Listener = NewObject<UPWTestTeamsListener>();
	Agent->OnOwnerChanged.AddDynamic(Listener, &UPWTestTeamsListener::OnAgentOwnerChanged);
	Agent->OnFactionChanged.AddDynamic(Listener, &UPWTestTeamsListener::OnAgentFactionChanged);

	Agent->SetOwnerParticipant(Player);
	TestEqual("Listeners hear the new owner", Listener->AgentOwnerChanges, 1);
	TestEqual("And the faction that came with it", Listener->LastAgentFaction, FName("Radiant"));

	Agent->SetOwnerParticipant(Player);
	TestEqual("Setting the same owner again says nothing", Listener->AgentOwnerChanges, 1);

	Teams->AssignParticipantToFaction(Player, "Dire");
	TestEqual("The agent hears when its owner changes sides", Listener->LastAgentFaction, FName("Dire"));
	TestEqual("Once per change", Listener->AgentFactionChanges, 2);

	Agent->SetFaction("Dire");
	TestEqual("Changing where the faction comes from says nothing while the faction is the same", Listener->AgentFactionChanges, 2);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTeamsAgentLookupTest, "PW.Teams.Agents.Lookup", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTeamsAgentLookupTest::RunTest(const FString& Parameters)
{
	const FPWTestWorld Test;
	UPWTeamsSubsystem* Teams = UPWTeamsSubsystem::Get(Test.World);

	APlayerState* PlayerState = Test.World->SpawnActor<APlayerState>();
	UPWTeamAgentComponent* PlayerAgent = AddAgent(PlayerState);
	const FPWTeamParticipantHandle Player = PlayerAgent->GetOwnerParticipant();
	TestTrue("A player state's agent registers a participant", Player.IsValid());
	TestEqual("Found again from the player state", Teams->FindParticipantForPlayer(PlayerState), Player);

	FPWTeamParticipant Data;
	Teams->GetParticipant(Player, Data);
	TestEqual("It knows its player", Data.PlayerState.Get(), PlayerState);

	APawn* Pawn = Cast<APawn>(Test.SpawnASC()->GetOwner());
	Pawn->SetPlayerState(PlayerState);
	TestEqual("A pawn is spoken for by its player state", UPWTeamsSubsystem::FindAgent(Pawn), PlayerAgent);

	APWTestAgentProxy* Proxy = Test.World->SpawnActor<APWTestAgentProxy>();
	TestNull("An actor that names no agent has none", UPWTeamsSubsystem::FindAgent(Proxy));
	Proxy->Agent = PlayerAgent;
	TestEqual("An actor can name the agent that speaks for it", UPWTeamsSubsystem::FindAgent(Proxy), PlayerAgent);

	Teams->AssignParticipantToFaction(Player, "Radiant");
	TestEqual("Everything the player is shares its faction", UPWTeamsSubsystem::GetAgentFaction(Proxy), FName("Radiant"));

	PlayerAgent->UnregisterComponent();
	TestFalse("When the player state goes, so does its participant", Teams->GetParticipant(Player, Data));

	return true;
}

#endif
