// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "PWTeamComponent.h"
#include "PWTeamFunctions.h"
#include "Ability/PWSpellParams.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

static UPWAbilitySystemComponent* SpawnTeamMember(const FPWTestWorld& Test, const int32 TeamId)
{
	UPWAbilitySystemComponent* ASC = Test.SpawnASC();
	UPWTeamComponent* Team = NewObject<UPWTeamComponent>(ASC->GetOwner());
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

	TestEqual("Team id", UPWTeamFunctions::GetTeamId(Red), 1);
	TestEqual("No team", UPWTeamFunctions::GetTeamId(Test.Actor), 0);

	TestEqual("Self", UPWTeamFunctions::GetRelation(Red, Red), EPWRelation::Self);
	TestEqual("Ally", UPWTeamFunctions::GetRelation(Red, RedAlly), EPWRelation::Ally);
	TestEqual("Enemy", UPWTeamFunctions::GetRelation(Red, Blue), EPWRelation::Enemy);
	TestEqual("No team is neutral", UPWTeamFunctions::GetRelation(Red, Test.Actor), EPWRelation::Neutral);
	TestEqual("A missing actor is unknown", UPWTeamFunctions::GetRelation(Red, nullptr), EPWRelation::Unknown);

	const uint8 EnemyMask = static_cast<uint8>(EPWRelationMask::Enemy);
	TestTrue("Enemy matches enemy mask", UPWTeamFunctions::MatchesMask(Red, Blue, EnemyMask));
	TestFalse("Ally does not", UPWTeamFunctions::MatchesMask(Red, RedAlly, EnemyMask));
	TestFalse("Self does not", UPWTeamFunctions::MatchesMask(Red, Red, EnemyMask));
	TestFalse("Neutral does not match the enemy mask", UPWTeamFunctions::MatchesMask(Red, Test.Actor, EnemyMask));
	TestTrue("Neutral matches the neutral mask", UPWTeamFunctions::MatchesMask(Red, Test.Actor, static_cast<uint8>(EPWRelationMask::Neutral)));
	TestTrue("Neutral matches enemy plus neutral", UPWTeamFunctions::MatchesMask(Red, Test.Actor, static_cast<uint8>(EPWRelationMask::Enemy | EPWRelationMask::Neutral)));

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

	Effect->EffectGroups[0].TargetMask = static_cast<int32>(EPWRelationMask::Enemy);
	FPWEffectGroup& AllyGroup = Effect->EffectGroups.AddDefaulted_GetRef();
	AllyGroup.TargetMask = static_cast<int32>(EPWRelationMask::Ally);
	AllyGroup.Effects.Add({UPWTestDurationEffect::StaticClass(), 1});

	Effect->ApplyEffects(Caster, Caster);
	Effect->ApplyEffects(Caster, Ally);
	Effect->ApplyEffects(Caster, Enemy);

	TestEqual("Self skipped", Caster->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 0);
	TestEqual("Ally skipped by the enemy group", Ally->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 0);
	TestEqual("Enemy hit", Enemy->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 1);
	TestEqual("Ally takes the ally group", Ally->GetGameplayEffectCount(UPWTestDurationEffect::StaticClass(), nullptr), 1);
	TestEqual("Enemy skipped by the ally group", Enemy->GetGameplayEffectCount(UPWTestDurationEffect::StaticClass(), nullptr), 0);
	TestEqual("Combined mask", Effect->GetCombinedMask(), static_cast<int32>(EPWRelationMask::Enemy | EPWRelationMask::Ally));

	return true;
}

#endif
