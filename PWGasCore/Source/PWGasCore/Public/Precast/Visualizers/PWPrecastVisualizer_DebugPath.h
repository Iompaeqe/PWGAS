// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "Precast/Visualizers/PWPrecastVisualizer.h"
#include "PWPrecastVisualizer_DebugPath.generated.h"

// Draws the projectile path, landing area and surface normal with debug lines.
UCLASS(meta=(DisplayName="Debug Path"))
class PWGASCORE_API UPWPrecastVisualizer_DebugPath : public UPWPrecastVisualizer
{
	GENERATED_BODY()

public:
	// UPWPrecastVisualizer interface
	virtual void Ensure_Implementation(class APlayerController* PC, float PreviewRadius, FName OriginSocket) override;
	virtual void Update_Implementation(const FPWPrecastVisualUpdate& Data) override;
	virtual void Hide_Implementation() override;

protected:
	/** Cached local controller for world access */
	UPROPERTY()
	TWeakObjectPtr<APlayerController> CachedPC;

	/** Last known preview radius (used if Data.Radius == 0) */
	UPROPERTY()
	float CachedRadius = 0.f;

	/** Visual tuning */
	UPROPERTY(EditDefaultsOnly, Category="Debug")
	float LineThickness = 2.0f;

	// Segments used to draw the impact sphere; higher is rounder.
	UPROPERTY(EditDefaultsOnly, Category="Debug")
	int32 SphereSegments = 24;

	// Color of the predicted path.
	UPROPERTY(EditDefaultsOnly, Category="Debug")
	FColor PathColor = FColor::Cyan;

	// Color of the sphere drawn at the impact point.
	UPROPERTY(EditDefaultsOnly, Category="Debug")
	FColor ImpactSphereColor = FColor::Red;

	// Color of the arrow showing the surface normal at the impact point.
	UPROPERTY(EditDefaultsOnly, Category="Debug")
	FColor ImpactNormalColor = FColor::Yellow;

	/** Length of the impact normal arrow (in units) */
	UPROPERTY(EditDefaultsOnly, Category="Debug")
	float ImpactNormalLength = 100.f;
};
