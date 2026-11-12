// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "Precast/Visualizers/PWPrecastVisualizer_DebugPath.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Precast/Data/PWPrecastData.h"

void UPWPrecastVisualizer_DebugPath::Ensure_Implementation(APlayerController* PC, float PreviewRadius, FName /*OriginSocket*/)
{
	CachedPC    = PC;
	CachedRadius = PreviewRadius;
}

void UPWPrecastVisualizer_DebugPath::Update_Implementation(const FPWPrecastVisualUpdate& Data)
{
	if (!CachedPC.IsValid())
	{
		return;
	}

	UWorld* World = CachedPC->GetWorld();
	if (!World)
	{
		return;
	}

	TArray<FPWPrecastPath> Paths = Data.Paths;
	if (Paths.IsEmpty() && Data.PathPoints.Num() > 1)
		Paths.AddDefaulted_GetRef().Points = Data.PathPoints;

	const float Radius = (Data.Radius > 0.f) ? Data.Radius : CachedRadius;
	const bool bHasRadius = Radius > KINDA_SMALL_NUMBER;

	for (const FPWPrecastPath& Path : Paths)
	{
		for (int32 Index = 1; Index < Path.Points.Num(); ++Index)
			DrawDebugLine(World, Path.Points[Index - 1], Path.Points[Index], PathColor, false, 0.f, 0, LineThickness);

		// Every projectile shows its own area; without a radius it gets a small marker.
		if (Paths.Num() > 1 && !Path.ImpactPoint.IsNearlyZero())
			DrawDebugSphere(World, Path.ImpactPoint, bHasRadius ? Radius : 15.f, bHasRadius ? SphereSegments : 8, ImpactSphereColor, false, 0.f, 0, LineThickness);
	}

	for (const FTransform& Spawn : Data.SpawnTransforms)
	{
		DrawDebugSphere(World, Spawn.GetLocation(), 20.f, 8, ImpactSphereColor, false, 0.f, 0, LineThickness);
		DrawDebugLine(World, Spawn.GetLocation(), Spawn.GetLocation() + Spawn.GetRotation().GetForwardVector() * 50.f, ImpactNormalColor, false, 0.f, 0, LineThickness);
	}

	// --- Explosion radius at impact point ---
	const FVector ImpactPoint = Data.ImpactPoint;

	if (Paths.Num() <= 1 && !ImpactPoint.IsNearlyZero() && bHasRadius)
	{
		DrawDebugSphere(
			World,
			ImpactPoint,
			Radius,
			SphereSegments,
			ImpactSphereColor,
			/*bPersistentLines=*/false,
			/*LifeTime=*/0.0f,
			/*DepthPriority=*/0,
			/*Thickness=*/LineThickness
		);
	}

	// --- Impact normal visualization ---
	if (!ImpactPoint.IsNearlyZero() && !Data.ImpactNormal.IsNearlyZero())
	{
		const FVector Start = ImpactPoint;
		const FVector End   = Start + Data.ImpactNormal.GetSafeNormal() * ImpactNormalLength;

		DrawDebugLine(
			World,
			Start,
			End,
			ImpactNormalColor,
			/*bPersistentLines=*/false,
			/*LifeTime=*/0.0f,
			/*DepthPriority=*/0,
			/*Thickness=*/LineThickness
		);
	}
}

void UPWPrecastVisualizer_DebugPath::Hide_Implementation()
{
	// Nothing to do for debug lines with LifeTime=0:
	// they only live for a single frame and will stop appearing
	// as soon as Update() stops being called.
	CachedPC = nullptr;
}
