// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWGasCore/Public/Targeting/Types/PWTargetPolicyBase.h"
#include "PWPrecastVisualizer.generated.h"

// What is drawn while a cast is being aimed. Subclass it in C++ or Blueprint for a custom look.
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWPrecastVisualizer : public UPWTargetPolicyBase
{
	GENERATED_BODY()

public:
	// Called when aiming starts.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Ensure(class APlayerController* PC, float PreviewRadius, FName OriginSocket);
	virtual void Ensure_Implementation(class APlayerController* PC, float PreviewRadius, FName OriginSocket);

	// Called every frame while aiming, with the current target, paths and spawn places.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Update(const FPWPrecastVisualUpdate& Data);
	virtual void Update_Implementation(const FPWPrecastVisualUpdate& Data);

	// Called when aiming ends.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Hide();
	virtual void Hide_Implementation();
};
