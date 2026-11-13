// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Targeting/Data/PWTargetingData.h"
#include "Targeting/PresetupPolicies/Targeting/PWTargeting_Self.h"
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

	Module->ProcessEvent(Module->FindFunctionChecked("OnTargetDataReady"), &Params);
	TargetASC->GetOwner()->Destroy();
	Params.Result.HitActors.Reset();
	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	Module->RemoveEffects();
	TestEqual("A destroyed target is skipped", Test.ASC->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 0);

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
	UPWAbilityModule_Resolver* ResolverModule = NewObject<UPWAbilityModule_Resolver>(Ability);
	ResolverModule->Resolver = Resolver;
	ResolverModule->Initialize(Ability);
	Ability->ControlModules.Add(ResolverModule);

	Module->ApplyEffectsAtLocation(FVector(100, 0, 0));
	TestEqual("A resolver alone is enough, no targeting module", TargetASC->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 1);
	Module->RemoveEffects();

	Module->ApplyEffects();
	TestEqual("Apply Effects with a resolver alone gathers around the caster", TargetASC->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 1);
	TestEqual("The caster was the resolve point", Resolver->LastLocation, Test.Actor->GetActorLocation());
	Module->RemoveEffects();

	UPWAbilityModule_Radius* RadiusModule = NewObject<UPWAbilityModule_Radius>(Ability);
	RadiusModule->Radius = 300.f;
	RadiusModule->Initialize(Ability);
	Ability->DataModules.Add(RadiusModule);

	FPWTargetingResult AtCaster = ResolverModule->ResolveAroundCaster();
	TestEqual("Inside the ability a policy reads the value from its data module", Resolver->LastRadius, 300.f);
	TestEqual("A key no module owns reads as zero", Resolver->GetFloat("Nothing"), 0.f);

	USpellParams* Params = Ability->MakeSpellParams();
	RadiusModule->ExtendParams(Params);
	ResolverModule->ExtendParams(Params);
	RadiusModule->Radius = 900.f;

	const IPWSpellTargetProvider* Provider = Cast<IPWSpellTargetProvider>(Params->TargetResolver);
	TArray<AActor*> ImpactTargets;
	AActor* SpellActor = TargetASC->GetOwner();
	if (TestNotNull("The resolver travels in the spell params", Provider))
		Provider->GetImpactTargets(SpellActor, Params, FHitResult(), ImpactTargets);
	TestEqual("A spawned actor gets the resolver's targets", ImpactTargets.Num(), 1);
	TestEqual("It resolved where the spawned actor is", Resolver->LastLocation, SpellActor->GetActorLocation());
	TestEqual("On a spawned actor the value comes from the spell params", Resolver->LastRadius, 300.f);
	Ability->DataModules.Remove(RadiusModule);

	UPWAbilityModule_Targeting* Targeting = NewObject<UPWAbilityModule_Targeting>(Ability);
	Targeting->Initialize(Ability);
	Ability->ControlModules.Add(Targeting);

	Module->ApplyEffectsAtLocation(FVector(100, 0, 0));
	TestEqual("Applied at location", TargetASC->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 1);

	Module->ApplyEffectsAtSocket("NoSuchSocket");
	TestEqual("Applied at socket fallback", TargetASC->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 2);

	FPWTargetingResult AroundCaster;
	TestTrue("A resolver with no targeting source resolves around the caster", Targeting->ComputeTarget(AroundCaster));
	TestEqual("The caster is the target point", AroundCaster.Location, Test.Actor->GetActorLocation());
	TestEqual("The resolver's targets are returned", AroundCaster.HitActors.Num(), 1);

	Resolver->Actors.Add(TargetASC->GetOwner());
	Module->ApplyEffectsAtLocation(FVector(100, 0, 0));
	TestEqual("A target resolved twice is hit once", TargetASC->GetGameplayEffectCount(UPWTestEffect::StaticClass(), nullptr), 3);

	UPWTargeting_Self* Source = NewObject<UPWTargeting_Self>(Targeting);
	Source->Offset = FVector(50, 0, 0);
	Targeting->TargetingSource = Source;

	Module->ApplyEffectsAtLocation(FVector(100, 0, 0));
	TestEqual("Source offset added to the location", Resolver->LastLocation, FVector(150, 0, 0));

	Module->ApplyEffectsAtSocket("NoSuchSocket");
	TestEqual("Source offset added to the socket", Resolver->LastLocation, Test.Actor->GetActorLocation() + FVector(50, 0, 0));

	return true;
}

#endif
