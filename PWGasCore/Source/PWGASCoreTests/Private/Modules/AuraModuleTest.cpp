// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "Components/SphereComponent.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

struct FAuraOverlapParams
{
	UPrimitiveComponent* OverlappedComp = nullptr;
	AActor* OtherActor = nullptr;
	UPrimitiveComponent* OtherComp = nullptr;
	int32 OtherBodyIndex = 0;
	bool bFromSweep = false;
	FHitResult SweepResult;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAuraModuleTest, "PW.Modules.Aura.PeriodicEffect", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAuraModuleTest::RunTest(const FString& Parameters)
{
	FPWTestWorld Test;

	UPWAbilitySystemComponent* TargetASC = Test.SpawnASC();
	AActor* Target = TargetASC->GetOwner();
	UPWTestAttributeSet* Set = NewObject<UPWTestAttributeSet>(Target);
	TargetASC->AddAttributeSetSubobject(Set);
	TargetASC->SetNumericAttributeBase(UPWTestAttributeSet::GetHealthMaxAttribute(), 100.f);
	TargetASC->SetNumericAttributeBase(UPWTestAttributeSet::GetHealthAttribute(), 100.f);

	USphereComponent* Body = NewObject<USphereComponent>(Target);
	Body->SetCollisionProfileName(TEXT("Pawn"));
	Body->SetGenerateOverlapEvents(true);
	Body->RegisterComponent();
	Body->AttachToComponent(Target->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	const UPWTestAuraAbility* Ability = Test.ActivateAbility<UPWTestAuraAbility>();
	if (!TestNotNull("Aura ability", Ability))
		return false;
	UPWAbilityModule_Aura* Aura = Ability->GetAuraModule();
	if (!TestNotNull("Aura module", Aura))
		return false;

	// A headless world has no physics overlaps, so the events are sent by hand.
	FAuraOverlapParams Params;
	Params.OtherActor = Target;
	Aura->ProcessEvent(Aura->FindFunctionChecked("HandleBeginOverlap"), &Params);
	TestEqual("Effect is on the target", TargetASC->GetGameplayEffectCount(UPWTestPeriodicDamageEffect::StaticClass(), nullptr), 1);

	Aura->ProcessEvent(Aura->FindFunctionChecked("HandleBeginOverlap"), &Params);
	TestEqual("A second overlap of the same actor does not stack", TargetASC->GetGameplayEffectCount(UPWTestPeriodicDamageEffect::StaticClass(), nullptr), 1);
	// The first periodic execution is deferred to the next timer tick.
	Test.TickTimers(0.25f);
	Test.TickTimers(0.25f);
	const float AfterFirst = Set->GetHealth();
	TestTrue("Damage after the first tick", AfterFirst < 100.f);

	for (int32 i = 0; i < 6; ++i)
		Test.TickTimers(0.25f);
	TestTrue("Damage keeps ticking", Set->GetHealth() < AfterFirst);

	Aura->ProcessEvent(Aura->FindFunctionChecked("HandleEndOverlap"), &Params);
	TestEqual("Effect removed when leaving", TargetASC->GetGameplayEffectCount(UPWTestPeriodicDamageEffect::StaticClass(), nullptr), 0);
	return true;
}

#endif
