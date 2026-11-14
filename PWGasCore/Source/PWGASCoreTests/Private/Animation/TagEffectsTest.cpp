// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSocket.h"
#include "GAS/Tags/GASCoreTags.h"
#include "Libraries/PWGasFunctions.h"
#include "Materials/MaterialInstanceConstant.h"
#include "VisualAndAudio/PWTagEffectSet.h"
#include "VisualAndAudio/PWTagEffectsComponent.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStaticMeshSocketTest, "PW.Animation.Sockets.StaticMesh", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStaticMeshSocketTest::RunTest(const FString& Parameters)
{
	const FPWTestWorld Test;

	UStaticMesh* StaticMesh = NewObject<UStaticMesh>();
	UStaticMeshSocket* Socket = NewObject<UStaticMeshSocket>(StaticMesh);
	Socket->SocketName = TEXT("RightFist");
	StaticMesh->Sockets.Add(Socket);

	UStaticMeshComponent* Body = NewObject<UStaticMeshComponent>(Test.Actor);
	Body->SetStaticMesh(StaticMesh);

	TestEqual("A static mesh's socket is found", UPWGasFunctions::FindSocketMesh(Test.Actor, TEXT("RightFist")), static_cast<UMeshComponent*>(Body));
	TestNull("A socket no mesh has", UPWGasFunctions::FindSocketMesh(Test.Actor, TEXT("LeftFist")));
	TestNull("No socket asked", UPWGasFunctions::FindSocketMesh(Test.Actor, NAME_None));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTagEffectsOverlayTest, "PW.Animation.TagEffects.Overlay", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTagEffectsOverlayTest::RunTest(const FString& Parameters)
{
	const FPWTestWorld Test;
	UStaticMeshComponent* Body = NewObject<UStaticMeshComponent>(Test.Actor);

	UMaterialInstanceConstant* Burn = NewObject<UMaterialInstanceConstant>();
	UMaterialInstanceConstant* Frost = NewObject<UMaterialInstanceConstant>();
	UPWTagEffectSet* Set = NewObject<UPWTagEffectSet>();
	Set->Effects.Add(PWTags::ASC::State::Moving, {{}, Burn});
	Set->Effects.Add(PWTags::ASC::State::Casting::Root, {{}, Frost});

	// The tag is on before anything listens, like a burn that lands before a client's character exists.
	Test.ASC->AddLooseGameplayTag(PWTags::ASC::State::Moving);

	UPWTagEffectsComponent* Effects = NewObject<UPWTagEffectsComponent>(Test.Actor);
	Effects->SetEffectSet(Set);
	Effects->RegisterComponent();
	Effects->SetAbilitySystem(Test.ASC);
	TestEqual("A tag already on plays once listening starts", Body->GetOverlayMaterial(), static_cast<UMaterialInterface*>(Burn));

	Test.ASC->AddLooseGameplayTag(PWTags::ASC::State::Casting::Channeling);
	TestEqual("A child tag plays its parent's entry, and the latest overlay wins", Body->GetOverlayMaterial(), static_cast<UMaterialInterface*>(Frost));

	Test.ASC->RemoveLooseGameplayTag(PWTags::ASC::State::Casting::Channeling);
	TestEqual("The earlier overlay comes back", Body->GetOverlayMaterial(), static_cast<UMaterialInterface*>(Burn));

	Test.ASC->RemoveLooseGameplayTag(PWTags::ASC::State::Moving);
	TestNull("The mesh's own overlay comes back", Body->GetOverlayMaterial());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTagEffectsEventsTest, "PW.Animation.TagEffects.Events", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTagEffectsEventsTest::RunTest(const FString& Parameters)
{
	const FPWTestWorld Test;

	// An entry with nothing in it, only there so Blueprints hear the tag.
	UPWTagEffectSet* Set = NewObject<UPWTagEffectSet>();
	Set->Effects.Add(PWTags::ASC::State::Casting::Root, {});

	UPWTagEffectsComponent* Effects = NewObject<UPWTagEffectsComponent>(Test.Actor);
	Effects->SetEffectSet(Set);
	Effects->RegisterComponent();
	Effects->SetAbilitySystem(Test.ASC);

	UPWTestTagListener* Listener = NewObject<UPWTestTagListener>();
	Effects->OnTagAdded.AddUniqueDynamic(Listener, &UPWTestTagListener::OnAdded);
	Effects->OnTagRemoved.AddUniqueDynamic(Listener, &UPWTestTagListener::OnRemoved);

	Test.ASC->AddLooseGameplayTag(PWTags::ASC::State::Casting::Channeling);
	Test.ASC->RemoveLooseGameplayTag(PWTags::ASC::State::Casting::Channeling);
	TestEqual("Added once", Listener->Added, 1);
	TestEqual("Removed once", Listener->Removed, 1);
	return true;
}

#endif
