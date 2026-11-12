// copyright mkurtt96

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "PWAttributeFunctions.h"
#include "GAS/Tags/GASCoreTags.h"
#include "Misc/AutomationTest.h"
#include "Net/UnrealNetwork.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAttributeSetTagsTest, "PW.Attributes.Base.Tags", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttributeSetTagsTest::RunTest(const FString& Parameters)
{
	const FPWTestWorld Test;
	UPWTestAttributeSet* Set = NewObject<UPWTestAttributeSet>(Test.Actor);
	Test.ASC->AddAttributeSetSubobject(Set);

	// The set looks under Ability, where Ability.SetByCaller.Cooldown matches the Cooldown attribute.
	const FGameplayTag Cooldown = PWTags::Ability::SetByCaller::Cooldown;

	TestTrue("Tag found by attribute name", Set->GetAttribute(Cooldown) == UPWTestAttributeSet::GetCooldownAttribute());
	TestTrue("Attribute to tag", Set->GetAttributeTag(UPWTestAttributeSet::GetCooldownAttribute()) == Cooldown);
	TestFalse("An attribute without a tag gives no tag", Set->GetAttributeTag(UPWTestAttributeSet::GetHealthAttribute()).IsValid());
	TestEqual("Only tagged attributes are mapped", Set->GetTagMap().Num(), 1);

	Set->SetAttributeValue(Cooldown, 40.f);
	TestEqual("Set and get by tag", Set->GetAttributeValue(Cooldown), 40.f);
	TestEqual("Library finds it through the component", UPWAttributeFunctions::GetAttributeValue(Test.ASC, Cooldown), 40.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAttributeSetReplicationTest, "PW.Attributes.Base.Replication", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttributeSetReplicationTest::RunTest(const FString& Parameters)
{
	UPWTestAttributeSet::StaticClass()->SetUpRuntimeReplicationData();

	TArray<FLifetimeProperty> Props;
	GetDefault<UPWTestAttributeSet>()->GetLifetimeReplicatedProps(Props);

	TestEqual("Replicated attributes are registered, local ones are not", Props.Num(), 2);
	for (const FLifetimeProperty& Prop : Props)
	{
		TestEqual("Always notifies", Prop.RepNotifyCondition, REPNOTIFY_Always);
		TestEqual("No condition", Prop.Condition, COND_None);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAttributeSetClampTest, "PW.Attributes.Base.Clamp", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttributeSetClampTest::RunTest(const FString& Parameters)
{
	const FPWTestWorld Test;
	UPWTestAttributeSet* Set = NewObject<UPWTestAttributeSet>(Test.Actor);
	Test.ASC->AddAttributeSetSubobject(Set);

	const FGameplayAttribute Health = UPWTestAttributeSet::GetHealthAttribute();
	const FGameplayAttribute HealthMax = UPWTestAttributeSet::GetHealthMaxAttribute();

	Test.ASC->SetNumericAttributeBase(HealthMax, 100.f);
	Test.ASC->SetNumericAttributeBase(Health, 150.f);
	TestEqual("Clamped to max", Set->GetHealth(), 100.f);
	TestEqual("Base is clamped too", Test.ASC->GetNumericAttributeBase(Health), 100.f);

	Test.ASC->SetNumericAttributeBase(Health, -5.f);
	TestEqual("Clamped to min", Set->GetHealth(), 0.f);

	Test.ASC->SetNumericAttributeBase(Health, 80.f);
	Test.ASC->ApplyModToAttribute(Health, EGameplayModOp::Additive, 50.f);
	TestEqual("A heal past max stops at max", Test.ASC->GetNumericAttributeBase(Health), 100.f);

	Test.ASC->SetNumericAttributeBase(HealthMax, 60.f);
	TestEqual("Lowering max pulls the value down", Set->GetHealth(), 60.f);

	Test.ASC->SetNumericAttributeBase(HealthMax, 200.f);
	TestEqual("Raising max leaves the value", Set->GetHealth(), 60.f);

	Test.ASC->SetNumericAttributeBase(UPWTestAttributeSet::GetCooldownAttribute(), 999.f);
	TestEqual("Other attributes are not clamped", Set->GetCooldown(), 999.f);
	return true;
}

#endif
