// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "GAS/Abilities/Modules/ActionModules/SpawnPatterns/PWSpreadPattern_Fan.h"

#include "Algo/RandomShuffle.h"
#include "Algo/Reverse.h"

TArray<FPWSpreadShot> UPWSpreadPattern_Fan::BuildShots_Implementation(int32 Count, float SpreadAngle) const
{
	const int32 Last = Count - 1;
	const auto FromCenter = [Last](const int32 Index) { return FMath::Abs(2 * Index - Last); };
	const auto IsCenter = [&](const int32 Index) { return FromCenter(Index) == Last % 2; };
	const bool bFromCenter = Order == EPWSpreadOrder::CenterOut || Order == EPWSpreadOrder::EdgesIn;

	TArray<int32> Indices;
	for (int32 i = 0; i < Count; ++i)
		Indices.Add(i);

	if (Order == EPWSpreadOrder::RightToLeft)
		Algo::Reverse(Indices);
	else if (bFromCenter)
		Indices.Sort([&](const int32 A, const int32 B)
		{
			if (FromCenter(A) != FromCenter(B))
				return Order == EPWSpreadOrder::CenterOut ? FromCenter(A) < FromCenter(B) : FromCenter(A) > FromCenter(B);
			return bRightFirst ? A > B : A < B;
		});

	TArray<FPWSpreadShot> Shots;
	int32 Step = INDEX_NONE;
	int32 Previous = INDEX_NONE;
	for (const int32 Index : Indices)
	{
		bool bSameStep = false;
		if (bFromCenter && Previous != INDEX_NONE)
		{
			if (Grouping == EPWSpreadGrouping::Pairs)
				bSameStep = FromCenter(Index) == FromCenter(Previous);
			else if (Grouping == EPWSpreadGrouping::CenterAndSides)
				bSameStep = IsCenter(Index) == IsCenter(Previous);
		}
		if (!bSameStep)
			++Step;

		FPWSpreadShot& Shot = Shots.AddDefaulted_GetRef();
		Shot.Offset = FRotator(0.f, FanYaw(Index, Count, SpreadAngle), 0.f);
		Shot.Step = Step;
		Previous = Index;
	}
	return Shots;
}
