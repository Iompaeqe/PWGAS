// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "Precast/Visualizers/PWPrecastVisualizer.h"
#include "Precast/Visualizers/PWPrecastTargetHighlighter.h"
#include "PWPrecastVisualizer_TargetHighlight.generated.h"

class UMaterialInterface;

// Highlights every actor the ability would affect while it is being aimed.
UCLASS(meta=(DisplayName="Target Highlight"))
class PWGASCORE_API UPWPrecastVisualizer_TargetHighlight : public UPWPrecastVisualizer
{
	GENERATED_BODY()

public:
	// Optional. Overlay material drawn over the target's meshes; empty uses a see-through tint.
	UPROPERTY(EditDefaultsOnly, Category="Highlight")
	TObjectPtr<UMaterialInterface> OverlayMaterial;

	// Tint of the highlight when no overlay material is set.
	UPROPERTY(EditDefaultsOnly, Category="Highlight")
	FLinearColor HighlightColor = FLinearColor(1.f, 0.25f, 0.1f, 0.35f);

	// Also writes the targets to the custom depth buffer, for games that draw outlines in post process.
	UPROPERTY(EditDefaultsOnly, Category="Highlight")
	bool bUseCustomDepth = false;

	// Stencil value written with custom depth, so an outline material can tell targets apart.
	UPROPERTY(EditDefaultsOnly, Category="Highlight", meta=(ClampMin="0", ClampMax="255", EditCondition="bUseCustomDepth"))
	int32 StencilValue = 1;

	virtual void Ensure_Implementation(APlayerController* PC, float PreviewRadius, FName OriginSocket) override;
	virtual void Update_Implementation(const FPWPrecastVisualUpdate& Data) override;
	virtual void Hide_Implementation() override;

private:
	FPWTargetHighlighter Highlighter;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> DefaultMaterial;
};
