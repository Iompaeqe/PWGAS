// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "Precast/Visualizers/PWPrecastVisualizer_Group.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Precast/Data/PWPrecastData.h"

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
