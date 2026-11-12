// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PWAbilityMathLibrary.generated.h"

UCLASS()
class PWGASCORE_API UPWAbilityMathLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category="USpellFunctionLibrary|SpellFunctions")
	static TArray<FRotator> EvenlySpreadRotators(const FVector& Forward, const FVector& Axis, const float Spread, const int32 Count);

	UFUNCTION(BlueprintPure, Category="USpellFunctionLibrary|SpellFunctions")
	static TArray<FVector> EvenlySpreadVectors(const FVector& Forward, const FVector& Axis, const float Spread, const int32 Count);

	UFUNCTION(BlueprintPure, Category="USpellFunctionLibrary|SpellFunctions")
	static void GetClosestTargets(int32 MaxTargets, const TArray<AActor*>& Actors, TArray<AActor*>& OutClosestTargets, const FVector& Origin);

	UFUNCTION(BlueprintPure, Category="USpellFunctionLibrary|SpellFunctions")
	static void GetClosestTarget(const TArray<AActor*>& Actors, AActor*& OutClosestTarget, const FVector& Origin);
};
