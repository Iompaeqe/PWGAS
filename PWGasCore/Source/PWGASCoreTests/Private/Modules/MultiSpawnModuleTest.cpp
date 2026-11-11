// copyright mkurtt96

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "Targeting/Data/PWTargetingData.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

static FPWTargetingResult MakeTargetInFront(const AActor* Actor, const float Distance)
{
	FPWTargetingResult Target;
	Target.Location = Actor->GetActorLocation() + Actor->GetActorForwardVector() * Distance;
	Target.Rotation = Actor->GetActorRotation();
	return Target;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMultiProjectileSpreadTest, "PW.Modules.MultiProjectile.Spread", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMultiProjectileSpreadTest::RunTest(const FString& Parameters)
{
	FPWTestWorld Test;
	UPWTestModularAbility* Ability = Test.ActivateAbility<UPWTestModularAbility>();

	UPWAbilityModule_MultiProjectile* Module = NewObject<UPWAbilityModule_MultiProjectile>(Ability);
	Module->Initialize(Ability);
	Module->ActorClass = APWTestSpellActor::StaticClass();
	Module->ProjectileCount = 3;
	Module->SpreadAngle = 40.f;

	Module->SpawnProjectileWithTargetingResult(MakeTargetInFront(Test.Actor, 1000.f));

	TArray<APWTestSpellActor*> Projectiles = Test.GetActors<APWTestSpellActor>();
	if (!TestEqual("Projectile count", Projectiles.Num(), 3))
		return false;

	TArray<float> Yaws;
	for (const APWTestSpellActor* Projectile : Projectiles)
		Yaws.Add(FRotator::NormalizeAxis(Projectile->GetActorRotation().Yaw));
	Yaws.Sort();

	TestEqual("Left", Yaws[0], -20.f, 0.1f);
	TestEqual("Center", Yaws[1], 0.f, 0.1f);
	TestEqual("Right", Yaws[2], 20.f, 0.1f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMultiProjectileSpreadPitchTest, "PW.Modules.MultiProjectile.SpreadPitch", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMultiProjectileSpreadPitchTest::RunTest(const FString& Parameters)
{
	FPWTestWorld Test;
	UPWTestModularAbility* Ability = Test.ActivateAbility<UPWTestModularAbility>();

	UPWAbilityModule_MultiProjectile* Module = NewObject<UPWAbilityModule_MultiProjectile>(Ability);
	Module->Initialize(Ability);
	Module->ActorClass = APWTestSpellActor::StaticClass();
	Module->ProjectileCount = 3;
	Module->SpreadAngle = 40.f;

	for (const float Pitch : {45.f, 85.f})
	{
		FPWTargetingResult Target;
		Target.Location = Test.Actor->GetActorLocation() + FRotator(Pitch, 0, 0).Vector() * 1000.f;
		Module->SpawnProjectileWithTargetingResult(Target);

		TArray<APWTestSpellActor*> Projectiles = Test.GetActors<APWTestSpellActor>();
		if (!TestEqual("Projectile count", Projectiles.Num(), 3))
			return false;

		float WidestAngle = 0.f;
		for (const APWTestSpellActor* A : Projectiles)
			for (const APWTestSpellActor* B : Projectiles)
				WidestAngle = FMath::Max(WidestAngle, FMath::RadiansToDegrees(FMath::Acos(A->GetActorForwardVector() | B->GetActorForwardVector())));
		TestEqual(FString::Printf(TEXT("Spread at pitch %.0f"), Pitch), WidestAngle, 40.f, 0.5f);

		for (APWTestSpellActor* Projectile : Projectiles)
			Projectile->Destroy();
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMultiProjectileDelayTest, "PW.Modules.MultiProjectile.Delay", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMultiProjectileDelayTest::RunTest(const FString& Parameters)
{
	FPWTestWorld Test;
	UPWTestModularAbility* Ability = Test.ActivateAbility<UPWTestModularAbility>();

	UPWAbilityModule_MultiProjectile* Module = NewObject<UPWAbilityModule_MultiProjectile>(Ability);
	Module->Initialize(Ability);
	Module->ActorClass = APWTestSpellActor::StaticClass();
	Module->ProjectileCount = 3;
	Module->SpreadDelay = 0.1f;

	const FPWTargetingResult Target = MakeTargetInFront(Test.Actor, 1000.f);

	Module->SpreadSpawnProjectiles(Target);
	TestEqual("First spawns immediately", Test.GetActors<APWTestSpellActor>().Num(), 1);
	Test.TickTimers(0.f);
	Test.TickTimers(0.11f);
	TestEqual("Second after delay", Test.GetActors<APWTestSpellActor>().Num(), 2);
	Test.TickTimers(0.11f);
	Test.TickTimers(0.11f);
	Test.TickTimers(0.11f);
	TestEqual("Stops at count", Test.GetActors<APWTestSpellActor>().Num(), 3);

	Module->SpreadSpawnProjectiles(Target);
	Test.TickTimers(0.f);
	Test.TickTimers(0.11f);
	TestEqual("Second cast keeps the same pace", Test.GetActors<APWTestSpellActor>().Num(), 5);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMultiActorDelayTest, "PW.Modules.MultiActor.Delay", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMultiActorDelayTest::RunTest(const FString& Parameters)
{
	FPWTestWorld Test;
	UPWTestModularAbility* Ability = Test.ActivateAbility<UPWTestModularAbility>();

	UPWAbilityModule_MultiActor* Module = NewObject<UPWAbilityModule_MultiActor>(Ability);
	Module->Initialize(Ability);
	Module->ActorClass = APWTestSpellActor::StaticClass();
	Module->ActorCount = 3;
	Module->Radius = 200.f;
	Module->SpawnDelay = 0.1f;

	const FPWTargetingResult Target = MakeTargetInFront(Test.Actor, 1000.f);

	Module->SpawnMultipleActors(Target);
	TestEqual("First spawns immediately", Test.GetActors<APWTestSpellActor>().Num(), 1);
	Test.TickTimers(0.f);
	Test.TickTimers(0.11f);
	TestEqual("Second after delay", Test.GetActors<APWTestSpellActor>().Num(), 2);
	Test.TickTimers(0.11f);
	Test.TickTimers(0.11f);
	Test.TickTimers(0.11f);
	TestEqual("Stops at count", Test.GetActors<APWTestSpellActor>().Num(), 3);

	Module->SpawnMultipleActors(Target);
	Test.TickTimers(0.f);
	Test.TickTimers(0.11f);
	TestEqual("Second cast keeps the same pace", Test.GetActors<APWTestSpellActor>().Num(), 5);

	return true;
}

#endif
