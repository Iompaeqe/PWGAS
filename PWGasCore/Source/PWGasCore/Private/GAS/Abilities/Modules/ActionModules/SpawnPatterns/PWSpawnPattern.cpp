// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "GAS/Abilities/Modules/ActionModules/SpawnPatterns/PWSpawnPattern.h"

TArray<FVector> UPWSpawnPattern::BuildPositions_Implementation(int32 Count, float Radius) const
{
	TArray<FVector> Positions;
	Positions.Init(FVector::ZeroVector, Count);
	return Positions;
}

void UPWSpawnPattern::OrderFromCenter(TArray<FVector>& Positions)
{
	const int32 Last = Positions.Num() - 1;

	TArray<int32> Order;
	for (int32 i = 0; i <= Last; ++i)
		Order.Add(i);
	Order.StableSort([Last](const int32 A, const int32 B) { return FMath::Abs(2 * A - Last) < FMath::Abs(2 * B - Last); });

	TArray<FVector> Ordered;
	for (const int32 Index : Order)
		Ordered.Add(Positions[Index]);
	Positions = MoveTemp(Ordered);
}
