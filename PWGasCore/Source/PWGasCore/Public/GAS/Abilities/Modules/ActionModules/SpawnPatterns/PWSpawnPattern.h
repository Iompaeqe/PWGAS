// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PWSpawnPattern.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWSpawnPattern : public UObject
{
	GENERATED_BODY()

public:
	// Positions around the pattern center in spawn order. X is forward, Y is right.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Pattern")
	TArray<FVector> BuildPositions(int32 Count, float Radius) const;

protected:
	// Reorders positions so the middle ones come first.
	static void OrderFromCenter(TArray<FVector>& Positions);
};
