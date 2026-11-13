// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"

class AActor;
class UMaterialInterface;
class UMeshComponent;

// Puts an overlay on the meshes of a changing set of actors and restores them when they leave the set.
struct PWGASCORE_API FPWTargetHighlighter
{
	void Update(const TArray<TObjectPtr<AActor>>& Targets, UMaterialInterface* Overlay, bool bUseCustomDepth, int32 StencilValue);
	void Clear();

	// A see-through tint for when no overlay material is given.
	static UMaterialInterface* MakeTint(UObject* Outer, const FLinearColor& Color);

private:
	struct FPWSavedMesh
	{
		TWeakObjectPtr<UMeshComponent> Mesh;
		TWeakObjectPtr<UMaterialInterface> Overlay;
		bool bCustomDepth = false;
		int32 Stencil = 0;
	};

	static void Restore(const TArray<FPWSavedMesh>& Saved);

	TMap<TWeakObjectPtr<AActor>, TArray<FPWSavedMesh>> Highlighted;
};
