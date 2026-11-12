// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestWorld.h"
#include "GAS/EffectContext/EffectContextFunctions.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEffectContextTest, "PW.ASC.EffectContext", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEffectContextTest::RunTest(const FString& Parameters)
{
	const FPWTestWorld Test;

	FHitResult Hit;
	Hit.Location = FVector(10.f, 20.f, 30.f);
	Hit.bBlockingHit = true;

	FGameplayEffectContextHandle Context = Test.ASC->MakeEffectContext();
	TestNotNull("The component makes a PW context", UEffectContextFunctions::GetPWContext(Context));
	TestTrue("Instigator is kept", Context.GetInstigator() == Test.Actor);

	FHitResult Out;
	TestFalse("No hit stored yet", UEffectContextFunctions::GetHitResult(Context, Out));
	UEffectContextFunctions::CopyHitResultToContext(Context, Hit);
	TestTrue("Stored hit is returned", UEffectContextFunctions::GetHitResult(Context, Out));
	TestEqual("Hit location round trips", Out.Location, Hit.Location);

	const FGameplayEffectContextHandle Copy = Context.Duplicate();
	TestTrue("A duplicate keeps the hit", UEffectContextFunctions::GetHitResult(Copy, Out));

	FGameplayEffectContextHandle Plain(new FGameplayEffectContext());
	TestNull("A plain context is not treated as a PW context", UEffectContextFunctions::GetPWContext(Plain));
	UEffectContextFunctions::CopyHitResultToContext(Plain, Hit);
	TestFalse("A plain context holds no PW hit", UEffectContextFunctions::GetHitResult(Plain, Out));

	const FGameplayEffectContextHandle Empty;
	TestFalse("An empty handle is handled", UEffectContextFunctions::GetHitResult(Empty, Out));
	return true;
}

#endif
