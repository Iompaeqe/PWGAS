// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "Precast/Visualizers/PWPrecastVisualizer_ProjectilePath.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Precast/Data/PWPrecastData.h"

void UPWPrecastVisualizer_ProjectilePath::Ensure_Implementation(APlayerController* PC, float PreviewRadius, FName OriginSocket)
{
	CachedPC = PC;
	CachedRadius = PreviewRadius;
}

void UPWPrecastVisualizer_ProjectilePath::Update_Implementation(const FPWPrecastVisualUpdate& Data)
{
	UWorld* World = CachedPC.IsValid() ? CachedPC->GetWorld() : nullptr;
	if (!World)
		return;

	if (!Holder.IsValid())
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Params.ObjectFlags |= RF_Transient;
		AActor* NewHolder = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, Params);
		if (!NewHolder)
			return;

		USceneComponent* Root = NewObject<USceneComponent>(NewHolder);
		NewHolder->SetRootComponent(Root);
		Root->RegisterComponent();
		Holder = NewHolder;

		UStaticMesh* DefaultPath = LoadObject<UStaticMesh>(nullptr, Style == EPWPathStyle::Dots ? TEXT("/Engine/BasicShapes/Sphere.Sphere") : TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
		PathMeshes = AddMeshes(NewHolder, PathMesh ? PathMesh.Get() : DefaultPath, PathMaterial, PathColor);
		if (bShowImpact)
			ImpactMeshes = AddMeshes(NewHolder, ImpactMesh ? ImpactMesh.Get() : LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")), ImpactMaterial, ImpactColor);
	}
	Holder->SetActorHiddenInGame(false);

	TArray<FPWPrecastPath> Paths = Data.Paths;
	if (Paths.IsEmpty() && Data.PathPoints.Num() > 1)
	{
		FPWPrecastPath& Single = Paths.AddDefaulted_GetRef();
		Single.Points = Data.PathPoints;
		Single.ImpactPoint = Data.ImpactPoint;
		Single.ImpactNormal = Data.ImpactNormal;
	}

	const float Radius = Data.Radius > 0.f ? Data.Radius : (CachedRadius > 0.f ? CachedRadius : ImpactSize);

	TArray<FTransform> PathTransforms;
	TArray<FTransform> ImpactTransforms;
	for (const FPWPrecastPath& Path : Paths)
	{
		BuildPathTransforms(Path.Points, PathTransforms);

		if (Path.ImpactPoint.IsNearlyZero())
			continue;
		const FVector Normal = Path.ImpactNormal.GetSafeNormal(UE_SMALL_NUMBER, FVector::UpVector);
		ImpactTransforms.Add(FTransform(FRotationMatrix::MakeFromZ(Normal).ToQuat(), Path.ImpactPoint, FVector(Radius / 50.f)));
	}

	if (PathMeshes.IsValid())
	{
		PathMeshes->ClearInstances();
		PathMeshes->AddInstances(PathTransforms, false, true);
	}
	if (ImpactMeshes.IsValid())
	{
		ImpactMeshes->ClearInstances();
		ImpactMeshes->AddInstances(ImpactTransforms, false, true);
	}

	if (bHighlightTargets)
	{
		if (!HighlightMaterial && !DefaultHighlight)
			DefaultHighlight = FPWTargetHighlighter::MakeTint(this, HighlightColor);
		Highlighter.Update(Data.Targets, HighlightMaterial ? HighlightMaterial.Get() : DefaultHighlight.Get(), false, 0);
	}
}

void UPWPrecastVisualizer_ProjectilePath::Hide_Implementation()
{
	if (Holder.IsValid())
		Holder->SetActorHiddenInGame(true);
	Highlighter.Clear();
}

void UPWPrecastVisualizer_ProjectilePath::BuildPathTransforms(const TArray<FVector>& Points, TArray<FTransform>& OutTransforms) const
{
	const float Width = Thickness / 100.f;

	if (Style == EPWPathStyle::Line)
	{
		for (int32 i = 1; i < Points.Num(); ++i)
		{
			const FVector Segment = Points[i] - Points[i - 1];
			const float Length = Segment.Size();
			if (Length > UE_KINDA_SMALL_NUMBER)
				OutTransforms.Add(FTransform(FRotationMatrix::MakeFromZ(Segment / Length).ToQuat(), (Points[i] + Points[i - 1]) * 0.5f, FVector(Width, Width, Length / 100.f)));
		}
		return;
	}

	float UntilNext = 0.f;
	for (int32 i = 1; i < Points.Num(); ++i)
	{
		const FVector Segment = Points[i] - Points[i - 1];
		const float Length = Segment.Size();
		if (Length <= UE_KINDA_SMALL_NUMBER)
			continue;

		float Along = UntilNext;
		for (; Along <= Length; Along += Spacing)
			OutTransforms.Add(FTransform(FQuat::Identity, Points[i - 1] + Segment * (Along / Length), FVector(Width)));
		UntilNext = Along - Length;
	}
}

UInstancedStaticMeshComponent* UPWPrecastVisualizer_ProjectilePath::AddMeshes(AActor* Owner, UStaticMesh* Mesh, UMaterialInterface* Material, const FLinearColor& Color) const
{
	UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner);
	Component->SetStaticMesh(Mesh);
	Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Component->SetCastShadow(false);
	Component->SetupAttachment(Owner->GetRootComponent());

	UMaterialInterface* Used = Material;
	if (!Used)
		if (UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineDebugMaterials/M_SimpleUnlitTranslucent.M_SimpleUnlitTranslucent")))
		{
			UMaterialInstanceDynamic* Dynamic = UMaterialInstanceDynamic::Create(Base, Owner);
			Dynamic->SetVectorParameterValue(TEXT("Color"), Color);
			Used = Dynamic;
		}
	if (Used)
		for (int32 i = 0; i < Component->GetNumMaterials(); ++i)
			Component->SetMaterial(i, Used);

	Component->RegisterComponent();
	return Component;
}
