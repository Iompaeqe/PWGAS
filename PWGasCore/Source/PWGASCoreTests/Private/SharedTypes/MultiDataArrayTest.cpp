// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWMultiDataArray.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMultiDataArrayTest, "PW.SharedTypes.MultiDataArray", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMultiDataArrayTest::RunTest(const FString& Parameters)
{
	UPWMultiDataArray* Data = NewObject<UPWMultiDataArray>();

	Data->AddFloat("Range", 750.f);
	Data->AddInt("Count", 3);
	Data->AddVector("Target", FVector(1, 2, 3));

	TestEqual("Float", Data->GetFloat("Range"), 750.f);
	TestEqual("Int", Data->GetInt("Count"), 3);
	TestEqual("Vector", Data->GetVector("Target"), FVector(1, 2, 3));
	TestEqual("Missing key", Data->GetFloat("Missing"), 0.f);

	return true;
}

#endif
