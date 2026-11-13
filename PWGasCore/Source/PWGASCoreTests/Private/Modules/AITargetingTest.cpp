// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "AIController.h"
#include "GAS/Abilities/Modules/ActionModules/PWAbilityModule_Projectile.h"
#include "Targeting/PresetupPolicies/Targeting/PWTargeting_ScreenCenter.h"
#include "Targeting/PresetupPolicies/Targeting/PWTargeting_Self.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAITargetingTest, "PW.Modules.Targeting.AICaster", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAITargetingTest::RunTest(const FString& Parameters)
{
	FPWTestWorld Test;
	APawn* Pawn = Cast<APawn>(Test.Actor);
	AAIController* AI = Test.World->SpawnActor<AAIController>();
	AI->Possess(Pawn);
	Test.ASC->InitAbilityActorInfo(Pawn, Pawn);

	AActor* Enemy = Test.SpawnASC()->GetOwner();
	Enemy->SetActorLocation(FVector(800.f, 300.f, 0.f));

	UPWTestModularAbility* Ability = Test.ActivateAbility<UPWTestModularAbility>();
	UPWAbilityModule_Targeting* Targeting = NewObject<UPWAbilityModule_Targeting>(Ability);
	Targeting->TargetingSource = NewObject<UPWTargeting_ScreenCenter>(Targeting);
	Targeting->Initialize(Ability);
	Ability->ControlModules.Add(Targeting);

	FPWTargetingResult Result;
	TestTrue("A player targeting source works for an AI caster", Targeting->ComputeTarget(Result));
	TestEqual("With no focus it aims straight ahead", Result.Location, Pawn->GetActorLocation() + Pawn->GetActorForwardVector() * 1000.f);

	AI->SetFocus(Enemy);
	TestTrue("Computes with a focus", Targeting->ComputeTarget(Result));
	TestEqual("It aims at the focus actor", Result.Location, Enemy->GetActorLocation());
	TestTrue("The focus actor is the hit actor", Result.Hit.GetActor() == Enemy);

	Targeting->TargetingSource = NewObject<UPWTargeting_Self>(Targeting);
	TestTrue("Self targeting computes", Targeting->ComputeTarget(Result));
	TestEqual("Self targeting stays on the caster", Result.Location, Pawn->GetActorLocation());

	Targeting->TargetingSource = NewObject<UPWTargeting_ScreenCenter>(Targeting);
	UPWAbilityModule_Projectile* Projectile = NewObject<UPWAbilityModule_Projectile>(Ability);
	Projectile->Initialize(Ability);
	Projectile->ActorClass = APWTestSpellActor::StaticClass();
	Projectile->SpawnProjectile();

	const TArray<APWTestSpellActor*> Spawned = Test.GetActors<APWTestSpellActor>();
	if (!TestEqual("The AI fires a projectile", Spawned.Num(), 1))
		return false;
	TestTrue("At its focus", Spawned[0]->GetActorRotation().Equals((Enemy->GetActorLocation() - Pawn->GetActorLocation()).Rotation(), 0.5f));
	return true;
}

#endif
