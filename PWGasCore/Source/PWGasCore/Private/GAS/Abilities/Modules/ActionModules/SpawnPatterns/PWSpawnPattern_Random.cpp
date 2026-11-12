// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "GAS/Abilities/Modules/ActionModules/SpawnPatterns/PWSpawnPattern_Random.h"

#include "Algo/Reverse.h"

TArray<FVector> UPWSpawnPattern_Random::BuildPositions_Implementation(int32 Count, float Radius) const
{
	TArray<FVector> Positions;
	for (int32 i = 0; i < Count; ++i)
		Positions.Add(FVector::ForwardVector.RotateAngleAxis(FMath::FRand() * 360.f, FVector::UpVector) * (FMath::Sqrt(FMath::FRand()) * Radius));

	return Positions;
}
