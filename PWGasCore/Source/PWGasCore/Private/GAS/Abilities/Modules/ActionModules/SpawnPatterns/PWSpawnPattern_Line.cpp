// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "GAS/Abilities/Modules/ActionModules/SpawnPatterns/PWSpawnPattern_Line.h"

#include "Algo/Reverse.h"

TArray<FVector> UPWSpawnPattern_Line::BuildPositions_Implementation(int32 Count, float Radius) const
{
	TArray<FVector> Positions;
	const FVector Direction = Axis == EPWLineAxis::Across ? FVector::RightVector : FVector::ForwardVector;
	const float Spacing = Count > 1 ? Radius * 2.f / (Count - 1) : 0.f;
	const FVector Start = bCentered && Count > 1 ? -Direction * Radius : FVector::ZeroVector;

	for (int32 i = 0; i < Count; ++i)
		Positions.Add(Start + Direction * (Spacing * i));

	if (Order == EPWLineOrder::CenterOut || Order == EPWLineOrder::EdgesIn)
		OrderFromCenter(Positions);
	if (Order == EPWLineOrder::EndToStart || Order == EPWLineOrder::EdgesIn)
		Algo::Reverse(Positions);

	return Positions;
}
