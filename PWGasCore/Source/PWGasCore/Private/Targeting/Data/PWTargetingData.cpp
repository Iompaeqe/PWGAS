// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "Targeting/Data/PWTargetingData.h"
#include "Engine/HitResult.h"

AActor* FPWTargetingResult::GetPrimaryActor() const
{
	return (HitActors.Num() > 0 && HitActors[0]) ? HitActors[0] : Hit.GetActor();
}