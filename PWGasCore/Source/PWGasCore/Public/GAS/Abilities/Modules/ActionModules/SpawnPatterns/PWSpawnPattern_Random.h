// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWSpawnPattern.h"
#include "PWSpawnPattern_Random.generated.h"

UCLASS(meta=(DisplayName="Random"))
class PWGASCORE_API UPWSpawnPattern_Random : public UPWSpawnPattern
{
	GENERATED_BODY()

public:
	virtual TArray<FVector> BuildPositions_Implementation(int32 Count, float Radius) const override;
};
