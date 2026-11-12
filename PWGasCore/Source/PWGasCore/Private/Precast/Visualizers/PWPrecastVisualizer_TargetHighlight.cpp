// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "Precast/Visualizers/PWPrecastVisualizer_TargetHighlight.h"

#include "Precast/Data/PWPrecastData.h"

void UPWPrecastVisualizer_TargetHighlight::Ensure_Implementation(APlayerController* PC, float PreviewRadius, FName OriginSocket)
{
}

void UPWPrecastVisualizer_TargetHighlight::Update_Implementation(const FPWPrecastVisualUpdate& Data)
{
	if (!OverlayMaterial && !DefaultMaterial)
		DefaultMaterial = FPWTargetHighlighter::MakeTint(this, HighlightColor);

	Highlighter.Update(Data.Targets, OverlayMaterial ? OverlayMaterial.Get() : DefaultMaterial.Get(), bUseCustomDepth, StencilValue);
}

void UPWPrecastVisualizer_TargetHighlight::Hide_Implementation()
{
	Highlighter.Clear();
}
