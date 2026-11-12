// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWGasCore/Public/Targeting/Types/PWTargetPolicyBase.h"
#include "UObject/Interface.h"
#include "PWPrecastTypes.generated.h"

// UINTERFACE(BlueprintType)
// class UPWPrecastVisualizer : public UInterface { GENERATED_BODY() };
// class IPWPrecastVisualizer
// {
// 	GENERATED_BODY()
// public:
// 	UFUNCTION(BlueprintNativeEvent, BlueprintCallable) void Ensure(class APlayerController* PC, float PreviewRadius, FName OriginSocket);
// 	UFUNCTION(BlueprintNativeEvent, BlueprintCallable) void Update(const FPWPrecastVisualUpdate& Data);
// 	UFUNCTION(BlueprintNativeEvent, BlueprintCallable) void Hide();
// };

UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWPrecastVisualizer : public UPWTargetPolicyBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Ensure(class APlayerController* PC, float PreviewRadius, FName OriginSocket);
	virtual void Ensure_Implementation(class APlayerController* PC, float PreviewRadius, FName OriginSocket);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Update(const FPWPrecastVisualUpdate& Data);
	virtual void Update_Implementation(const FPWPrecastVisualUpdate& Data);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Hide();
	virtual void Hide_Implementation();
};
