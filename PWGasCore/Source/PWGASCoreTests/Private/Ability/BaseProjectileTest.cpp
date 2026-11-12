// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestWorld.h"
#include "BaseProjectile.h"
#include "Ability/SpellParams.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

static ABaseProjectile* SpawnProjectile(const FPWTestWorld& Test, const FGuid& CastId, const FVector& Location)
{
	const FTransform Transform(Location);
	ABaseProjectile* Projectile = Test.World->SpawnActorDeferred<ABaseProjectile>(ABaseProjectile::StaticClass(), Transform, Test.Actor, Cast<APawn>(Test.Actor), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	USpellParams* Params = NewObject<USpellParams>(Projectile);
	Params->SourceAvatar = Test.Actor;
	Params->CastId = CastId;
	IPWSpellParamsInterface::Execute_SetSpellParams(Projectile, Params);

	Projectile->FinishSpawning(Transform);
	return Projectile;
}

struct FOverlapParams
{
	UPrimitiveComponent* OverlappedComponent = nullptr;
	AActor* OtherActor = nullptr;
	UPrimitiveComponent* OtherComp = nullptr;
	int32 OtherBodyIndex = 0;
	bool bFromSweep = false;
	FHitResult SweepResult;
};

static void Overlap(ABaseProjectile* Projectile, AActor* OtherActor)
{
	FOverlapParams Params;
	Params.OtherActor = OtherActor;
	Projectile->ProcessEvent(Projectile->FindFunctionChecked("OnOverlap"), &Params);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBaseProjectileSameCastTest, "PW.Ability.BaseProjectile.SameCast", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBaseProjectileSameCastTest::RunTest(const FString& Parameters)
{
	FPWTestWorld Test;
	const FGuid CastId = FGuid::NewGuid();

	ABaseProjectile* First = SpawnProjectile(Test, CastId, FVector(0, 0, 500));
	ABaseProjectile* Second = SpawnProjectile(Test, CastId, FVector(0, 300, 500));
	ABaseProjectile* Other = SpawnProjectile(Test, FGuid::NewGuid(), FVector(0, 600, 500));

	Overlap(First, Second);
	Overlap(Second, First);
	TestTrue("Projectiles of the same cast pass through each other", IsValid(First) && IsValid(Second));

	Overlap(Other, First);
	TestFalse("Another cast of the same caster still collides", IsValid(Other));

	return true;
}

#endif
