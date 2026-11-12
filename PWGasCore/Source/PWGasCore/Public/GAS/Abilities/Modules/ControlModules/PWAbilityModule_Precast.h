// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWControlModule.h"
#include "Targeting/Data/PWTargetingData.h"
#include "PWAbilityModule_Precast.generated.h"


class UPWPrecastVisualizer;
class UPWPrecastControllerComponent;

UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWAbilityModule_Precast : public UPWControlModule
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UPWPrecastVisualizer> Visualizer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float PreviewRadius = 150.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float PreviewRange = 9999.f;
	
	UFUNCTION(BlueprintCallable, Category="Module")
	void StartPrecast();

	UFUNCTION(BlueprintCallable, Category="Module")
	void StopPrecast();
	
	UFUNCTION(BlueprintCallable, Category="Module")
	void UpdateProjectileSimConfig(const FPWProjectileSimConfig& Cfg);

	UFUNCTION(BlueprintPure, Category="Module")
	FPWTargetingResult GetPrecastResult() const { return CachedResult; }

	virtual void OnAbilityEnded_Implementation() override;

protected:
	UPROPERTY()
	FPWTargetingResult CachedResult;

	bool bRunning = false;

private:
	UPWPrecastControllerComponent* GetPrecastController() const;
};
