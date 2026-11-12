// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)
//
// #pragma once
//
// #include "CoreMinimal.h"
// #include "Components/SplineComponent.h"
// #include "Precast/Data/PWPrecastInterfaces.h"
// #include "UObject/Object.h"
// #include "ProjectilePathVisualizer.generated.h"
//
// /**
//  *	THIS IS AN EXAMPLE CLASS FOR THE PRECAST SYSTEMS' USAGE.
//  *	APrecastDecal Actor class is a basic actor that will show up to visualize the precast. Any type of visual setup can be used.
//  */
//
// UCLASS()
// class PWGASCORE_API UProjectilePathVisualizer : public UPWPrecastVisualizer
// {
// 	GENERATED_BODY()
// public:
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSubclassOf<class APrecastDecal> ImpactDecalClass;
//
// 	virtual void Ensure_Implementation(APlayerController* PC, float, FName) override
// 	{
// 		OwnerPC = PC;
//
// 		if (!SplineActor)
// 		{
// 			SplineActor = PC->GetWorld()->SpawnActor<AActor>();
// 			Spline = NewObject<USplineComponent>(SplineActor, TEXT("PrecastSpline"));
// 			Spline->RegisterComponent();
// 			Spline->AttachToComponent(SplineActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
// 		}
//
// 		if (!ImpactDecal && ImpactDecalClass)
// 		{
// 			ImpactDecal = PC->GetWorld()->SpawnActor<APrecastDecal>(ImpactDecalClass);
// 			ImpactDecal->SetActorHiddenInGame(true);
// 		}
// 	}
//
// 	virtual void Update_Implementation(const FPWPrecastVisualUpdate& Data) override
// 	{
// 		if (Spline)
// 		{
// 			Spline->ClearSplinePoints(false);
// 			for (const FVector& P : Data.PathPoints)
// 				Spline->AddSplinePoint(P, ESplineCoordinateSpace::World, false);
// 			Spline->UpdateSpline();
// 			// (Optionally add spline meshes / niagara ribbon here)
// 		}
//
// 		if (ImpactDecal && Data.ImpactPoint != FVector::ZeroVector)
// 		{
// 			ImpactDecal->SetActorHiddenInGame(false);
// 			ImpactDecal->SetRadius(Data.Radius);
// 			FVector Loc = Data.ImpactPoint;
// 			if (OwnerPC && OwnerPC->GetPawn()) Loc.Z = OwnerPC->GetPawn()->GetActorLocation().Z;
// 			ImpactDecal->SetActorLocation(Loc);
// 		}
// 	}
//
// 	virtual void Hide_Implementation() override
// 	{
// 		if (ImpactDecal) ImpactDecal->SetActorHiddenInGame(true);
// 		// Optionally clear/hide spline meshes / niagara here.
// 	}
//
// private:
// 	UPROPERTY() APlayerController* OwnerPC = nullptr;
// 	UPROPERTY() AActor* SplineActor = nullptr;
// 	UPROPERTY() USplineComponent* Spline = nullptr;
// 	UPROPERTY() APrecastDecal* ImpactDecal = nullptr;
// };