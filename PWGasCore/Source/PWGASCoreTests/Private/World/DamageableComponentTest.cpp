// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "Ability/PWSpellParams.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Effect.h"
#include "World/PWDamageableActor.h"
#include "World/PWDamageableComponent.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

static UPWSpellParams* MakeHitParams(AActor* Source, const TSubclassOf<UGameplayEffect> Effect)
{
	UPWSpellParams* Params = NewObject<UPWSpellParams>();
	Params->AbilityLevel = 1;
	Params->SourceAvatar = Source;
	Params->AddExtension<UPWEffectSpellParams>()->Groups.AddDefaulted_GetRef().Effects.Add({Effect, 1});
	return Params;
}

static APWDamageableActor* SpawnDamageable(const FPWTestWorld& Test, const float MaxHealth)
{
	APWDamageableActor* Actor = Test.World->SpawnActorDeferred<APWDamageableActor>(APWDamageableActor::StaticClass(), FTransform::Identity);
	Actor->GetDamageable()->DamageAttribute = UPWTestAttributeSet::GetHealthAttribute();
	Actor->GetDamageable()->MaxHealth = MaxHealth;
	Actor->FinishSpawning(FTransform::Identity);
	return Actor;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDamageableInstantTest, "PW.World.Damageable.Instant", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDamageableInstantTest::RunTest(const FString& Parameters)
{
	const FPWTestWorld Test;
	APWDamageableActor* Actor = SpawnDamageable(Test, 100.f);
	UPWDamageableComponent* Damageable = Actor->GetDamageable();
	TestEqual("Starts at max health", Damageable->GetHealth(), 100.f);

	TestTrue("Effect accepted through the interface", IPWAffectable::Execute_ApplyEffects(Actor, MakeHitParams(Test.Actor, UPWTestInstantHitEffect::StaticClass())));
	TestEqual("Instant damage", Damageable->GetHealth(), 75.f);

	TestFalse("An effect on another attribute is ignored", IPWAffectable::Execute_ApplyEffects(Actor, MakeHitParams(Test.Actor, UPWTestEffect::StaticClass())));
	TestEqual("Health unchanged", Damageable->GetHealth(), 75.f);

	Damageable->ApplyDamage(-500.f, nullptr);
	TestEqual("Healing stops at max", Damageable->GetHealth(), 100.f);

	Damageable->ApplyDamage(500.f, Test.Actor);
	TestTrue("Depleted", Damageable->IsDepleted());
	TestTrue("Owner destroyed", Actor->IsActorBeingDestroyed());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDamageableOverTimeTest, "PW.World.Damageable.OverTime", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDamageableOverTimeTest::RunTest(const FString& Parameters)
{
	const FPWTestWorld Test;
	{
		const APWDamageableActor* Actor = SpawnDamageable(Test, 100.f);
		UPWDamageableComponent* Damageable = Actor->GetDamageable();

		Damageable->ApplySpellParams(MakeHitParams(Test.Actor, UPWTestTimedHitEffect::StaticClass()));
		TestEqual("Nothing before the first period", Damageable->GetHealth(), 100.f);

		for (int32 i = 0; i < 4; ++i)
			Test.TickTimers(0.25f);
		TestTrue("Damage ticks", Damageable->GetHealth() < 100.f);

		for (int32 i = 0; i < 8; ++i)
			Test.TickTimers(0.25f);
		const float AfterDuration = Damageable->GetHealth();
		TestTrue("No more than the duration allows", AfterDuration >= 60.f);

		for (int32 i = 0; i < 8; ++i)
			Test.TickTimers(0.25f);
		TestEqual("Stops after the duration", Damageable->GetHealth(), AfterDuration);
	}
	{
		const APWDamageableActor* Actor = SpawnDamageable(Test, 100.f);
		UPWDamageableComponent* Damageable = Actor->GetDamageable();

		Damageable->ApplySpellParams(MakeHitParams(Test.Actor, UPWTestTimedHitEffect::StaticClass()));
		Damageable->ClearOngoingEffects();
		for (int32 i = 0; i < 8; ++i)
			Test.TickTimers(0.25f);
		TestEqual("Cleared effects do not tick", Damageable->GetHealth(), 100.f);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDamageableFromModuleTest, "PW.World.Damageable.FromEffectModule", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDamageableFromModuleTest::RunTest(const FString& Parameters)
{
	const FPWTestWorld Test;
	UPWTestModularAbility* Ability = Test.ActivateAbility<UPWTestModularAbility>();

	UPWAbilityModule_Effect* Effect = NewObject<UPWAbilityModule_Effect>(Ability);
	Effect->Initialize(Ability);
	Effect->EffectGroups.AddDefaulted_GetRef().Effects.Add({UPWTestInstantHitEffect::StaticClass(), 1});

	APWDamageableActor* Actor = SpawnDamageable(Test, 100.f);
	UPWDamageableComponent* Damageable = Actor->GetDamageable();

	Effect->EffectGroups[0].TargetMask = static_cast<int32>(EPWRelationMask::Enemy);
	Effect->ApplyEffectsToActor(Test.ASC, Actor);
	TestEqual("A neutral actor is skipped by an enemy only group", Damageable->GetHealth(), 100.f);

	Effect->EffectGroups[0].TargetMask = static_cast<int32>(EPWRelationMask::All);
	Effect->ApplyEffectsToActor(Test.ASC, Actor);
	TestEqual("An actor without an ability system takes the effect", Damageable->GetHealth(), 75.f);

	Effect->EffectGroups.Reset();
	Effect->EffectGroups.AddDefaulted_GetRef().Effects.Add({UPWTestPeriodicDamageEffect::StaticClass(), 1});
	Effect->ApplyEffectsToActor(Test.ASC, Actor);
	for (int32 i = 0; i < 6; ++i)
		Test.TickTimers(0.25f);
	const float WhileInside = Damageable->GetHealth();
	TestTrue("An infinite effect keeps ticking", WhileInside != 75.f);

	Effect->RemoveEffectsFromActor(Test.ASC, Actor);
	for (int32 i = 0; i < 6; ++i)
		Test.TickTimers(0.25f);
	TestEqual("Removing it stops the ticks", Damageable->GetHealth(), WhileInside);
	return true;
}

#endif
