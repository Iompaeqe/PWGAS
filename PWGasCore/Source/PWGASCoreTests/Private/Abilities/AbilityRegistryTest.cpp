// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "GameFramework/GameStateBase.h"
#include "GAS/Abilities/PWAbilityDataAsset.h"
#include "GAS/Abilities/PWAbilityRegistry.h"
#include "GAS/Abilities/PWAbilitySet.h"
#include "GAS/Tags/GASCoreTags.h"
#include "Settings/PWGASCoreSettings.h"
#include "Misc/AutomationTest.h"
#include "Misc/DataValidation.h"

#if WITH_DEV_AUTOMATION_TESTS

// Swaps the registry in Project Settings for as long as it lives.
struct FPWScopedProjectRegistry
{
	TSoftObjectPtr<UPWAbilityRegistry>* Slot = nullptr;
	TSoftObjectPtr<UPWAbilityRegistry> Original;

	explicit FPWScopedProjectRegistry(UPWAbilityRegistry* Registry)
	{
		UPWGASCoreSettings* Settings = GetMutableDefault<UPWGASCoreSettings>();
		Slot = Settings->GetClass()->FindPropertyByName("AbilityRegistry")->ContainerPtrToValuePtr<TSoftObjectPtr<UPWAbilityRegistry>>(Settings);
		Original = *Slot;
		*Slot = Registry;
	}

	~FPWScopedProjectRegistry() { *Slot = Original; }
};

static UPWAbilityDataAsset* MakeAbilityData(const FGameplayTag& Tag, const TSubclassOf<UGameplayAbility> AbilityClass)
{
	UPWAbilityDataAsset* Data = NewObject<UPWAbilityDataAsset>();
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

	UPWAbilityDataAsset* Good = MakeAbilityData(Tag, UPWTestModularAbility::StaticClass());
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAbilityRegistryLookupTest, "PW.Abilities.Registry.Lookup", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAbilityRegistryLookupTest::RunTest(const FString& Parameters)
{
	const FGameplayTag Tag = PWTags::Ability::State::AwaitingConfirmation;
	const FGameplayTag OtherTag = PWTags::Ability::Activation::Failed;

	UPWAbilityDataAsset* Data = MakeAbilityData(Tag, UPWTestModularAbility::StaticClass());
	UPWAbilityRegistry* Registry = NewObject<UPWAbilityRegistry>();
	Registry->Abilities.Add(Data);

	TestTrue("Finds a listed ability", Registry->GetAbilityByTag(Tag).AbilityClass == UPWTestModularAbility::StaticClass());
	TestFalse("An unlisted tag", Registry->GetAbilityByTag(OtherTag).IsValid());

	// The first lookup filled the cache; an ability renamed in the editor afterwards is still found by its new tag.
	Data->Ability.Tag = OtherTag;
	TestTrue("The renamed ability under its new tag", Registry->GetAbilityByTag(OtherTag).IsValid());
	TestFalse("and not under the old one", Registry->GetAbilityByTag(Tag).IsValid());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAbilityRegistryWorldTest, "PW.Abilities.Registry.World", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAbilityRegistryWorldTest::RunTest(const FString& Parameters)
{
	const FGameplayTag Tag = PWTags::Ability::State::AwaitingConfirmation;

	UPWAbilityRegistry* ProjectRegistry = NewObject<UPWAbilityRegistry>();
	UPWAbilityRegistry* WorldRegistry = NewObject<UPWAbilityRegistry>();
	WorldRegistry->Abilities.Add(MakeAbilityData(Tag, UPWTestModularAbility::StaticClass()));
	const FPWScopedProjectRegistry Scoped(ProjectRegistry);

	const FPWTestWorld Test;
	TestTrue("Without a game mode registry the project's is used", UPWAbilityRegistry::Get(Test.ASC) == ProjectRegistry);
	TestFalse("which doesn't list the ability", UPWAbilityRegistry::FindAbility(Test.ASC, Tag).IsValid());

	// As on a client: no game mode, only the game state carrying its class.
	APWTestRegistryGameMode* GameModeDefaults = GetMutableDefault<APWTestRegistryGameMode>();
	GameModeDefaults->Registry = WorldRegistry;
	AGameStateBase* GameState = Test.World->SpawnActor<AGameStateBase>();
	GameState->GameModeClass = APWTestRegistryGameMode::StaticClass();
	Test.World->SetGameState(GameState);

	TestTrue("The game mode's registry is used", UPWAbilityRegistry::Get(Test.ASC) == WorldRegistry);
	TestTrue("and its abilities are found", UPWAbilityRegistry::FindAbility(Test.ASC, Tag).IsValid());

	GameModeDefaults->Registry = nullptr;
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAbilityRegistryOwnedAbilityTest, "PW.Abilities.Registry.OwnedAbility", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAbilityRegistryOwnedAbilityTest::RunTest(const FString& Parameters)
{
	const FGameplayTag Tag = PWTags::Ability::Skill::Root;

	UPWAbilityDataAsset* Data = MakeAbilityData(Tag, UPWTestLeveledAbility::StaticClass());
	Data->Ability.MaxLevel = 3;
	UPWAbilityRegistry* Registry = NewObject<UPWAbilityRegistry>();
	Registry->Abilities.Add(Data);
	const FPWScopedProjectRegistry Scoped(Registry);

	const FPWTestWorld Test;
	FPWOwnedAbility Owned = Test.ASC->GetOwnedAbility(Tag);
	TestFalse("Not owned before it is granted", Owned.bOwned);
	TestEqual("but the definition is there", Owned.Info.MaxLevel, 3);

	Test.ASC->Abilities().AddAbility(Tag, PWTags::Input::Root);
	Test.ASC->Abilities().UpgradeAbility(Tag);
	Owned = Test.ASC->GetOwnedAbility(Tag);
	TestTrue("Owned once granted", Owned.bOwned);
	TestEqual("The owner's level", Owned.Level, 2);
	TestTrue("The owner's input", Owned.InputTag == PWTags::Input::Root);
	return true;
}

#endif
