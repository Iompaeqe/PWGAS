// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "GAS/Abilities/AbilityDataAsset.h"
#include "GAS/Abilities/PWAbilityRegistry.h"
#include "GAS/Abilities/PWAbilitySet.h"
#include "Settings/PWGASCoreSettings.h"
#include "GAS/Abilities/Derived/PWGameplayAbility_Channeling.h"
#include "GAS/Tags/GASCoreTags.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAbilityLifecycleInvalidInputTest, "PW.ASC.AbilityLifecycle.InvalidInput", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAbilityLifecycleInvalidInputTest::RunTest(const FString& Parameters)
{
	FPWTestWorld Test;
	const FPWASC_AbilityLifecycle& Abilities = Test.ASC->Abilities();

	Abilities.AddAbility(PWTags::Ability::State::AwaitingConfirmation);
	TestEqual("Unknown tag grants nothing", Test.ASC->GetActivatableAbilities().Num(), 0);

	FPWAbilitySetEntry Unknown;
	Unknown.AbilityTag = PWTags::Ability::State::AwaitingConfirmation;
	Unknown.InputTag = PWTags::Input::GlobalCancel;

	UPWAbilitySet* AbilitySet = NewObject<UPWAbilitySet>();
	AbilitySet->Abilities.Add(Unknown);
	Abilities.AddAbilitySet(AbilitySet);
	Abilities.AddAbilitySet(nullptr);
	TestEqual("Unknown set entry grants nothing", Test.ASC->GetActivatableAbilities().Num(), 0);

	const FGameplayAbilitySpecHandle Handle = Test.ASC->GiveAbility(FGameplayAbilitySpec(UPWTestPlainAbility::StaticClass(), 1));
	Test.ASC->ClearAbility(Handle);

	Abilities.RemoveGrantedAbility(Handle);
	TestFalse("Deactivation on stale handle", Abilities.ApplyDeactivationPolicies(Handle, PWTags::Ability::Deactivation::OnRemoved));
	TestFalse("Tag check on stale handle", Test.ASC->Data().DoesAbilityHaveTag(Handle, PWTags::Ability::Skill::Root));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAbilityLifecycleForeignAbilityTest, "PW.ASC.AbilityLifecycle.ForeignAbility", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAbilityLifecycleForeignAbilityTest::RunTest(const FString& Parameters)
{
	FPWTestWorld Test;

	const FGameplayAbilitySpecHandle Plain = Test.ASC->GiveAbility(FGameplayAbilitySpec(UPWTestPlainAbility::StaticClass(), 1));
	const FGameplayAbilitySpecHandle Exclusive = Test.ASC->GiveAbility(FGameplayAbilitySpec(UPWGameplayAbility_Channeling::StaticClass(), 1));

	TestTrue("Plain ability activates", Test.ASC->TryActivateAbility(Plain));
	TestTrue("Exclusive ability activates beside it", Test.ASC->TryActivateAbility(Exclusive));
	TestTrue("Plain ability still active", Test.ASC->FindAbilitySpecFromHandle(Plain)->IsActive());

	Test.ASC->CancelAbilityHandle(Plain);
	TestFalse("Plain ability ended", Test.ASC->FindAbilitySpecFromHandle(Plain)->IsActive());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAbilityLifecycleLevelsTest, "PW.ASC.AbilityLifecycle.Levels", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAbilityLifecycleLevelsTest::RunTest(const FString& Parameters)
{
	const FGameplayTag Tag = PWTags::Ability::Skill::Root;

	UAbilityDataAsset* Data = NewObject<UAbilityDataAsset>();
	Data->Ability.Tag = Tag;
	Data->Ability.AbilityClass = UPWTestLeveledAbility::StaticClass();
	Data->Ability.MaxLevel = 3;
	UPWAbilityRegistry* Registry = NewObject<UPWAbilityRegistry>();
	Registry->Abilities.Add(Data);

	UPWGASCoreSettings* Settings = GetMutableDefault<UPWGASCoreSettings>();
	TSoftObjectPtr<UPWAbilityRegistry>* RegistrySlot = Settings->GetClass()->FindPropertyByName("AbilityRegistry")->ContainerPtrToValuePtr<TSoftObjectPtr<UPWAbilityRegistry>>(Settings);
	const TSoftObjectPtr<UPWAbilityRegistry> Original = *RegistrySlot;
	*RegistrySlot = Registry;

	{
		FPWTestWorld Test;
		FPWASC_AbilityLifecycle& Abilities = Test.ASC->Abilities();

		Abilities.AddAbility(Tag);
		Abilities.AddAbility(Tag);
		TestEqual("Granting twice keeps one copy", Test.ASC->GetActivatableAbilities().Num(), 1);
		TestEqual("Starts at level 1", Abilities.GetAbilityLevel(Tag), 1);

		TestFalse("Cannot go below level 1", Abilities.DowngradeAbility(Tag));
		TestEqual("Still level 1", Abilities.GetAbilityLevel(Tag), 1);

		TestTrue("Upgrade to 2", Abilities.UpgradeAbility(Tag));
		TestTrue("Upgrade to 3", Abilities.UpgradeAbility(Tag));
		TestTrue("At max level", Abilities.IsAbilityMaxLevel(Tag));
		TestFalse("Cannot pass max level", Abilities.UpgradeAbility(Tag));
		TestEqual("Stays at max level", Abilities.GetAbilityLevel(Tag), 3);

		TestTrue("Downgrade to 2", Abilities.DowngradeAbility(Tag));
		TestEqual("Level 2", Abilities.GetAbilityLevel(Tag), 2);

		TestFalse("Upgrading an ability the owner lacks fails", Abilities.UpgradeAbility(PWTags::Ability::Activation::Failed));
	}

	*RegistrySlot = Original;
	return true;
}

#endif
