// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "Precast/Visualizers/PWPrecastVisualizer.h"
#include "Precast/Visualizers/PWPrecastTargetHighlighter.h"
#include "PWPrecastVisualizer_ProjectilePath.generated.h"

class UStaticMesh;
class UMaterialInterface;
class UInstancedStaticMeshComponent;

UENUM(BlueprintType)
enum class EPWPathStyle : uint8
{
	Dots,
	Line
};

// Draws each projectile's flight path and landing spot with meshes and materials of your choice.
UCLASS(meta=(DisplayName="Projectile Path"))
class PWGASCORE_API UPWPrecastVisualizer_ProjectilePath : public UPWPrecastVisualizer
{
	GENERATED_BODY()

public:
	// Dots places a mesh every Spacing units; Line stretches a mesh along every piece of the path.
	UPROPERTY(EditDefaultsOnly, Category="Path")
	EPWPathStyle Style = EPWPathStyle::Dots;

	// Mesh repeated along the path; empty uses a sphere for dots and a cylinder for a line.
	UPROPERTY(EditDefaultsOnly, Category="Path")
	TObjectPtr<UStaticMesh> PathMesh;

	// Optional. Replaces the default see-through material on the path.
	UPROPERTY(EditDefaultsOnly, Category="Path")
	TObjectPtr<UMaterialInterface> PathMaterial;

	// Tint of the path when no material is set.
	UPROPERTY(EditDefaultsOnly, Category="Path")
	FLinearColor PathColor = FLinearColor(0.2f, 0.8f, 1.f, 0.6f);

	// Width of the path in units, for a mesh that is 100 units wide like the basic engine shapes.
	UPROPERTY(EditDefaultsOnly, Category="Path", meta=(ClampMin="0.1"))
	float Thickness = 6.f;

	// Distance between dots.
	UPROPERTY(EditDefaultsOnly, Category="Path", meta=(ClampMin="1.0", EditCondition="Style == EPWPathStyle::Dots", EditConditionHides))
	float Spacing = 40.f;

	// Shows a marker where each projectile lands.
	UPROPERTY(EditDefaultsOnly, Category="Impact")
	bool bShowImpact = true;

	// Mesh shown at the landing spot, scaled to the ability's radius; empty uses a sphere.
	UPROPERTY(EditDefaultsOnly, Category="Impact", meta=(EditCondition="bShowImpact"))
	TObjectPtr<UStaticMesh> ImpactMesh;

	// Optional. Replaces the default see-through material on the landing marker.
	UPROPERTY(EditDefaultsOnly, Category="Impact", meta=(EditCondition="bShowImpact"))
	TObjectPtr<UMaterialInterface> ImpactMaterial;

	// Tint of the landing marker when no material is set.
	UPROPERTY(EditDefaultsOnly, Category="Impact", meta=(EditCondition="bShowImpact"))
	FLinearColor ImpactColor = FLinearColor(1.f, 0.3f, 0.1f, 0.35f);

	// Radius of the landing marker when the ability has no radius of its own.
	UPROPERTY(EditDefaultsOnly, Category="Impact", meta=(ClampMin="1.0", EditCondition="bShowImpact"))
	float ImpactSize = 30.f;

	// Tints the actors the ability would affect, as found by its target resolver.
	UPROPERTY(EditDefaultsOnly, Category="Targets")
	bool bHighlightTargets = true;

	// Optional. Overlay material drawn over the targets; empty uses a see-through tint.
	UPROPERTY(EditDefaultsOnly, Category="Targets", meta=(EditCondition="bHighlightTargets"))
	TObjectPtr<UMaterialInterface> HighlightMaterial;

	// Tint of the targets when no highlight material is set.
	UPROPERTY(EditDefaultsOnly, Category="Targets", meta=(EditCondition="bHighlightTargets"))
	FLinearColor HighlightColor = FLinearColor(1.f, 0.25f, 0.1f, 0.35f);

	virtual void Ensure_Implementation(APlayerController* PC, float PreviewRadius, FName OriginSocket) override;
	virtual void Update_Implementation(const FPWPrecastVisualUpdate& Data) override;
	virtual void Hide_Implementation() override;

private:
	UInstancedStaticMeshComponent* AddMeshes(AActor* Owner, UStaticMesh* Mesh, UMaterialInterface* Material, const FLinearColor& Color) const;
	void BuildPathTransforms(const TArray<FVector>& Points, TArray<FTransform>& OutTransforms) const;

	TWeakObjectPtr<APlayerController> CachedPC;
	TWeakObjectPtr<AActor> Holder;
	TWeakObjectPtr<UInstancedStaticMeshComponent> PathMeshes;
	TWeakObjectPtr<UInstancedStaticMeshComponent> ImpactMeshes;
	float CachedRadius = 0.f;

	FPWTargetHighlighter Highlighter;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> DefaultHighlight;
};
