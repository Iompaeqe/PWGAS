// copyright mkurtt96

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "Targeting/Data/PWTargetingData.h"
#include "GAS/Abilities/Modules/ActionModules/SpawnPatterns/PWSpawnPatterns.h"
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMultiActorPositionsTest, "PW.Modules.MultiActor.Positions", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMultiActorPositionsTest::RunTest(const FString& Parameters)
{
	FPWTestWorld Test;
	UPWTestModularAbility* Ability = Test.ActivateAbility<UPWTestModularAbility>();

	UPWAbilityModule_MultiActor* Module = NewObject<UPWAbilityModule_MultiActor>(Ability);
	Module->Initialize(Ability);
	Module->ActorClass = APWTestSpellActor::StaticClass();
	Module->ActorCount = 4;
	Module->Radius = 200.f;

	const FPWTargetingResult Target = MakeTargetInFront(Test.Actor, 1000.f);
	Module->SpawnMultipleActors(Target);

	const TArray<APWTestSpellActor*> Actors = Test.GetActors<APWTestSpellActor>();
	if (!TestEqual("Actor count", Actors.Num(), 4))
		return false;

	for (int32 A = 0; A < Actors.Num(); ++A)
	{
		TestEqual("On the circle around the target", static_cast<float>(FVector::Dist(Actors[A]->GetActorLocation(), Target.Location)), 200.f, 0.5f);
		for (int32 B = A + 1; B < Actors.Num(); ++B)
			TestTrue("Each at its own position", FVector::Dist(Actors[A]->GetActorLocation(), Actors[B]->GetActorLocation()) > 100.f);
	}

	return true;
}

static UPWAbilityModule_MultiProjectile* MakeSpreadModule(const FPWTestWorld& Test, const int32 Count, const EProjectileSpreadPattern Pattern)
{
	UPWTestModularAbility* Ability = Test.ActivateAbility<UPWTestModularAbility>();
	UPWAbilityModule_MultiProjectile* Module = NewObject<UPWAbilityModule_MultiProjectile>(Ability);
	Module->Initialize(Ability);
	Module->ActorClass = APWTestSpellActor::StaticClass();
	Module->ProjectileCount = Count;
	Module->SpreadAngle = 30.f;
	Module->SpreadDelay = 0.1f;
	Module->SpreadPattern = Pattern;
	return Module;
}

static TArray<int32> SpawnStepSizes(const FPWTestWorld& Test, UPWAbilityModule_MultiProjectile* Module)
{
	TArray<int32> Sizes;
	int32 Previous = Test.GetActors<APWTestSpellActor>().Num();

	Module->SpreadSpawnProjectiles(MakeTargetInFront(Test.Actor, 1000.f));
	Test.TickTimers(0.f);
	for (int32 Step = 0; Step < 8; ++Step)
	{
		const int32 Current = Test.GetActors<APWTestSpellActor>().Num();
		if (Current == Previous)
			break;
		Sizes.Add(Current - Previous);
		Previous = Current;
		Test.TickTimers(0.11f);
	}
	return Sizes;
}

