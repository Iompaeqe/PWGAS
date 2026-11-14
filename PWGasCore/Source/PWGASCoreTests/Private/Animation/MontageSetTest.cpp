// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestWorld.h"
#include "Animation/AnimMontage.h"
#include "GAS/Tags/GASCoreTags.h"
#include "VisualAndAudio/PWMontageSet.h"
#include "VisualAndAudio/PWMontageSetComponent.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMontageSetComponentTest, "PW.Animation.MontageSet.Component", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMontageSetComponentTest::RunTest(const FString& Parameters)
{
	const FPWTestWorld Test;
	TestNull("No provider yet", IPWMontageProvider::Find(Test.Actor));

	UAnimMontage* CastMontage = NewObject<UAnimMontage>();
	UPWMontageSet* Set = NewObject<UPWMontageSet>();
	Set->Montages.Add(PWTags::MontageEvent::Trigger, {CastMontage, 1.5f});

	UPWMontageSetComponent* Component = NewObject<UPWMontageSetComponent>(Test.Actor);
	Component->RegisterComponent();
	Component->SetMontageSet(Set);

	const UObject* Provider = IPWMontageProvider::Find(Test.Actor);
	TestEqual("The component answers for the pawn", Provider, static_cast<const UObject*>(Component));

	float Rate = 0.f;
	TestEqual("Montage for the tag", IPWMontageProvider::Execute_GetMontageForTag(Provider, PWTags::MontageEvent::Trigger, Rate), CastMontage);
	TestEqual("Play rate from the set", Rate, 1.5f);

	TestNull("Unknown tag", IPWMontageProvider::Execute_GetMontageForTag(Provider, PWTags::Event::Input, Rate));
	TestEqual("Unknown tag plays at normal rate", Rate, 1.f);

	Component->SetMontageSet(nullptr);
	TestNull("No set, no montage", IPWMontageProvider::Execute_GetMontageForTag(Provider, PWTags::MontageEvent::Trigger, Rate));
	return true;
}

#endif
