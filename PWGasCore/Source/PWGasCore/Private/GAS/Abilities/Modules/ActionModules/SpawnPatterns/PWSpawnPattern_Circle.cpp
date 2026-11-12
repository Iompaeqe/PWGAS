// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "GAS/Abilities/Modules/ActionModules/SpawnPatterns/PWSpawnPattern_Circle.h"

#include "Algo/Reverse.h"

TArray<FVector> UPWSpawnPattern_Circle::BuildPositions_Implementation(int32 Count, float Radius) const
{
	TArray<FVector> Positions;
	const float Step = (bClockwise ? 360.f : -360.f) / FMath::Max(Count, 1);
	for (int32 i = 0; i < Count; ++i)
		Positions.Add(FVector::ForwardVector.RotateAngleAxis(Step * i, FVector::UpVector) * Radius);

	return Positions;
}
