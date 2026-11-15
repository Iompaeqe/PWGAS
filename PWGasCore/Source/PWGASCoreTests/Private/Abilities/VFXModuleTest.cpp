// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "GAS/Abilities/Modules/ControlModules/PWAbilityModule_VFX.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Radius.h"
#include "GAS/Abilities/Values/PWAbilityValue.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFXModuleParametersTest, "PW.Modules.VFX.Parameters", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FVFXModuleParametersTest::RunTest(const FString& Parameters)
{
	const FPWTestWorld Test;
	const FGameplayAbilitySpecHandle Handle = Test.ASC->GiveAbility(FGameplayAbilitySpec(UPWTestModularAbility::StaticClass(), 1));
	UPWTestModularAbility* Ability = Cast<UPWTestModularAbility>(Test.ASC->FindAbilitySpecFromHandle(Handle)->GetPrimaryInstance());

	UPWAbilityModule_Radius* Radius = NewObject<UPWAbilityModule_Radius>(Ability);
	Radius->Radius = FScalableFloat(700.f);
	Ability->DataModules.Add(Radius);
	UPWAbilityModule_VFX* VFX = NewObject<UPWAbilityModule_VFX>(Ability);
	Ability->ControlModules.Add(VFX);
	Radius->Initialize(Ability);
	VFX->Initialize(Ability);

	FPWVFXEntry Entry;
	const auto AddParameter = [&Entry](const TCHAR* Name, UPWAbilityValue* Value)
	{
		FPWVFXParameter& Parameter = Entry.Parameters.AddDefaulted_GetRef();
		Parameter.Name = Name;
		Parameter.Value = Value;
	};

	UPWAbilityValue_Constant* Constant = NewObject<UPWAbilityValue_Constant>(VFX);
	Constant->Value = 5.f;
	UPWAbilityValue_Subtract* Fitted = NewObject<UPWAbilityValue_Subtract>(VFX);
	TestEqual("A new Subtract starts at 0 - 0", Fitted->Evaluate(Ability), 0.f);
	Fitted->A = NewObject<UPWAbilityValue_RadiusModule>(Fitted);
	Cast<UPWAbilityValue_Constant>(Fitted->B)->Value = 65.f;
	UPWAbilityValue_Multiply* Doubled = NewObject<UPWAbilityValue_Multiply>(VFX);
	TestEqual("A new Multiply starts at 1 * 1", Doubled->Evaluate(Ability), 1.f);
	Doubled->A = Fitted;
	Cast<UPWAbilityValue_Constant>(Doubled->B)->Value = 2.f;
	UPWAbilityValue_Divide* Halved = NewObject<UPWAbilityValue_Divide>(VFX);
	Halved->A = NewObject<UPWAbilityValue_RadiusModule>(Halved);
	Cast<UPWAbilityValue_Constant>(Halved->B)->Value = 2.f;
	UPWAbilityValue_Divide* ByZero = NewObject<UPWAbilityValue_Divide>(VFX);
	Cast<UPWAbilityValue_Constant>(ByZero->B)->Value = 0.f;
	UPWAbilityValue_Variable* Kills = NewObject<UPWAbilityValue_Variable>(VFX);
	Kills->Variable = GET_MEMBER_NAME_CHECKED(UPWTestModularAbility, KillCount);
	UPWAbilityValue_Variable* Missing = NewObject<UPWAbilityValue_Variable>(VFX);
	Missing->Variable = TEXT("NoSuchVariable");

	AddParameter(TEXT("User.Radius"), NewObject<UPWAbilityValue_RadiusModule>(VFX));
	AddParameter(TEXT("User.Level"), NewObject<UPWAbilityValue_AbilityLevel>(VFX));
	AddParameter(TEXT("User.Fixed"), Constant);
	AddParameter(TEXT("User.Reach"), NewObject<UPWAbilityValue_RangeModule>(VFX));
	AddParameter(TEXT("User.Fitted"), Fitted);
	AddParameter(TEXT("User.Doubled"), Doubled);
	AddParameter(TEXT("User.Kills"), Kills);
	AddParameter(TEXT("User.Missing"), Missing);
	AddParameter(TEXT("User.Unset"), nullptr);
	VFX->Entries.Add(Entry);

	Ability->KillCount = 3;
	TestEqual("Radius Module is the radius in use", VFX->GetParameterValue(0, 0), 700.f);
	TestEqual("Ability Level is the level", VFX->GetParameterValue(0, 1), 1.f);
	TestEqual("Constant is its value", VFX->GetParameterValue(0, 2), 5.f);
	TestEqual("Range Module without a Range module is 0", VFX->GetParameterValue(0, 3), 0.f);
	TestEqual("Subtract takes B from A", VFX->GetParameterValue(0, 4), 635.f);
	TestEqual("Math nests", VFX->GetParameterValue(0, 5), 1270.f);
	TestEqual("Divide divides", Halved->Evaluate(Ability), 350.f);
	TestEqual("and dividing by 0 gives 0", ByZero->Evaluate(Ability), 0.f);
	TestEqual("Blueprint Variable reads the running ability", VFX->GetParameterValue(0, 6), 3.f);
	TestEqual("an unknown variable is 0", VFX->GetParameterValue(0, 7), 0.f);
	TestEqual("and an unset value is 0", VFX->GetParameterValue(0, 8), 0.f);
	TestEqual("Debug shows the math", Doubled->Describe(Ability), FString(TEXT("((Radius Module 700 - 65) * 2)")));

	Radius->SetRadius(900.f);
	Ability->KillCount = 4;
	TestEqual("A changed radius shows in the next value", VFX->GetParameterValue(0, 4), 835.f);
	TestEqual("and a changed variable", VFX->GetParameterValue(0, 6), 4.f);
	return true;
}

#endif
