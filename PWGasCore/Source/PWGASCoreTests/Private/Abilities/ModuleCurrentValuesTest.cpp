// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "Ability/PWSpellParams.h"
#include "Engine/CurveTable.h"
#include "GAS/Abilities/Modules/ActionModules/PWAbilityModule_Projectile.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Radius.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Range.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace PWCurrentValuesTest
{
	// Value times the level, from a curve keyed 1, 2, 3.
	FScalableFloat ByLevel(UCurveTable* Table, const float Value)
	{
		FScalableFloat Out(Value);
		Out.Curve.CurveTable = Table;
		Out.Curve.RowName = TEXT("ByLevel");
		return Out;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FModuleCurrentValuesTest, "PW.Modules.CurrentValues", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FModuleCurrentValuesTest::RunTest(const FString& Parameters)
{
	using namespace PWCurrentValuesTest;

	const FPWTestWorld Test;
	const FGameplayAbilitySpecHandle Handle = Test.ASC->GiveAbility(FGameplayAbilitySpec(UPWTestModularAbility::StaticClass(), 1));
	UPWTestModularAbility* Ability = Cast<UPWTestModularAbility>(Test.ASC->FindAbilitySpecFromHandle(Handle)->GetPrimaryInstance());

	UCurveTable* Table = NewObject<UCurveTable>(GetTransientPackage());
	FRichCurve& Curve = Table->AddRichCurve(TEXT("ByLevel"));
	Curve.AddKey(1.f, 1.f);
	Curve.AddKey(2.f, 2.f);
	Curve.AddKey(3.f, 3.f);

	UPWAbilityModule_Radius* Radius = NewObject<UPWAbilityModule_Radius>(Ability);
	Radius->Radius = ByLevel(Table, 200.f);
	UPWAbilityModule_Range* Range = NewObject<UPWAbilityModule_Range>(Ability);
	Range->Range = FScalableFloat(1200.f);
	UPWAbilityModule_Projectile* Projectile = NewObject<UPWAbilityModule_Projectile>(Ability);
	Ability->DataModules.Add(Radius);
	Ability->DataModules.Add(Range);
	Ability->ActionModule = Projectile;
	Radius->Initialize(Ability);
	Range->Initialize(Ability);
	Projectile->Initialize(Ability);

	TestEqual("The configured radius at the ability's level", Radius->GetRadius(), 200.f);
	TestEqual("and at another level", Radius->GetRadiusAtLevel(3), 600.f);

	Radius->SetRadius(800.f);
	Range->SetRange(300.f);
	Projectile->SetProjectileSpeed(3000.f);

	float Read = 0.f;
	TestEqual("A set radius is the one in use", Radius->GetRadius(), 800.f);
	TestTrue("and the one policies read", Radius->GetValue(PWParamKeys::Radius, Read) && Read == 800.f);
	TestEqual("A plain set radius is the same at every level", Radius->GetRadiusAtLevel(3), 800.f);
	TestEqual("while the configured radius stays", Radius->GetBaseRadius(), 200.f);
	TestEqual("and still scales", Radius->GetBaseRadiusAtLevel(2), 400.f);

	Radius->SetRadiusCurve(ByLevel(Table, 300.f));
	TestEqual("A set curve scales with level", Radius->GetRadiusAtLevel(2), 600.f);
	TestEqual("from the ability's level", Radius->GetRadius(), 300.f);

	TestEqual("A set range is the one in use", Range->GetRange(), 300.f);
	TestEqual("while the configured range stays", Range->GetBaseRange(), 1200.f);

	TestEqual("A set projectile speed is the one in use", Projectile->GetProjectileSpeed(), 3000.f);
	TestEqual("and the one in its config", Projectile->GetProjectileConfig().InitialSpeed.GetValueAtLevel(1), 3000.f);
	TestEqual("while the configured speed stays", Projectile->GetBaseProjectileSpeed(), 1200.f);

	FPWProjectileSimConfig Floaty = Projectile->GetProjectileConfig();
	Floaty.GravityZOverride = -100.f;
	Projectile->SetProjectileConfig(Floaty);
	Projectile->SetProjectileSpeed(2000.f);
	TestEqual("Setting the speed keeps the rest of the config this cast set", Projectile->GetProjectileConfig().GravityZOverride, -100.f);

	if (!TestTrue("Activates", Test.ASC->TryActivateAbility(Handle)))
		return false;
	TestEqual("The next activation goes back to the configured radius", Radius->GetRadius(), 200.f);
	TestEqual("range", Range->GetRange(), 1200.f);
	TestEqual("projectile speed", Projectile->GetProjectileSpeed(), 1200.f);
	TestEqual("and the rest of the projectile config", Projectile->GetProjectileConfig().GravityZOverride, 0.f);
	return true;
}

#endif
