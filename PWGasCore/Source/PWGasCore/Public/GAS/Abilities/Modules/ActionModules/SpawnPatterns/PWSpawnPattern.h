// copyright mkurtt96

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
};
