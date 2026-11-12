// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "GAS/Abilities/Modules/ActionModules/SpawnPatterns/PWSpreadPattern.h"

float UPWSpreadPattern::FanYaw(const int32 Index, const int32 Count, const float SpreadAngle)
{
	return Count > 1 ? FMath::Lerp(-SpreadAngle * 0.5f, SpreadAngle * 0.5f, Index / float(Count - 1)) : 0.f;
}

TArray<FPWSpreadShot> UPWSpreadPattern::BuildShots_Implementation(int32 Count, float SpreadAngle) const
{
	TArray<FPWSpreadShot> Shots;
	Shots.SetNum(FMath::Max(Count, 0));
	return Shots;
}
