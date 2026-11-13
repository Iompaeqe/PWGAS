// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "Core/PWTeamsSubsystem.h"
#include "Core/PWTeamsTags.h"
#include "Agents/PWTeamAgentComponent.h"
#include "Ability/PWSpellParams.h"
#include "Libraries/PWGasFunctions.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Effect.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

static UPWAbilitySystemComponent* SpawnMember(const FPWTestWorld& Test, const FName Faction)
{
	UPWAbilitySystemComponent* ASC = Test.SpawnASC();
	UPWTeamAgentComponent* Agent = NewObject<UPWTeamAgentComponent>(ASC->GetOwner());
	Agent->RegisterComponent();
	Agent->SetOwnerParticipant(UPWTeamsSubsystem::Get(Test.World)->RegisterParticipant(Faction, EPWTeamParticipantType::Player, Faction));
	return ASC;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEffectModuleTargetFilterTest, "PW.Modules.Effect.TargetFilter", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEffectModuleTargetFilterTest::RunTest(const FString& Parameters)
{
	const FPWTestWorld Test;
	UPWTeamsSubsystem* Teams = UPWTeamsSubsystem::Get(Test.World);
	UPWAbilitySystemComponent* Caster = SpawnMember(Test, "Radiant");
	UPWAbilitySystemComponent* Ally = SpawnMember(Test, "Radiant");
	UPWAbilitySystemComponent* Enemy = SpawnMember(Test, "Dire");

	const FGameplayAbilitySpecHandle Handle = Caster->GiveAbility(FGameplayAbilitySpec(UPWTestInstantEffectAbility::StaticClass(), 1));
	Caster->TryActivateAbility(Handle);
	const UPWModularGameplayAbility* Ability = Cast<UPWModularGameplayAbility>(Caster->FindAbilitySpecFromHandle(Handle)->GetPrimaryInstance());
	UPWAbilityModule_Effect* Effect = Ability ? Ability->GetEffectModule() : nullptr;
	if (!TestNotNull("Effect module", Effect))
		return false;

	Effect->EffectGroups[0].TargetFilter.AnyOf.AddTag(PWTeamTags::Relation_Enemy);
	FPWEffectGroup& AllyGroup = Effect->EffectGroups.AddDefaulted_GetRef();
	AllyGroup.TargetFilter.AnyOf.AddTag(PWTeamTags::Relation_Ally);
	AllyGroup.Effects.Add({UPWTestDurationEffect::StaticClass(), 1});

	Effect->ApplyEffects(Caster, Caster);
	Effect->ApplyEffects(Caster, Ally);
	Effect->ApplyEffects(Caster, Enemy);

	TestEqual("Self skipped by the enemy group", Caster->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 0);
	TestEqual("Self is not its own ally", Caster->GetGameplayEffectCount(UPWTestDurationEffect::StaticClass(), nullptr), 0);
	TestEqual("Ally skipped by the enemy group", Ally->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 0);
	TestEqual("Enemy hit", Enemy->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 1);
	TestEqual("Ally takes the ally group", Ally->GetGameplayEffectCount(UPWTestDurationEffect::StaticClass(), nullptr), 1);
	TestEqual("Enemy skipped by the ally group", Enemy->GetGameplayEffectCount(UPWTestDurationEffect::StaticClass(), nullptr), 0);

	TestTrue("The module applies to an enemy", Effect->AppliesTo(Caster->GetOwner(), Enemy->GetOwner()));
	TestFalse("But to nothing on the caster", Effect->AppliesTo(Caster->GetOwner(), Caster->GetOwner()));

	// The caster changes sides: the same ability now treats its old ally as an enemy.
	Teams->AssignParticipantToFaction(UPWTeamsSubsystem::GetAgentOwner(Caster->GetOwner()), "Dire");
	Effect->ApplyEffects(Caster, Ally);
	TestEqual("Filters follow a change of faction", Ally->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLegacyMaskFilterTest, "PW.Teams.Consumers.LegacyMask", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLegacyMaskFilterTest::RunTest(const FString& Parameters)
{
	using namespace PWTeamTags;

	TestTrue("Every flag ticked becomes the empty filter", UPWGasFunctions::FilterFromLegacyMask(15).IsEmpty());

	const FPWTeamTargetFilter EnemyNeutral = UPWGasFunctions::FilterFromLegacyMask(12);
	TestTrue("Enemy carries over", EnemyNeutral.AnyOf.HasTagExact(Relation_Enemy));
	TestTrue("Neutral carries over", EnemyNeutral.AnyOf.HasTagExact(Relation_Neutral));
	TestTrue("And also covers what has no faction, as it used to", EnemyNeutral.AnyOf.HasTagExact(Relation_Unaffiliated));
	TestFalse("Ally was not ticked", EnemyNeutral.AnyOf.HasTagExact(Relation_Ally));

	const FPWTeamTargetFilter SelfOnly = UPWGasFunctions::FilterFromLegacyMask(1);
	TestEqual("Self alone", SelfOnly.AnyOf.Num(), 1);
	TestTrue("Is self", SelfOnly.AnyOf.HasTagExact(Relation_Self));

	const FPWTeamTargetFilter Nobody = UPWGasFunctions::FilterFromLegacyMask(0);
	FGameplayTagContainer AnEnemy;
	AnEnemy.AddTag(Relation_Enemy);
	TestFalse("Nothing ticked still reaches nobody", Nobody.Matches(AnEnemy));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpellParamsSourceIdentityTest, "PW.Teams.Consumers.SourceIdentity", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSpellParamsSourceIdentityTest::RunTest(const FString& Parameters)
{
	const FPWTestWorld Test;
	UPWTeamsSubsystem* Teams = UPWTeamsSubsystem::Get(Test.World);
	UPWAbilitySystemComponent* Caster = SpawnMember(Test, "Radiant");
	UPWAbilitySystemComponent* Enemy = SpawnMember(Test, "Dire");

	UPWTestModularAbility* Ability = nullptr;
	{
		const FGameplayAbilitySpecHandle Handle = Caster->GiveAbility(FGameplayAbilitySpec(UPWTestModularAbility::StaticClass(), 1));
		Caster->TryActivateAbility(Handle);
		Ability = Cast<UPWTestModularAbility>(Caster->FindAbilitySpecFromHandle(Handle)->GetPrimaryInstance());
	}
	if (!TestNotNull("Ability", Ability))
		return false;

	UPWSpellParams* Params = Ability->MakeSpellParams();
	const FPWTeamParticipantHandle CasterParticipant = UPWTeamsSubsystem::GetAgentOwner(Caster->GetOwner());
	TestEqual("The params carry the caster's participant", Params->SourceParticipant, CasterParticipant);
	TestEqual("And resolve the caster's faction", Params->GetSourceIdentity().Faction, FName("Radiant"));

	// The caster is gone before the spell lands.
	Params->SourceAvatar = nullptr;
	const FPWTeamIdentity Source = Params->GetSourceIdentity();
	TestEqual("The spell still knows whose it is", Source.Owner, CasterParticipant);
	TestEqual("And which side it is on", Source.Faction, FName("Radiant"));
	TestEqual("So it still knows its enemies", Teams->GetRelationship(Source, UPWTeamsSubsystem::GetIdentity(Enemy->GetOwner())).Attitude, EPWTeamAttitude::Hostile);

	return true;
}

#endif
