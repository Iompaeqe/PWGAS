// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "GAS/Abilities/Modules/ActionModules/SpawnPatterns/PWSpawnPattern_Grid.h"

#include "Algo/Reverse.h"

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
