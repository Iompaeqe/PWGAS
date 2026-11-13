// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "GAS/Tags/GASCoreTags.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAbilityBaseCostTest, "PW.Abilities.Base.Cost", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAbilityBaseCostTest::RunTest(const FString& Parameters)
{
	TArray<FPWCostData> Costs;
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAbilityCancellationTagsTest, "PW.Abilities.Base.CancellationTags", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAbilityCancellationTagsTest::RunTest(const FString& Parameters)
{
	FPWTestWorld Test;
	const FGameplayTag Stun = PWTags::Ability::Activation::Failed;
	const FGameplayAbilitySpecHandle Handle = Test.ASC->GiveAbility(FGameplayAbilitySpec(UPWTestInterruptibleAbility::StaticClass(), 1));
	const auto IsActive = [&] { return Test.ASC->FindAbilitySpecFromHandle(Handle)->IsActive(); };
	const UObject* Instance = Test.ASC->FindAbilitySpecFromHandle(Handle)->GetPrimaryInstance();
	const auto IsListening = [&] { return Test.ASC->RegisterGameplayTagEvent(Stun, EGameplayTagEventType::NewOrRemoved).IsBoundToObject(Instance); };

	TestTrue("Activates", Test.ASC->TryActivateAbility(Handle));
	TestTrue("Listens while active", IsListening());

	Test.ASC->AddLooseGameplayTag(Stun);
	TestFalse("Gaining the tag cancels it", IsActive());
	TestFalse("No listener left after the cancel", IsListening());

	TestFalse("Cannot start while the tag is held", Test.ASC->TryActivateAbility(Handle));

	Test.ASC->RemoveLooseGameplayTag(Stun);
	TestTrue("Starts again once the tag is gone", Test.ASC->TryActivateAbility(Handle));

	Test.ASC->CancelAbilityHandle(Handle);
	TestFalse("No listener left after a normal end", IsListening());

	for (int32 i = 0; i < 3; ++i)
	{
		Test.ASC->TryActivateAbility(Handle);
		Test.ASC->CancelAbilityHandle(Handle);
	}
	Test.ASC->AddLooseGameplayTag(Stun);
	TestFalse("Repeated casts leave nothing behind", IsListening());
	return true;
}

#endif