static float FirstYaw(const FPWTestWorld& Test)
{
	const TArray<APWTestSpellActor*> Actors = Test.GetActors<APWTestSpellActor>();
	return Actors.Num() > 0 ? FRotator::NormalizeAxis(Actors[0]->GetActorRotation().Yaw) : 0.f;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMultiProjectilePatternsTest, "PW.Modules.MultiProjectile.Patterns", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMultiProjectilePatternsTest::RunTest(const FString& Parameters)
{
	{
		FPWTestWorld Test;
		TestEqual("Left to right, one at a time", SpawnStepSizes(Test, MakeSpreadModule(Test, 4, EProjectileSpreadPattern::LeftToRight)), TArray<int32>({1, 1, 1, 1}));
	}
	{
		FPWTestWorld Test;
		UPWAbilityModule_MultiProjectile* Module = MakeSpreadModule(Test, 4, EProjectileSpreadPattern::CenterFanOutRight);
		Module->SpreadSpawnProjectiles(MakeTargetInFront(Test.Actor, 1000.f));
		TestEqual("Fan out starts right of center", FirstYaw(Test), 5.f, 0.1f);
	}
	{
		FPWTestWorld Test;
		TestEqual("Fan out with an even count fires all four", SpawnStepSizes(Test, MakeSpreadModule(Test, 4, EProjectileSpreadPattern::CenterFanOutLeft)), TArray<int32>({1, 1, 1, 1}));
	}
	{
		FPWTestWorld Test;
		TestEqual("Center out in pairs, odd", SpawnStepSizes(Test, MakeSpreadModule(Test, 5, EProjectileSpreadPattern::CenterOut)), TArray<int32>({1, 2, 2}));
	}
	{
		FPWTestWorld Test;
		TestEqual("Center out in pairs, even", SpawnStepSizes(Test, MakeSpreadModule(Test, 4, EProjectileSpreadPattern::CenterOut)), TArray<int32>({2, 2}));
	}
	{
		FPWTestWorld Test;
		TestEqual("Out in ends on the center", SpawnStepSizes(Test, MakeSpreadModule(Test, 5, EProjectileSpreadPattern::OutIn)), TArray<int32>({2, 2, 1}));
	}
	{
		FPWTestWorld Test;
		TestEqual("Center then all sides", SpawnStepSizes(Test, MakeSpreadModule(Test, 5, EProjectileSpreadPattern::CenterToSides)), TArray<int32>({1, 4}));
	}
	{
		FPWTestWorld Test;
		TestEqual("All sides then center", SpawnStepSizes(Test, MakeSpreadModule(Test, 5, EProjectileSpreadPattern::SidesToCenter)), TArray<int32>({4, 1}));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMultiActorOrientationTest, "PW.Modules.MultiActor.Orientation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMultiActorOrientationTest::RunTest(const FString& Parameters)
{
	FPWTestWorld Test;
	UPWTestModularAbility* Ability = Test.ActivateAbility<UPWTestModularAbility>();

	UPWAbilityModule_MultiActor* Module = NewObject<UPWAbilityModule_MultiActor>(Ability);
	Module->Initialize(Ability);
	Module->ActorClass = APWTestSpellActor::StaticClass();
	Module->ActorCount = 3;
	Module->Radius = 200.f;

	FPWTargetingResult Target;
	Target.Location = FVector(0, 1000, -300);
	Target.Rotation = (Target.Location - Test.Actor->GetActorLocation()).Rotation();
	Test.Actor->SetActorRotation(FRotator(0, 90, 0));

	Module->Pattern = NewObject<UPWSpawnPattern_Line>(Module);
	Module->SpawnMultipleActors(Target);
	for (APWTestSpellActor* Actor : Test.GetActors<APWTestSpellActor>())
	{
		TestEqual("Line stays at the target height", static_cast<float>(Actor->GetActorLocation().Z), -300.f, 0.5f);
		TestEqual("Line runs across the cast direction", static_cast<float>(Actor->GetActorLocation().Y), 1000.f, 0.5f);
		Actor->Destroy();
	}

	Module->Pattern = NewObject<UPWSpawnPattern_Circle>(Module);
	Module->SpawnMultipleActors(Target);
	const TArray<APWTestSpellActor*> Circle = Test.GetActors<APWTestSpellActor>();
	if (TestEqual("Circle count", Circle.Num(), 3))
		TestTrue("Circle starts on the far side of the target", Circle.ContainsByPredicate([&](const APWTestSpellActor* Actor) { return Actor->GetActorLocation().Equals(FVector(0, 1200, -300), 0.5f); }));

	const auto HasActorAt = [&Test](const FVector& Location)
	{
		return Test.GetActors<APWTestSpellActor>().ContainsByPredicate([&](const APWTestSpellActor* Actor) { return Actor->GetActorLocation().Equals(Location, 0.5f); });
	};
	const auto Respawn = [&]()
	{
		for (APWTestSpellActor* Actor : Test.GetActors<APWTestSpellActor>())
			Actor->Destroy();
		Module->SpawnMultipleActors(Target);
	};

	Module->PatternOrientation = EActorPatternOrientation::World;
	Respawn();
	TestTrue("World orientation starts on world forward", HasActorAt(FVector(200, 1000, -300)));

	Module->PatternOrientation = EActorPatternOrientation::Caster;
	Module->PatternRotation = FRotator(0, 90, 0);
	Respawn();
	TestTrue("Pattern rotation turns the pattern from the cast direction", HasActorAt(FVector(-200, 1000, -300)));

	Module->PatternRotation = FRotator::ZeroRotator;
	Module->PatternOffset = FVector(100, 0, 0);
	Respawn();
	TestTrue("Pattern offset moves the center along the cast direction", HasActorAt(FVector(0, 1300, -300)));

	Module->PatternOffset = FVector::ZeroVector;
	Module->PatternRotation = FRotator(90, 0, 0);
	Respawn();
	TestTrue("Pitched pattern stands upright", HasActorAt(FVector(0, 1000, -100)));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMultiActorSpreadLineTest, "PW.Modules.MultiActor.SpreadLine", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMultiActorSpreadLineTest::RunTest(const FString& Parameters)
{
	for (const int32 Count : {4, 5})
	{
		FPWTestWorld Test;
		UPWTestModularAbility* Ability = Test.ActivateAbility<UPWTestModularAbility>();

		UPWAbilityModule_MultiActor* Module = NewObject<UPWAbilityModule_MultiActor>(Ability);
		Module->Initialize(Ability);
		Module->ActorClass = APWTestSpellActor::StaticClass();
		Module->ActorCount = Count;
		Module->Radius = 300.f;
		Module->PatternOrientation = EActorPatternOrientation::World;
		UPWSpawnPattern_Line* Line = NewObject<UPWSpawnPattern_Line>(Module);
		Line->Order = EPWLineOrder::CenterOut;
		Module->Pattern = Line;

		FPWTargetingResult Target;
		Target.Location = FVector(1000, 0, 0);
		Module->SpawnMultipleActors(Target);

		TArray<float> Offsets;
		for (const APWTestSpellActor* Actor : Test.GetActors<APWTestSpellActor>())
			Offsets.Add(FMath::RoundToFloat(Actor->GetActorLocation().Y));
		Offsets.Sort();

		if (Count == 4)
			TestEqual("Even count is evenly spaced around the center", Offsets, TArray<float>({-300.f, -100.f, 100.f, 300.f}));
		else
			TestEqual("Odd count is evenly spaced around the center", Offsets, TArray<float>({-300.f, -150.f, 0.f, 150.f, 300.f}));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMultiActorSpreadStepsTest, "PW.Modules.MultiActor.SpreadSteps", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMultiActorSpreadStepsTest::RunTest(const FString& Parameters)
{
	for (const bool bCenterOut : {false, true})
	{
		for (const int32 Count : {4, 5})
		{
			FPWTestWorld Test;
			UPWTestModularAbility* Ability = Test.ActivateAbility<UPWTestModularAbility>();

			UPWAbilityModule_MultiActor* Module = NewObject<UPWAbilityModule_MultiActor>(Ability);
			Module->Initialize(Ability);
			Module->ActorClass = APWTestSpellActor::StaticClass();
			Module->ActorCount = Count;
			Module->Radius = 300.f;
			Module->PatternOrientation = EActorPatternOrientation::World;
			UPWSpawnPattern_Rings* Rings = NewObject<UPWSpawnPattern_Rings>(Module);
			Rings->ActorsPerRing = 2;
			Rings->bCenterActor = Count % 2 == 1;
			Rings->bCenterOut = bCenterOut;
			Module->Pattern = Rings;

			FPWTargetingResult Target;
			Target.Location = FVector(0, 1000, 0);
			Module->SpawnMultipleActors(Target);

			TArray<float> Offsets;
			for (const APWTestSpellActor* Actor : Test.GetActors<APWTestSpellActor>())
				Offsets.Add(FMath::RoundToFloat(Actor->GetActorLocation().X));
			Offsets.Sort();

			if (Count == 4)
				TestEqual("Even count is evenly spaced around the center", Offsets, TArray<float>({-300.f, -100.f, 100.f, 300.f}));
			else
				TestEqual("Odd count is evenly spaced around the center", Offsets, TArray<float>({-300.f, -150.f, 0.f, 150.f, 300.f}));
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnPatternRingsTest, "PW.Modules.SpawnPattern.Rings", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSpawnPatternRingsTest::RunTest(const FString& Parameters)
{
	const auto CountAt = [](const TArray<FVector>& Positions, const float Distance)
	{
		return Positions.FilterByPredicate([Distance](const FVector& Position) { return FMath::IsNearlyEqual(static_cast<float>(Position.Size()), Distance, 0.5f); }).Num();
	};

	UPWSpawnPattern_Rings* Rings = NewObject<UPWSpawnPattern_Rings>();

	Rings->Distribution = EPWRingDistribution::Custom;
	Rings->RingCounts = {9, 6, 4};
	TArray<FVector> Positions = Rings->BuildPositions(20, 300.f);
	TestEqual("Custom total", Positions.Num(), 20);
	TestEqual("Custom outer ring", CountAt(Positions, 300.f), 9);
	TestEqual("Custom middle ring", CountAt(Positions, 200.f), 6);
	TestEqual("Custom inner ring", CountAt(Positions, 100.f), 4);
	TestEqual("Custom ends on the center", Positions.Last(), FVector::ZeroVector);

	Rings->RingCounts = {4};
	Positions = Rings->BuildPositions(10, 300.f);
	TestEqual("Left over actors go to the outer ring", CountAt(Positions, 300.f), 9);

	Rings->Distribution = EPWRingDistribution::ByRingSize;
	Rings->RingCount = 3;
	Positions = Rings->BuildPositions(20, 300.f);
	TestEqual("By size total", Positions.Num(), 20);
	TestEqual("By size outer ring", CountAt(Positions, 300.f), 10);
	TestEqual("By size middle ring", CountAt(Positions, 200.f), 6);
	TestEqual("By size inner ring", CountAt(Positions, 100.f), 3);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnPatternGridTest, "PW.Modules.SpawnPattern.Grid", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSpawnPatternGridTest::RunTest(const FString& Parameters)
{
	UPWSpawnPattern_Grid* Grid = NewObject<UPWSpawnPattern_Grid>();
	Grid->Rows = 2;

	TArray<FVector> Positions = Grid->BuildPositions(6, 100.f);
	TestEqual("Two rows of three", Positions.Num(), 6);
	TestEqual("Row by row starts on the back left", Positions[0], FVector(-50, -100, 0));
	TestEqual("Row by row finishes the first row", Positions[2], FVector(-50, 100, 0));

	Grid->bSnake = true;
	Positions = Grid->BuildPositions(6, 100.f);
	TestEqual("Snake turns around on the second row", Positions[3], FVector(50, 100, 0));
	Grid->bSnake = false;

	Grid->Order = EPWGridOrder::ColumnByColumn;
	Positions = Grid->BuildPositions(6, 100.f);
	TestEqual("Column by column goes up the first column", Positions[1], FVector(50, -100, 0));

	Grid->Order = EPWGridOrder::CenterOut;
	Positions = Grid->BuildPositions(6, 100.f);
	TestEqual("Center out starts in the middle column", static_cast<float>(Positions[0].Y), 0.f);
	TestEqual("Center out keeps the middle column first", static_cast<float>(Positions[1].Y), 0.f);

	Grid->Order = EPWGridOrder::EdgesIn;
	Positions = Grid->BuildPositions(6, 100.f);
	TestEqual("Edges in ends in the middle column", static_cast<float>(Positions.Last().Y), 0.f);

	Grid->Rows = 0;
	Grid->Order = EPWGridOrder::RowByRow;
	Positions = Grid->BuildPositions(5, 100.f);
	TestEqual("A short last row is centered", static_cast<float>(Positions[4].Y), 0.f);

	return true;
}

#endif
