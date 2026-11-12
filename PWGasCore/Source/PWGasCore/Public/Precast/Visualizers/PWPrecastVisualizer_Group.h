// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "Precast/Visualizers/PWPrecastVisualizer.h"
#include "PWPrecastVisualizer_Group.generated.h"

// Runs several visualizers together.
UCLASS(meta=(DisplayName="Group"))
class PWGASCORE_API UPWPrecastVisualizer_Group : public UPWPrecastVisualizer
{
	GENERATED_BODY()

public:
	// Visualizers shown together, for example a path and a ground decal.
	UPROPERTY(EditDefaultsOnly, Instanced, Category="Group")
	TArray<TObjectPtr<UPWPrecastVisualizer>> Visualizers;

	virtual void Ensure_Implementation(APlayerController* PC, float PreviewRadius, FName OriginSocket) override;
	virtual void Update_Implementation(const FPWPrecastVisualUpdate& Data) override;
	virtual void Hide_Implementation() override;
};
