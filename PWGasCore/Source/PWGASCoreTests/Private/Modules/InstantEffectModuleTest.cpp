// copyright mkurtt96

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Targeting/Data/PWTargetingData.h"
#include "Targeting/PresetupPolicies/Targeting/SelfTargeting.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

struct FTargetDataReadyParams
{
	FGameplayAbilityTargetDataHandle DataHandle;
	FPWTargetingResult Result;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInstantEffectModuleTest, "PW.Modules.InstantEffect.Targets", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInstantEffectModuleTest::RunTest(const FString& Parameters)
{
	FPWTestWorld Test;
	UPWAbilitySystemComponent* TargetASC = Test.SpawnASC();

	const FGameplayAbilitySpecHandle Handle = Test.ASC->GiveAbility(FGameplayAbilitySpec(UPWTestInstantEffectAbility::StaticClass(), 1));
	TestTrue("Activated", Test.ASC->TryActivateAbility(Handle));

	const UPWModularGameplayAbility* Ability = Cast<UPWModularGameplayAbility>(Test.ASC->FindAbilitySpecFromHandle(Handle)->GetPrimaryInstance());
	UPWAbilityModule_InstantEffect* Module = Ability ? Ability->GetInstantEffectModule() : nullptr;
	if (!TestNotNull("Instant effect module", Module))
		return false;

	Module->ApplyEffects();
	TestEqual("Applied to self without targeting", Test.ASC->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 1);

	Module->RemoveEffects();
	TestEqual("Removed from self", Test.ASC->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 0);

	FTargetDataReadyParams Params;
	Params.Result.HitActors.Add(TargetASC->GetOwner());
	Module->ProcessEvent(Module->FindFunctionChecked("OnTargetDataReady"), &Params);
	TestEqual("Applied to target", TargetASC->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 1);

	Module->RemoveEffects();
	TestEqual("Removed from target", TargetASC->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInstantEffectAtLocationTest, "PW.Modules.InstantEffect.AtLocation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInstantEffectAtLocationTest::RunTest(const FString& Parameters)
{
	FPWTestWorld Test;
	UPWAbilitySystemComponent* TargetASC = Test.SpawnASC();
	UPWTestInstantEffectAbility* Ability = Test.ActivateAbility<UPWTestInstantEffectAbility>();
	UPWAbilityModule_InstantEffect* Module = Ability->GetInstantEffectModule();

	Module->ApplyEffectsAtLocation(FVector(100, 0, 0));
	TestEqual("Nothing without a targeting module", TargetASC->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 0);

	UPWTestTargetResolver* Resolver = NewObject<UPWTestTargetResolver>(Ability);
	Resolver->Actors.Add(TargetASC->GetOwner());
	UPWAbilityModule_Targeting* Targeting = NewObject<UPWAbilityModule_Targeting>(Ability);
	Targeting->TargetResolver = Resolver;
	Targeting->Initialize(Ability);
	Ability->ControlModules.Add(Targeting);

	Module->ApplyEffectsAtLocation(FVector(100, 0, 0));
	TestEqual("Applied at location", TargetASC->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 1);

	Module->ApplyEffectsAtSocket("NoSuchSocket");
	TestEqual("Applied at socket fallback", TargetASC->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 2);

	USelfTargeting* Source = NewObject<USelfTargeting>(Targeting);
	Source->Offset = FVector(50, 0, 0);
	Targeting->TargetingSource = Source;

	Module->ApplyEffectsAtLocation(FVector(100, 0, 0));
	TestEqual("Source offset added to the location", Resolver->LastLocation, FVector(150, 0, 0));

	Module->ApplyEffectsAtSocket("NoSuchSocket");
	TestEqual("Source offset added to the socket", Resolver->LastLocation, Test.Actor->GetActorLocation() + FVector(50, 0, 0));

	return true;
}

#endif
