// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "GAS/Abilities/Derived/PWGameplayAbility_Passive.h"
#include "GAS/Tags/GASCoreTags.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPassiveAbilityTest, "PW.Abilities.Passive", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPassiveAbilityTest::RunTest(const FString& Parameters)
{
	FPWTestWorld Test;
	const FPWASC_AbilityLifecycle& Abilities = Test.ASC->Abilities();

	const FGameplayAbilitySpecHandle Handle = Test.ASC->GiveAbility(FGameplayAbilitySpec(UPWGameplayAbility_Passive::StaticClass(), 1));
	const FGameplayAbilitySpec* Spec = Test.ASC->FindAbilitySpecFromHandle(Handle);

	TestTrue("Activated on granted", Abilities.ApplyActivationPolicies(Handle, PWTags::Ability::Activation::OnGranted));
	TestTrue("Active", Spec->IsActive());

	TestTrue("Deactivated on removed", Abilities.ApplyDeactivationPolicies(Handle, PWTags::Ability::Deactivation::OnRemoved));
	TestFalse("Not active", Spec->IsActive());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPassiveAbilityEffectTest, "PW.Abilities.PassiveEffect", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPassiveAbilityEffectTest::RunTest(const FString& Parameters)
{
	FPWTestWorld Test;
	const FPWASC_AbilityLifecycle& Abilities = Test.ASC->Abilities();

	const FGameplayAbilitySpecHandle Handle = Test.ASC->GiveAbility(FGameplayAbilitySpec(UPWTestPassiveEffectAbility::StaticClass(), 1));

	Abilities.ApplyActivationPolicies(Handle, PWTags::Ability::Activation::OnGranted);
	TestEqual("Effect applied on activation", Test.ASC->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 1);

	Abilities.ApplyDeactivationPolicies(Handle, PWTags::Ability::Deactivation::OnRemoved);
	TestEqual("Effect removed on end", Test.ASC->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 0);

	UPWAbilitySystemComponent* Late = Test.SpawnASC();
	AActor* LateAvatar = Late->GetOwner();
	Late->InitAbilityActorInfo(LateAvatar, nullptr);
	const FGameplayAbilitySpecHandle LateHandle = Late->GiveAbility(FGameplayAbilitySpec(UPWTestPassiveEffectAbility::StaticClass(), 1));
	TestFalse("Cannot activate without an avatar", Late->Abilities().ApplyActivationPolicies(LateHandle, PWTags::Ability::Activation::OnGranted) && Late->FindAbilitySpecFromHandle(LateHandle)->IsActive());

	Late->InitAbilityActorInfo(LateAvatar, LateAvatar);
	TestTrue("Activates once the avatar is set", Late->FindAbilitySpecFromHandle(LateHandle)->IsActive());
	TestEqual("Effect applied once the avatar is set", Late->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 1);

	return true;
}

#endif
