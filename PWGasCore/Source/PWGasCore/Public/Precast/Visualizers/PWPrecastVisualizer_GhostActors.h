// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "Precast/Visualizers/PWPrecastVisualizer.h"
#include "PWPrecastVisualizer_GhostActors.generated.h"

class UStaticMesh;
class UMaterialInterface;

// Shows a see-through copy of the ability's actor at every place one will spawn.
// Only the meshes are copied, the actor itself is never spawned.
UCLASS(meta=(DisplayName="Ghost Actors"))
class PWGASCORE_API UPWPrecastVisualizer_GhostActors : public UPWPrecastVisualizer
{
	GENERATED_BODY()

public:
	// Optional. Copies the meshes of this actor instead of the ability's own actor.
	UPROPERTY(EditDefaultsOnly, Category="Ghost")
	TSubclassOf<AActor> GhostActorClass;

	// Used when the actor has no static meshes.
	UPROPERTY(EditDefaultsOnly, Category="Ghost")
	TObjectPtr<UStaticMesh> Mesh;

	// Optional. Replaces the default see-through material.
	UPROPERTY(EditDefaultsOnly, Category="Ghost")
	TObjectPtr<UMaterialInterface> Material;

	// Tint and opacity of the ghost preview.
	UPROPERTY(EditDefaultsOnly, Category="Ghost")
	FLinearColor GhostColor = FLinearColor(0.2f, 0.8f, 1.f, 0.35f);

	// Offset, rotation and scale applied to the ghost mesh.
	UPROPERTY(EditDefaultsOnly, Category="Ghost")
	FTransform GhostTransform;

	virtual void Ensure_Implementation(APlayerController* PC, float PreviewRadius, FName OriginSocket) override;
	virtual void Update_Implementation(const FPWPrecastVisualUpdate& Data) override;
	virtual void Hide_Implementation() override;

private:
	AActor* SpawnGhost(UWorld* World, TSubclassOf<AActor> SourceClass) const;
	void AddGhostMesh(AActor* Ghost, UStaticMesh* GhostMesh, const FTransform& Relative, UMaterialInterface* GhostMaterial) const;

	TWeakObjectPtr<APlayerController> CachedPC;
	TArray<TWeakObjectPtr<AActor>> Ghosts;
	TSubclassOf<AActor> GhostSourceClass;
};
