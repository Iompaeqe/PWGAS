// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "TeamComponent.h"
#include "TeamFunctions.h"
#include "Ability/PWSpellParams.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

static UPWAbilitySystemComponent* SpawnTeamMember(const FPWTestWorld& Test, const int32 TeamId)
{
	UPWAbilitySystemComponent* ASC = Test.SpawnASC();
	UTeamComponent* Team = NewObject<UTeamComponent>(ASC->GetOwner());
	Team->RegisterComponent();
	Team->TeamId = TeamId;
	return ASC;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTeamFunctionsRelationTest, "PW.Teams.Relation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTeamFunctionsRelationTest::RunTest(const FString& Parameters)
{
	FPWTestWorld Test;
	const AActor* Red = SpawnTeamMember(Test, 1)->GetOwner();
	const AActor* RedAlly = SpawnTeamMember(Test, 1)->GetOwner();
	const AActor* Blue = SpawnTeamMember(Test, 2)->GetOwner();

	TestEqual("Team id", UTeamFunctions::GetTeamId(Red), 1);
	TestEqual("No team", UTeamFunctions::GetTeamId(Test.Actor), 0);

	TestEqual("Self", UTeamFunctions::GetRelation(Red, Red), ERelation::Self);
	TestEqual("Ally", UTeamFunctions::GetRelation(Red, RedAlly), ERelation::Ally);
	TestEqual("Enemy", UTeamFunctions::GetRelation(Red, Blue), ERelation::Enemy);
	TestEqual("No team is neutral", UTeamFunctions::GetRelation(Red, Test.Actor), ERelation::Neutral);
	TestEqual("A missing actor is unknown", UTeamFunctions::GetRelation(Red, nullptr), ERelation::Unknown);

	const uint8 EnemyMask = static_cast<uint8>(ERelationMask::Enemy);
	TestTrue("Enemy matches enemy mask", UTeamFunctions::MatchesMask(Red, Blue, EnemyMask));
	TestFalse("Ally does not", UTeamFunctions::MatchesMask(Red, RedAlly, EnemyMask));
	TestFalse("Self does not", UTeamFunctions::MatchesMask(Red, Red, EnemyMask));
	TestFalse("Neutral does not match the enemy mask", UTeamFunctions::MatchesMask(Red, Test.Actor, EnemyMask));
	TestTrue("Neutral matches the neutral mask", UTeamFunctions::MatchesMask(Red, Test.Actor, static_cast<uint8>(ERelationMask::Neutral)));
	TestTrue("Neutral matches enemy plus neutral", UTeamFunctions::MatchesMask(Red, Test.Actor, static_cast<uint8>(ERelationMask::Enemy | ERelationMask::Neutral)));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEffectModuleTargetMaskTest, "PW.Modules.Effect.TargetMask", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEffectModuleTargetMaskTest::RunTest(const FString& Parameters)
{
	FPWTestWorld Test;
	UPWAbilitySystemComponent* Caster = SpawnTeamMember(Test, 1);
	UPWAbilitySystemComponent* Ally = SpawnTeamMember(Test, 1);
	UPWAbilitySystemComponent* Enemy = SpawnTeamMember(Test, 2);

	const FGameplayAbilitySpecHandle Handle = Caster->GiveAbility(FGameplayAbilitySpec(UPWTestInstantEffectAbility::StaticClass(), 1));
	Caster->TryActivateAbility(Handle);
	const UPWModularGameplayAbility* Ability = Cast<UPWModularGameplayAbility>(Caster->FindAbilitySpecFromHandle(Handle)->GetPrimaryInstance());
	UPWAbilityModule_Effect* Effect = Ability ? Ability->GetEffectModule() : nullptr;
	if (!TestNotNull("Effect module", Effect))
		return false;

	Effect->EffectGroups[0].TargetMask = static_cast<int32>(ERelationMask::Enemy);
	FPWEffectGroup& AllyGroup = Effect->EffectGroups.AddDefaulted_GetRef();
	AllyGroup.TargetMask = static_cast<int32>(ERelationMask::Ally);
	AllyGroup.Effects.Add({UPWTestDurationEffect::StaticClass(), 1});

	Effect->ApplyEffects(Caster, Caster);
	Effect->ApplyEffects(Caster, Ally);
	Effect->ApplyEffects(Caster, Enemy);

	TestEqual("Self skipped", Caster->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 0);
	TestEqual("Ally skipped by the enemy group", Ally->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 0);
	TestEqual("Enemy hit", Enemy->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 1);
	TestEqual("Ally takes the ally group", Ally->GetGameplayEffectCount(UPWTestDurationEffect::StaticClass(), nullptr), 1);
	TestEqual("Enemy skipped by the ally group", Enemy->GetGameplayEffectCount(UPWTestDurationEffect::StaticClass(), nullptr), 0);
	TestEqual("Combined mask", Effect->GetCombinedMask(), static_cast<int32>(ERelationMask::Enemy | ERelationMask::Ally));

	return true;
}

#endif
