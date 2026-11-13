// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestWorld.h"
#include "GAS/EffectContext/PWEffectContextFunctions.h"
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
	TestNotNull("The component makes a PW context", UPWEffectContextFunctions::GetPWContext(Context));
	TestTrue("Instigator is kept", Context.GetInstigator() == Test.Actor);

	FHitResult Out;
	TestFalse("No hit stored yet", UPWEffectContextFunctions::GetHitResult(Context, Out));
	UPWEffectContextFunctions::CopyHitResultToContext(Context, Hit);
	TestTrue("Stored hit is returned", UPWEffectContextFunctions::GetHitResult(Context, Out));
	TestEqual("Hit location round trips", Out.Location, Hit.Location);

	const FGameplayEffectContextHandle Copy = Context.Duplicate();
	TestTrue("A duplicate keeps the hit", UPWEffectContextFunctions::GetHitResult(Copy, Out));

	FGameplayEffectContextHandle Plain(new FGameplayEffectContext());
	TestNull("A plain context is not treated as a PW context", UPWEffectContextFunctions::GetPWContext(Plain));
	UPWEffectContextFunctions::CopyHitResultToContext(Plain, Hit);
	TestFalse("A plain context holds no PW hit", UPWEffectContextFunctions::GetHitResult(Plain, Out));

	const FGameplayEffectContextHandle Empty;
	TestFalse("An empty handle is handled", UPWEffectContextFunctions::GetHitResult(Empty, Out));
	return true;
}

#endif
