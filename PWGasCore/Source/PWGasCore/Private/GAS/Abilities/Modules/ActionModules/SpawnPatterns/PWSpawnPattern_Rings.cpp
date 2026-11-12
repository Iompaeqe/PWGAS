// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "GAS/Abilities/Modules/ActionModules/SpawnPatterns/PWSpawnPattern_Rings.h"

#include "Algo/Reverse.h"

TArray<int32> UPWSpawnPattern_Rings::GetRingCounts(int32 Count) const
{
	TArray<int32> Counts;
	if (Count <= 0)
		return Counts;

	if (Distribution == EPWRingDistribution::Custom)
	{
		for (const int32 Wanted : RingCounts)
		{
			const int32 InRing = FMath::Min(Wanted, Count);
			if (InRing <= 0)
				continue;

			Counts.Add(InRing);
			Count -= InRing;
		}

		if (Counts.IsEmpty())
			Counts.Add(Count);
		else
			Counts[0] += Count;
	}
	else if (Distribution == EPWRingDistribution::ByRingSize)
	{
		const int32 Rings = FMath::Clamp(RingCount, 1, Count);
		const int32 Weight = Rings * (Rings + 1) / 2;

		int32 Remaining = Count;
		for (int32 Ring = 0; Ring < Rings; ++Ring)
		{
			Counts.Add(Count * (Rings - Ring) / Weight);
			Remaining -= Counts.Last();
		}
		for (int32 Ring = 0; Remaining > 0; Ring = (Ring + 1) % Rings, --Remaining)
			Counts[Ring]++;

		Counts.RemoveAll([](const int32 InRing) { return InRing <= 0; });
	}
	else
	{
		const int32 PerRing = FMath::Max(ActorsPerRing, 1);
		for (; Count > 0; Count -= PerRing)
			Counts.Add(FMath::Min(PerRing, Count));
	}

	return Counts;
}

TArray<FVector> UPWSpawnPattern_Rings::BuildPositions_Implementation(int32 Count, float Radius) const
{
	TArray<FVector> Positions;
	const bool bHasCenter = bCenterActor && Count > 0;
	const float Inset = bHasCenter ? 0.f : 0.5f;
	const TArray<int32> Counts = GetRingCounts(bHasCenter ? Count - 1 : Count);

	for (int32 Ring = 0; Ring < Counts.Num(); ++Ring)
	{
		const float RingRadius = Radius * (Counts.Num() - Ring - Inset) / (Counts.Num() - Inset);
		for (int32 i = 0; i < Counts[Ring]; ++i)
			Positions.Add(FVector::ForwardVector.RotateAngleAxis(360.f / Counts[Ring] * i + TwistPerRing * Ring, FVector::UpVector) * RingRadius);
	}

	if (bHasCenter)
		Positions.Add(FVector::ZeroVector);
	if (bCenterOut)
		Algo::Reverse(Positions);

	return Positions;
}
