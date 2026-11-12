// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestTypes.h"
#include "GAS/Abilities/AbilityDataAsset.h"
#include "GAS/Abilities/PWAbilityRegistry.h"
#include "GAS/Abilities/PWAbilitySet.h"
#include "GAS/Tags/GASCoreTags.h"
#include "Misc/AutomationTest.h"
#include "Misc/DataValidation.h"

#if WITH_DEV_AUTOMATION_TESTS

static UAbilityDataAsset* MakeAbilityData(const FGameplayTag& Tag, const TSubclassOf<UGameplayAbility> AbilityClass)
{
	UAbilityDataAsset* Data = NewObject<UAbilityDataAsset>();
	Data->Ability.Tag = Tag;
	Data->Ability.AbilityClass = AbilityClass;
	return Data;
}

static bool IsValidData(const UObject* Object)
{
	FDataValidationContext Context;
	return Object->IsDataValid(Context) == EDataValidationResult::Valid;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAbilityRegistryValidationTest, "PW.Abilities.Registry.Validation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAbilityRegistryValidationTest::RunTest(const FString& Parameters)
{
	const FGameplayTag Tag = PWTags::Ability::State::AwaitingConfirmation;
	const FGameplayTag OtherTag = PWTags::Ability::Activation::Failed;

	UPWTestModularAbility* AbilityDefault = GetMutableDefault<UPWTestModularAbility>();
	const FGameplayTag OriginalTag = AbilityDefault->AbilityTag;
	AbilityDefault->AbilityTag = Tag;

	UAbilityDataAsset* Good = MakeAbilityData(Tag, UPWTestModularAbility::StaticClass());
	TestTrue("Matching tag and class", IsValidData(Good));
	TestFalse("Tag of another ability", IsValidData(MakeAbilityData(OtherTag, UPWTestModularAbility::StaticClass())));
	TestFalse("Missing class", IsValidData(MakeAbilityData(Tag, nullptr)));
	TestFalse("Missing tag", IsValidData(MakeAbilityData(FGameplayTag(), UPWTestModularAbility::StaticClass())));

	UPWAbilityRegistry* Registry = NewObject<UPWAbilityRegistry>();
	Registry->Abilities.Add(Good);
	TestTrue("Registry with one valid entry", IsValidData(Registry));

	Registry->Abilities.Add(MakeAbilityData(Tag, UPWTestModularAbility::StaticClass()));
	TestFalse("Registry with a duplicate tag", IsValidData(Registry));

	FPWAbilitySetEntry Unregistered;
	Unregistered.AbilityTag = OtherTag;

	UPWAbilitySet* AbilitySet = NewObject<UPWAbilitySet>();
	AbilitySet->Abilities.Add(Unregistered);
	AbilitySet->Abilities.Add(FPWAbilitySetEntry());
	FDataValidationContext SetContext;
	AbilitySet->IsDataValid(SetContext);
	TestEqual("Set warns about the unregistered tag only", static_cast<int32>(SetContext.GetNumWarnings()), 1);

	AbilityDefault->AbilityTag = OriginalTag;
	return true;
}

#endif
