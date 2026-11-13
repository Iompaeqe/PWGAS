// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "Precast/Visualizers/PWPrecastTargetHighlighter.h"

#include "Components/MeshComponent.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"

void FPWTargetHighlighter::Update(const TArray<TObjectPtr<AActor>>& Targets, UMaterialInterface* Overlay, const bool bUseCustomDepth, const int32 StencilValue)
{
	for (auto It = Highlighted.CreateIterator(); It; ++It)
	{
		if (It.Key().IsValid() && Targets.Contains(It.Key().Get()))
			continue;
		Restore(It.Value());
		It.RemoveCurrent();
	}

	for (AActor* Target : Targets)
	{
		if (!Target || Highlighted.Contains(Target))
			continue;

		TArray<FPWSavedMesh>& Saved = Highlighted.Add(Target);
		TInlineComponentArray<UMeshComponent*> Meshes(Target);
		for (UMeshComponent* Mesh : Meshes)
		{
			FPWSavedMesh& Entry = Saved.AddDefaulted_GetRef();
			Entry.Mesh = Mesh;
			Entry.Overlay = Mesh->GetOverlayMaterial();
			Entry.bCustomDepth = Mesh->bRenderCustomDepth;
			Entry.Stencil = Mesh->CustomDepthStencilValue;

			if (Overlay)
				Mesh->SetOverlayMaterial(Overlay);
			if (bUseCustomDepth)
			{
				Mesh->SetRenderCustomDepth(true);
				Mesh->SetCustomDepthStencilValue(StencilValue);
			}
		}
	}
}

void FPWTargetHighlighter::Clear()
{
	for (const TPair<TWeakObjectPtr<AActor>, TArray<FPWSavedMesh>>& Pair : Highlighted)
		Restore(Pair.Value);
	Highlighted.Reset();
}

void FPWTargetHighlighter::Restore(const TArray<FPWSavedMesh>& Saved)
{
	for (const FPWSavedMesh& Entry : Saved)
	{
		UMeshComponent* Mesh = Entry.Mesh.Get();
		if (!Mesh)
			continue;

		Mesh->SetOverlayMaterial(Entry.Overlay.Get());
		Mesh->SetRenderCustomDepth(Entry.bCustomDepth);
		Mesh->SetCustomDepthStencilValue(Entry.Stencil);
	}
}

UMaterialInterface* FPWTargetHighlighter::MakeTint(UObject* Outer, const FLinearColor& Color)
{
	UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineDebugMaterials/M_SimpleUnlitTranslucent.M_SimpleUnlitTranslucent"));
	if (!Base)
		return nullptr;

	UMaterialInstanceDynamic* Dynamic = UMaterialInstanceDynamic::Create(Base, Outer);
	Dynamic->SetVectorParameterValue(TEXT("Color"), Color);
	return Dynamic;
}
