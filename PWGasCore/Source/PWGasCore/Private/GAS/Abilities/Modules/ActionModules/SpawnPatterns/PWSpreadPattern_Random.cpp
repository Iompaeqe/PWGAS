// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "GAS/Abilities/Modules/ActionModules/SpawnPatterns/PWSpreadPattern_Random.h"

#include "Algo/RandomShuffle.h"
#include "Algo/Reverse.h"

TArray<FPWSpreadShot> UPWSpreadPattern_Random::BuildShots_Implementation(int32 Count, float SpreadAngle) const
{
	TArray<int32> Indices;
	for (int32 i = 0; i < Count; ++i)
		Indices.Add(i);
	Algo::RandomShuffle(Indices);

	TArray<FPWSpreadShot> Shots;
	for (int32 Step = 0; Step < Indices.Num(); ++Step)
	{
		FPWSpreadShot& Shot = Shots.AddDefaulted_GetRef();
		Shot.Offset.Yaw = FanYaw(Indices[Step], Count, SpreadAngle);
		if (bRandomPitch)
			Shot.Offset.Pitch = FMath::FRandRange(-SpreadAngle * 0.5f, SpreadAngle * 0.5f);
		Shot.Step = Step;
	}
	return Shots;
}
