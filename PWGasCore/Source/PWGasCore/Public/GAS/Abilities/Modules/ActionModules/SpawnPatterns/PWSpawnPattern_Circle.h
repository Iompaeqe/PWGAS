// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWSpawnPattern.h"
#include "PWSpawnPattern_Circle.generated.h"

UCLASS(meta=(DisplayName="Circle"))
class PWGASCORE_API UPWSpawnPattern_Circle : public UPWSpawnPattern
{
	GENERATED_BODY()

public:
	// Places the actors clockwise around the circle; off goes counter-clockwise.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
	bool bClockwise = true;

	virtual TArray<FVector> BuildPositions_Implementation(int32 Count, float Radius) const override;
};
