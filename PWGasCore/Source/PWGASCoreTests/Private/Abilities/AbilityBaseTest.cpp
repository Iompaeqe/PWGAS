// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestTypes.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAbilityBaseCostTest, "PW.Abilities.Base.Cost", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAbilityBaseCostTest::RunTest(const FString& Parameters)
{
	TArray<FCostData> Costs;
	GetDefault<UPWTestCostAbility>()->GetCost(1, Costs);
	if (!TestEqual("Cost count", Costs.Num(), 1))
		return false;
	TestEqual("Cost value", Costs[0].Value, -25.f);

	Costs.Reset();
	GetDefault<UPWTestModularAbility>()->GetCost(1, Costs);
	TestEqual("No cost effect", Costs.Num(), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FModularAbilityExtractLocationTest, "PW.Abilities.Modular.ExtractLocation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FModularAbilityExtractLocationTest::RunTest(const FString& Parameters)
{
	const UPWModularGameplayAbility* Ability = GetDefault<UPWTestModularAbility>();

	FHitResult Hit;
	Hit.bBlockingHit = true;
	Hit.ImpactPoint = FVector(100, 200, 300);
	const FGameplayAbilityTargetData_SingleTargetHit HitData(Hit);
	TestEqual("Hit result", Ability->ExtractLocation(&HitData), FVector(100, 200, 300));

	FGameplayAbilityTargetData_LocationInfo LocationData;
	LocationData.TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
	LocationData.TargetLocation.LiteralTransform = FTransform(FVector(400, 500, 600));
	TestEqual("Location info", Ability->ExtractLocation(&LocationData), FVector(400, 500, 600));

	TestEqual("Null data", Ability->ExtractLocation(nullptr), FVector::ZeroVector);

	return true;
}

#endif
