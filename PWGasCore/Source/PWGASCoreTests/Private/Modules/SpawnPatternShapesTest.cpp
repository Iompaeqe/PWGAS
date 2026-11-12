// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "GAS/Abilities/Modules/ActionModules/SpawnPatterns/PWSpawnPattern_Circle.h"
#include "GAS/Abilities/Modules/ActionModules/SpawnPatterns/PWSpawnPattern_Grid.h"
#include "GAS/Abilities/Modules/ActionModules/SpawnPatterns/PWSpawnPattern_Line.h"
#include "GAS/Abilities/Modules/ActionModules/SpawnPatterns/PWSpawnPattern_Random.h"
#include "GAS/Abilities/Modules/ActionModules/SpawnPatterns/PWSpawnPattern_Rings.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

static TArray<FVector> Sorted(TArray<FVector> Positions)
{
	Positions.Sort([](const FVector& A, const FVector& B) { return FMath::IsNearlyEqual(A.X, B.X, 0.1) ? A.Y < B.Y : A.X < B.X; });
	return Positions;
}

static bool SameSpots(const TArray<FVector>& A, const TArray<FVector>& B)
{
	const TArray<FVector> SortedA = Sorted(A);
	const TArray<FVector> SortedB = Sorted(B);
	if (SortedA.Num() != SortedB.Num())
		return false;

	for (int32 i = 0; i < SortedA.Num(); ++i)
		if (!SortedA[i].Equals(SortedB[i], 0.5f))
			return false;
	return true;
}

static void CheckShape(FAutomationTestBase& Test, const FString& Name, const UPWSpawnPattern* Pattern, const int32 Count, const float Reach, const bool bUnique = true)
{
	const TArray<FVector> Positions = Pattern->BuildPositions(Count, 300.f);

	bool bInReach = true;
	bool bFlat = true;
	bool bNoOverlap = true;
	for (int32 i = 0; i < Positions.Num(); ++i)
	{
		bInReach &= FMath::Max(FMath::Abs(Positions[i].X), FMath::Abs(Positions[i].Y)) <= Reach + 0.5f;
		bFlat &= Positions[i].Z == 0.f;
		for (int32 j = 0; j < i; ++j)
			bNoOverlap &= !Positions[i].Equals(Positions[j], 1.f);
	}

	const FString Label = FString::Printf(TEXT("%s x%d"), *Name, Count);
	Test.TestEqual(Label + TEXT(" count"), Positions.Num(), Count);
	Test.TestTrue(Label + TEXT(" stays in reach"), bInReach);
	Test.TestTrue(Label + TEXT(" is flat"), bFlat);
	if (bUnique)
		Test.TestTrue(Label + TEXT(" has no overlapping spots"), bNoOverlap);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpawnPatternShapesTest, "PW.Modules.SpawnPattern.Shapes", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSpawnPatternShapesTest::RunTest(const FString& Parameters)
{
	for (int32 Count = 1; Count <= 24; ++Count)
	{
		UPWSpawnPattern_Circle* Circle = NewObject<UPWSpawnPattern_Circle>();
		for (const bool bClockwise : {true, false})
		{
			Circle->bClockwise = bClockwise;
			CheckShape(*this, FString::Printf(TEXT("Circle cw%d"), bClockwise), Circle, Count, 300.f);
		}

		UPWSpawnPattern_Line* Line = NewObject<UPWSpawnPattern_Line>();
		for (const EPWLineAxis Axis : {EPWLineAxis::Across, EPWLineAxis::Along})
			for (const bool bCentered : {true, false})
			{
				Line->Axis = Axis;
				Line->bCentered = bCentered;
				Line->Order = EPWLineOrder::StartToEnd;
				const TArray<FVector> Base = Line->BuildPositions(Count, 300.f);

				for (const EPWLineOrder Order : {EPWLineOrder::StartToEnd, EPWLineOrder::EndToStart, EPWLineOrder::CenterOut, EPWLineOrder::EdgesIn})
				{
					Line->Order = Order;
					const FString Name = FString::Printf(TEXT("Line axis%d centered%d order%d"), static_cast<int32>(Axis), bCentered, static_cast<int32>(Order));
					CheckShape(*this, Name, Line, Count, bCentered ? 300.f : 600.f);
					TestTrue(Name + TEXT(" only changes the order"), SameSpots(Base, Line->BuildPositions(Count, 300.f)));
				}
			}

		UPWSpawnPattern_Rings* Rings = NewObject<UPWSpawnPattern_Rings>();
		Rings->TwistPerRing = 17.f;
		for (const bool bCenterActor : {true, false})
			for (const bool bCenterOut : {true, false})
			{
				Rings->bCenterActor = bCenterActor;
				Rings->bCenterOut = bCenterOut;
				const FString Suffix = FString::Printf(TEXT(" center%d out%d"), bCenterActor, bCenterOut);

				Rings->Distribution = EPWRingDistribution::SamePerRing;
				for (const int32 PerRing : {1, 2, 5})
				{
					Rings->ActorsPerRing = PerRing;
					CheckShape(*this, FString::Printf(TEXT("Rings same%d"), PerRing) + Suffix, Rings, Count, 300.f);
				}

				Rings->Distribution = EPWRingDistribution::ByRingSize;
				for (const int32 RingCount : {1, 3, 6})
				{
					Rings->RingCount = RingCount;
					CheckShape(*this, FString::Printf(TEXT("Rings size%d"), RingCount) + Suffix, Rings, Count, 300.f);
				}

				Rings->Distribution = EPWRingDistribution::Custom;
				for (const TArray<int32>& Custom : {TArray<int32>({9, 6, 4}), TArray<int32>({3}), TArray<int32>({0, -2, 5}), TArray<int32>()})
				{
					Rings->RingCounts = Custom;
					CheckShape(*this, FString::Printf(TEXT("Rings custom%d"), Custom.Num()) + Suffix, Rings, Count, 300.f);
				}
			}

		UPWSpawnPattern_Grid* Grid = NewObject<UPWSpawnPattern_Grid>();
		for (const int32 Rows : {0, 1, 3, 30})
		{
			Grid->Rows = Rows;
			Grid->Order = EPWGridOrder::RowByRow;
			Grid->bSnake = false;
			const TArray<FVector> Base = Grid->BuildPositions(Count, 300.f);

			for (const EPWGridOrder Order : {EPWGridOrder::RowByRow, EPWGridOrder::ColumnByColumn, EPWGridOrder::CenterOut, EPWGridOrder::EdgesIn})
				for (const bool bSnake : {false, true})
				{
					Grid->Order = Order;
					Grid->bSnake = bSnake;
					const FString Name = FString::Printf(TEXT("Grid rows%d order%d snake%d"), Rows, static_cast<int32>(Order), bSnake);
					CheckShape(*this, Name, Grid, Count, 300.f);
					TestTrue(Name + TEXT(" only changes the order"), SameSpots(Base, Grid->BuildPositions(Count, 300.f)));
				}
		}

		CheckShape(*this, TEXT("Random"), NewObject<UPWSpawnPattern_Random>(), Count, 300.f, false);
	}

	return true;
}

#endif
