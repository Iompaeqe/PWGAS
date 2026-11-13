// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "Core/PWTeamsSubsystem.h"
#include "Core/PWTeamsTags.h"
#include "Agents/PWTeamAgentComponent.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	struct FTeamsFixture
	{
		FPWTestWorld Test;
		UPWTeamsSubsystem* Teams = nullptr;
		FPWTeamParticipantHandle RedPlayer;
		FPWTeamParticipantHandle RedMate;
		FPWTeamParticipantHandle BluePlayer;
		AActor* Hero = nullptr;
		AActor* Summon = nullptr;
		AActor* Mate = nullptr;
		AActor* Enemy = nullptr;
		AActor* EnemyMinion = nullptr;
		AActor* Wildlife = nullptr;
		AActor* Crate = nullptr;

		AActor* Spawn(const FPWTeamParticipantHandle Owner, const FName ExplicitFaction = NAME_None, const bool bPrimary = false) const
		{
			AActor* Actor = Test.SpawnASC()->GetOwner();
			UPWTeamAgentComponent* Agent = NewObject<UPWTeamAgentComponent>(Actor);
			Agent->RegisterComponent();
			Agent->SetOwnerParticipant(Owner);
			if (!ExplicitFaction.IsNone())
				Agent->SetExplicitFaction(ExplicitFaction);
			Agent->SetPrimary(bPrimary);
			return Actor;
		}

		FTeamsFixture()
		{
			Teams = UPWTeamsSubsystem::Get(Test.World);
			Teams->CreateFaction("Wild", true);
			RedPlayer = Teams->RegisterParticipant("Red", EPWTeamParticipantType::Player, "Radiant");
			RedMate = Teams->RegisterParticipant("Mate", EPWTeamParticipantType::Player, "Radiant");
			BluePlayer = Teams->RegisterParticipant("Blue", EPWTeamParticipantType::Player, "Dire");

			Hero = Spawn(RedPlayer, NAME_None, true);
			Summon = Spawn(RedPlayer);
			Mate = Spawn(RedMate, NAME_None, true);
			Enemy = Spawn(BluePlayer, NAME_None, true);
			EnemyMinion = Spawn(FPWTeamParticipantHandle(), "Dire");
			Wildlife = Spawn(FPWTeamParticipantHandle(), "Wild");
			Crate = Test.SpawnASC()->GetOwner();
		}

		bool Has(const AActor* Source, const AActor* Target, const FGameplayTag& Tag) const
		{
			return Teams->GetTargetTags(UPWTeamsSubsystem::GetIdentity(Source), UPWTeamsSubsystem::GetIdentity(Target)).HasTagExact(Tag);
		}
	};

	FPWTeamTargetFilter MakeFilter(const TArray<FGameplayTag>& Any, const TArray<FGameplayTag>& All = {}, const TArray<FGameplayTag>& None = {})
	{
		FPWTeamTargetFilter Filter;
		for (const FGameplayTag& Tag : Any) Filter.AnyOf.AddTag(Tag);
		for (const FGameplayTag& Tag : All) Filter.AllOf.AddTag(Tag);
		for (const FGameplayTag& Tag : None) Filter.NoneOf.AddTag(Tag);
		return Filter;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTeamsDefaultRulesTest, "PW.Teams.Relationships.DefaultRules", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTeamsDefaultRulesTest::RunTest(const FString& Parameters)
{
	const FTeamsFixture F;
	using Teams = UPWTeamsSubsystem;

	TestEqual("The same actor is friendly", Teams::GetActorRelationship(F.Hero, F.Hero).Attitude, EPWTeamAttitude::Friendly);
	TestTrue("And is the same actor", Teams::GetActorRelationship(F.Hero, F.Hero).bSameActor);
	TestEqual("The same faction is friendly", Teams::GetActorRelationship(F.Hero, F.Mate).Attitude, EPWTeamAttitude::Friendly);
	TestEqual("Different factions are hostile", Teams::GetActorRelationship(F.Hero, F.Enemy).Attitude, EPWTeamAttitude::Hostile);
	TestEqual("A neutral faction is neutral", Teams::GetActorRelationship(F.Hero, F.Wildlife).Attitude, EPWTeamAttitude::Neutral);
	TestEqual("Neutral works both ways", Teams::GetActorRelationship(F.Wildlife, F.Hero).Attitude, EPWTeamAttitude::Neutral);
	TestEqual("No faction is unknown", Teams::GetActorRelationship(F.Hero, F.Crate).Attitude, EPWTeamAttitude::Unknown);
	TestEqual("A missing actor is unknown", Teams::GetActorRelationship(F.Hero, nullptr).Attitude, EPWTeamAttitude::Unknown);

	TestTrue("A summon has its caster's owner", Teams::IsSameOwner(F.Hero, F.Summon));
	TestTrue("And faction", Teams::IsSameFaction(F.Hero, F.Summon));
	TestFalse("A teammate has the faction but not the owner", Teams::IsSameOwner(F.Hero, F.Mate));
	TestTrue("A teammate is an ally", Teams::IsAlly(F.Hero, F.Mate));
	TestTrue("A faction minion with no owner is still an enemy", Teams::IsEnemy(F.Hero, F.EnemyMinion));

	// A turret the player owns that was captured by the other side.
	AActor* Turret = F.Spawn(F.RedPlayer, "Dire");
	const FPWTeamRelationship Captured = Teams::GetActorRelationship(F.Hero, Turret);
	TestTrue("Same owner", Captured.bSameOwner);
	TestFalse("Different faction", Captured.bSameFaction);
	TestEqual("Owning it does not make it friendly", Captured.Attitude, EPWTeamAttitude::Hostile);

	F.Teams->DefaultAttitude = EPWTeamAttitude::Neutral;
	TestEqual("The default between factions can be changed", Teams::GetActorRelationship(F.Hero, F.Enemy).Attitude, EPWTeamAttitude::Neutral);
	TestEqual("Same faction stays friendly", Teams::GetActorRelationship(F.Hero, F.Mate).Attitude, EPWTeamAttitude::Friendly);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTeamsTargetTagsTest, "PW.Teams.Relationships.TargetTags", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTeamsTargetTagsTest::RunTest(const FString& Parameters)
{
	const FTeamsFixture F;
	using namespace PWTeamTags;

	TestTrue("Self is self", F.Has(F.Hero, F.Hero, Relation_Self));
	TestFalse("Self is not its own ally", F.Has(F.Hero, F.Hero, Relation_Ally));
	TestFalse("Nor its own owner match", F.Has(F.Hero, F.Hero, Relation_SameOwner));

	TestTrue("A summon is same owner", F.Has(F.Hero, F.Summon, Relation_SameOwner));
	TestTrue("And same faction", F.Has(F.Hero, F.Summon, Relation_SameFaction));
	TestTrue("And an ally, all at once", F.Has(F.Hero, F.Summon, Relation_Ally));
	TestFalse("But not self", F.Has(F.Hero, F.Summon, Relation_Self));

	TestTrue("A teammate is an ally", F.Has(F.Hero, F.Mate, Relation_Ally));
	TestFalse("With a different owner", F.Has(F.Hero, F.Mate, Relation_SameOwner));
	TestTrue("An enemy", F.Has(F.Hero, F.Enemy, Relation_Enemy));
	TestTrue("A neutral", F.Has(F.Hero, F.Wildlife, Relation_Neutral));
	TestTrue("A world object is unaffiliated", F.Has(F.Hero, F.Crate, Relation_Unaffiliated));
	TestFalse("And not neutral", F.Has(F.Hero, F.Crate, Relation_Neutral));

	TestTrue("A hero is primary", F.Has(F.Hero, F.Enemy, Role_Primary));
	TestFalse("A minion is not", F.Has(F.Hero, F.EnemyMinion, Role_Primary));

	const FGameplayTag Structure = FGameplayTag::RequestGameplayTag("Event.Input");
	FGameplayTagContainer AgentTags;
	AgentTags.AddTag(Structure);
	UPWTeamsSubsystem::FindAgent(F.EnemyMinion)->SetAgentTags(AgentTags);
	TestTrue("An agent's own tags describe it too", F.Has(F.Hero, F.EnemyMinion, Structure));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTeamsTargetFilterTest, "PW.Teams.Relationships.Filter", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTeamsTargetFilterTest::RunTest(const FString& Parameters)
{
	const FTeamsFixture F;
	using namespace PWTeamTags;
	using Teams = UPWTeamsSubsystem;

	const FPWTeamTargetFilter Everyone;
	TestTrue("An empty filter accepts everyone", Teams::ActorMatchesFilter(F.Hero, F.Enemy, Everyone));
	TestTrue("Even a world object", Teams::ActorMatchesFilter(F.Hero, F.Crate, Everyone));

	const FPWTeamTargetFilter Fireball = MakeFilter({Relation_Enemy, Relation_Unaffiliated});
	TestTrue("Fireball hits an enemy", Teams::ActorMatchesFilter(F.Hero, F.Enemy, Fireball));
	TestTrue("And a crate", Teams::ActorMatchesFilter(F.Hero, F.Crate, Fireball));
	TestFalse("Not an ally", Teams::ActorMatchesFilter(F.Hero, F.Mate, Fireball));
	TestFalse("Not the caster", Teams::ActorMatchesFilter(F.Hero, F.Hero, Fireball));
	TestFalse("Not a neutral", Teams::ActorMatchesFilter(F.Hero, F.Wildlife, Fireball));

	const FPWTeamTargetFilter HeroesOnly = MakeFilter({Relation_Enemy}, {Role_Primary});
	TestTrue("Heroes only hits the enemy hero", Teams::ActorMatchesFilter(F.Hero, F.Enemy, HeroesOnly));
	TestFalse("Not its minion", Teams::ActorMatchesFilter(F.Hero, F.EnemyMinion, HeroesOnly));
	TestFalse("Not an allied hero", Teams::ActorMatchesFilter(F.Hero, F.Mate, HeroesOnly));

	const FPWTeamTargetFilter Detonate = MakeFilter({Relation_SameOwner});
	TestTrue("Detonate hits my summon", Teams::ActorMatchesFilter(F.Hero, F.Summon, Detonate));
	TestFalse("Not me", Teams::ActorMatchesFilter(F.Hero, F.Hero, Detonate));
	TestFalse("Not a teammate", Teams::ActorMatchesFilter(F.Hero, F.Mate, Detonate));

	const FPWTeamTargetFilter HealOthers = MakeFilter({Relation_Ally}, {}, {Relation_SameOwner});
	TestTrue("Heal reaches a teammate", Teams::ActorMatchesFilter(F.Hero, F.Mate, HealOthers));
	TestFalse("But skips my own summon", Teams::ActorMatchesFilter(F.Hero, F.Summon, HealOthers));

	const FPWTeamTargetFilter AnyRelation = MakeFilter({Relation});
	TestTrue("A parent tag matches every relation under it", Teams::ActorMatchesFilter(F.Hero, F.Wildlife, AnyRelation));

	TestEqual("A list of actors can be filtered in one go", Teams::FilterActors(F.Hero, {F.Hero, F.Summon, F.Mate, F.Enemy, F.EnemyMinion, F.Wildlife, F.Crate}, Fireball).Num(), 3);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTeamsProviderTest, "PW.Teams.Relationships.Provider", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTeamsProviderTest::RunTest(const FString& Parameters)
{
	const FTeamsFixture F;
	using namespace PWTeamTags;
	using Teams = UPWTeamsSubsystem;

	const FPWTeamParticipantHandle GreenPlayer = F.Teams->RegisterParticipant("Green", EPWTeamParticipantType::Player, "Sentinel");
	AActor* Green = F.Spawn(GreenPlayer, NAME_None, true);
	TestTrue("A third faction starts hostile", Teams::IsEnemy(F.Hero, Green));

	TestNull("An abstract provider cannot be added", F.Teams->AddRelationshipProvider(UPWTeamRelationshipProvider::StaticClass()));

	UPWTestBlocProvider* Blocs = Cast<UPWTestBlocProvider>(F.Teams->AddRelationshipProvider(UPWTestBlocProvider::StaticClass()));
	if (!TestNotNull("A game can add its own rules", Blocs))
		return false;

	Blocs->SameBlocTag = FGameplayTag::RequestGameplayTag("Input.Cancel");
	Blocs->BlocOfFaction.Add("Radiant", "West");
	Blocs->BlocOfFaction.Add("Sentinel", "West");
	Blocs->BlocOfFaction.Add("Dire", "East");

	TestTrue("Two factions in one bloc are friendly", Teams::IsAlly(F.Hero, Green));
	TestFalse("Without being the same faction", Teams::IsSameFaction(F.Hero, Green));
	TestTrue("A faction in another bloc stays hostile", Teams::IsEnemy(F.Hero, F.Enemy));
	TestTrue("The provider's own tag reaches the filters", F.Has(F.Hero, Green, Blocs->SameBlocTag));

	const FPWTeamTargetFilter Fireball = MakeFilter({Relation_Enemy});
	TestFalse("Abilities follow the game's rules", Teams::ActorMatchesFilter(F.Hero, Green, Fireball));

	const FPWTeamTargetFilter BlocOnly = MakeFilter({Blocs->SameBlocTag}, {}, {Relation_SameFaction});
	TestTrue("A filter can ask for the game's own tag", Teams::ActorMatchesFilter(F.Hero, Green, BlocOnly));
	TestFalse("And tell it from the built-in ones", Teams::ActorMatchesFilter(F.Hero, F.Mate, BlocOnly));

	F.Teams->RemoveRelationshipProvider(Blocs);
	TestTrue("Removing the rules restores the default", Teams::IsEnemy(F.Hero, Green));

	return true;
}

#endif
