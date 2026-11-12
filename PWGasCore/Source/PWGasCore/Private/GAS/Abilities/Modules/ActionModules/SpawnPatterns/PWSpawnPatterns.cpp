// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "GAS/Abilities/Modules/ActionModules/SpawnPatterns/PWSpawnPatterns.h"

#include "Algo/Reverse.h"

TArray<FVector> UPWSpawnPattern::BuildPositions_Implementation(int32 Count, float Radius) const
{
	TArray<FVector> Positions;
	Positions.Init(FVector::ZeroVector, Count);
	return Positions;
}

static void OrderFromCenter(TArray<FVector>& Positions)
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

TArray<FVector> UPWSpawnPattern_Circle::BuildPositions_Implementation(int32 Count, float Radius) const
{
	TArray<FVector> Positions;
	const float Step = (bClockwise ? 360.f : -360.f) / FMath::Max(Count, 1);
	for (int32 i = 0; i < Count; ++i)
		Positions.Add(FVector::ForwardVector.RotateAngleAxis(Step * i, FVector::UpVector) * Radius);

	return Positions;
}

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

TArray<FVector> UPWSpawnPattern_Grid::BuildPositions_Implementation(int32 Count, float Radius) const
{
	struct FCell
	{
		FVector Position;
		int32 Major;
		int32 Minor;
		float Angle;
	};

	TArray<FCell> Cells;
	if (Count <= 0)
		return TArray<FVector>();

	const int32 WantedRows = Rows > 0 ? FMath::Min(Rows, Count) : FMath::CeilToInt(FMath::Sqrt(static_cast<float>(Count)));
	const int32 Columns = FMath::DivideAndRoundUp(Count, WantedRows);
	const int32 UsedRows = FMath::DivideAndRoundUp(Count, Columns);
	const float CellSize = Radius * 2.f / FMath::Max(1, FMath::Max(UsedRows, Columns) - 1);
	const bool bByColumn = Order == EPWGridOrder::ColumnByColumn;

	for (int32 i = 0; i < Count; ++i)
	{
		const int32 Row = i / Columns;
		const int32 Column = i % Columns;
		const int32 InRow = Row == UsedRows - 1 ? Count - Row * Columns : Columns;

		FCell& Cell = Cells.AddDefaulted_GetRef();
		Cell.Position = FVector((Row - (UsedRows - 1) * 0.5f) * CellSize, (Column - (InRow - 1) * 0.5f) * CellSize, 0.f);
		Cell.Angle = FMath::Atan2(Cell.Position.Y, Cell.Position.X);

		if (Order == EPWGridOrder::CenterOut || Order == EPWGridOrder::EdgesIn)
		{
			const float FromCenter = FMath::Max(FMath::Abs(Cell.Position.X), FMath::Abs(Cell.Position.Y));
			Cell.Major = CellSize > 0.f ? FMath::RoundToInt(FromCenter / CellSize * 2.f) : 0;
			Cell.Minor = 0;
		}
		else
		{
			Cell.Major = bByColumn ? Column : Row;
			Cell.Minor = bByColumn ? Row : Column;
			if (bSnake && Cell.Major % 2 == 1)
				Cell.Minor = -Cell.Minor;
		}
	}

	Cells.StableSort([](const FCell& A, const FCell& B)
	{
		if (A.Major != B.Major)
			return A.Major < B.Major;
		if (A.Minor != B.Minor)
			return A.Minor < B.Minor;
		return A.Angle < B.Angle;
	});

	TArray<FVector> Positions;
	for (const FCell& Cell : Cells)
		Positions.Add(Cell.Position);
	if (Order == EPWGridOrder::EdgesIn)
		Algo::Reverse(Positions);

	return Positions;
}

TArray<FVector> UPWSpawnPattern_Random::BuildPositions_Implementation(int32 Count, float Radius) const
{
	TArray<FVector> Positions;
	for (int32 i = 0; i < Count; ++i)
		Positions.Add(FVector::ForwardVector.RotateAngleAxis(FMath::FRand() * 360.f, FVector::UpVector) * (FMath::Sqrt(FMath::FRand()) * Radius));

	return Positions;
}
