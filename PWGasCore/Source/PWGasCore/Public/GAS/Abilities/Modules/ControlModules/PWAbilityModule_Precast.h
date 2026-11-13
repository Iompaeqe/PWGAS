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
	// What is drawn while aiming: a decal, a projectile path, a ghost actor or several of them.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UPWPrecastVisualizer> Visualizer;

	// Which of the ability's Animation Tags is the montage that releases the projectile; -1 previews from where the hand is now.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Release Pose", meta=(ClampMin="-1"))
	int32 ReleaseAnimationIndex = INDEX_NONE;

	// Time in that montage, in seconds, at which the projectile leaves the hand.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Release Pose", meta=(ClampMin="0.0", EditCondition="ReleaseAnimationIndex >= 0"))
	float ReleaseTime = 0.f;
	
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
