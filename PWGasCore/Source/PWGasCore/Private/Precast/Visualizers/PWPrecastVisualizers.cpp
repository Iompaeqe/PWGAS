// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "Precast/Visualizers/PWPrecastVisualizers.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Precast/Data/PWPrecastData.h"

void UPWPrecastVisualizer_GhostActors::Ensure_Implementation(APlayerController* PC, float PreviewRadius, FName OriginSocket)
{
	CachedPC = PC;
}

void UPWPrecastVisualizer_GhostActors::Update_Implementation(const FPWPrecastVisualUpdate& Data)
{
	UWorld* World = CachedPC.IsValid() ? CachedPC->GetWorld() : nullptr;
	if (!World)
		return;

	const TSubclassOf<AActor> SourceClass = GhostActorClass ? GhostActorClass : Data.ActorClass;
	if (SourceClass != GhostSourceClass)
	{
		for (const TWeakObjectPtr<AActor>& Ghost : Ghosts)
			if (Ghost.IsValid())
				Ghost->Destroy();
		Ghosts.Reset();
		GhostSourceClass = SourceClass;
	}

	Ghosts.RemoveAll([](const TWeakObjectPtr<AActor>& Ghost) { return !Ghost.IsValid(); });
	while (Ghosts.Num() < Data.SpawnTransforms.Num())
	{
		AActor* Ghost = SpawnGhost(World, SourceClass);
		if (!Ghost)
			break;
		Ghosts.Add(Ghost);
	}

	for (int32 i = 0; i < Ghosts.Num(); ++i)
	{
		const bool bUsed = i < Data.SpawnTransforms.Num();
		Ghosts[i]->SetActorHiddenInGame(!bUsed);
		if (bUsed)
			Ghosts[i]->SetActorTransform(GhostTransform * Data.SpawnTransforms[i]);
	}
}

void UPWPrecastVisualizer_GhostActors::Hide_Implementation()
{
	for (const TWeakObjectPtr<AActor>& Ghost : Ghosts)
		if (Ghost.IsValid())
			Ghost->SetActorHiddenInGame(true);
}

AActor* UPWPrecastVisualizer_GhostActors::SpawnGhost(UWorld* World, const TSubclassOf<AActor> SourceClass) const
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.ObjectFlags |= RF_Transient;

	AActor* Ghost = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, Params);
	if (!Ghost)
		return nullptr;

	USceneComponent* Root = NewObject<USceneComponent>(Ghost);
	Ghost->SetRootComponent(Root);
	Root->RegisterComponent();

	UMaterialInterface* GhostMaterial = Material;
	if (!GhostMaterial)
		if (UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineDebugMaterials/M_SimpleUnlitTranslucent.M_SimpleUnlitTranslucent")))
		{
			UMaterialInstanceDynamic* Dynamic = UMaterialInstanceDynamic::Create(Base, Ghost);
			Dynamic->SetVectorParameterValue(TEXT("Color"), GhostColor);
			GhostMaterial = Dynamic;
		}

	bool bHasMesh = false;
	if (SourceClass)
		AActor::ForEachComponentOfActorClassDefault<UStaticMeshComponent>(SourceClass, [&](const UStaticMeshComponent* Template)
		{
			if (Template->GetStaticMesh())
			{
				AddGhostMesh(Ghost, Template->GetStaticMesh(), Template->GetRelativeTransform(), GhostMaterial);
				bHasMesh = true;
			}
			return true;
		});

	if (!bHasMesh)
		AddGhostMesh(Ghost, Mesh ? Mesh.Get() : LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")), FTransform::Identity, GhostMaterial);

	return Ghost;
}

void UPWPrecastVisualizer_GhostActors::AddGhostMesh(AActor* Ghost, UStaticMesh* GhostMesh, const FTransform& Relative, UMaterialInterface* GhostMaterial) const
{
	UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(Ghost);
	Component->SetStaticMesh(GhostMesh);
	Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Component->SetCastShadow(false);
	Component->SetupAttachment(Ghost->GetRootComponent());
	Component->SetRelativeTransform(Relative);
	if (GhostMaterial)
		for (int32 i = 0; i < Component->GetNumMaterials(); ++i)
			Component->SetMaterial(i, GhostMaterial);
	Component->RegisterComponent();
}

void UPWPrecastVisualizer_Group::Ensure_Implementation(APlayerController* PC, float PreviewRadius, FName OriginSocket)
{
	for (UPWPrecastVisualizer* Visualizer : Visualizers)
		if (Visualizer)
			Visualizer->Ensure(PC, PreviewRadius, OriginSocket);
}

void UPWPrecastVisualizer_Group::Update_Implementation(const FPWPrecastVisualUpdate& Data)
{
	for (UPWPrecastVisualizer* Visualizer : Visualizers)
		if (Visualizer)
			Visualizer->Update(Data);
}

void UPWPrecastVisualizer_Group::Hide_Implementation()
{
	for (UPWPrecastVisualizer* Visualizer : Visualizers)
		if (Visualizer)
			Visualizer->Hide();
}
