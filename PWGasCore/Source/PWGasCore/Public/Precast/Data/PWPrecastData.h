// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Templates/SubclassOf.h"

#include "PWPrecastData.generated.h"

USTRUCT(BlueprintType)
struct FPWPrecastPath
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite) TArray<FVector> Points;
	UPROPERTY(BlueprintReadWrite) FVector ImpactPoint = FVector::ZeroVector;
	UPROPERTY(BlueprintReadWrite) FVector ImpactNormal = FVector::UpVector;
	UPROPERTY(BlueprintReadWrite) bool bHit = false;
};

USTRUCT(BlueprintType)
struct FPWPrecastVisualUpdate
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite) TArray<FVector> PathPoints;
	// One per projectile. PathPoints and the impact below are the middle one.
	UPROPERTY(BlueprintReadWrite) TArray<FPWPrecastPath> Paths;
	UPROPERTY(BlueprintReadWrite) TArray<FTransform> SpawnTransforms;
	UPROPERTY(BlueprintReadWrite) TSubclassOf<AActor> ActorClass;
	UPROPERTY(BlueprintReadWrite) FVector ImpactPoint = FVector::ZeroVector;
	UPROPERTY(BlueprintReadWrite) FVector ImpactNormal = FVector::UpVector;
	UPROPERTY(BlueprintReadWrite) float Radius = 0.f;
	UPROPERTY(BlueprintReadWrite) bool bValid = true;
};
