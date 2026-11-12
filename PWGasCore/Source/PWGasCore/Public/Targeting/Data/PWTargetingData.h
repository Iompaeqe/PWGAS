// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWTargetingData.generated.h"

USTRUCT(BlueprintType)
struct FPWTargetingResult
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) FVector Location = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly) FRotator Rotation = FRotator::ZeroRotator;
	UPROPERTY(BlueprintReadOnly) bool bHasHit = false;
	UPROPERTY(BlueprintReadOnly) FHitResult Hit;
	UPROPERTY(BlueprintReadOnly) TArray<AActor*> HitActors;

	AActor* GetPrimaryActor() const;

	bool IsValid() const
	{
		return bHasHit || !Location.IsNearlyZero();
	}
};

USTRUCT(BlueprintType)
struct FPWProjectileSimConfig
{
	GENERATED_BODY()
	// Launch speed in units per second.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float InitialSpeed = 1200.f;
	// Gravity applied to the projectile; 0 uses the projectile's own setting.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float GravityZOverride = 0.f;
	// Collision radius of the preview trace when the projectile actor has no sphere to read it from.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ProjectileRadius = 8.f;
	// Seconds of flight the precast preview simulates.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxSimTime = 2.f;
	// Simulation steps per second for the preview; higher is smoother and costs more.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float SimFrequency = 30.f;
	// Object types the preview path stops against, matching what the projectile itself collides with.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<TEnumAsByte<ECollisionChannel>> PreviewObjectTypes = {ECC_WorldStatic, ECC_WorldDynamic, ECC_Pawn, ECC_PhysicsBody, ECC_Destructible};
	// Trace channel used for the preview path when Preview Object Types is empty.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;
	// Tilts the impact marker to the surface it lands on; off keeps it flat.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAlignToGroundNormal = true;
};