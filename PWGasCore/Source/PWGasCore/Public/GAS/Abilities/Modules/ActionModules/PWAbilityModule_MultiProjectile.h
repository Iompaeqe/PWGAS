// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWAbilityModule_Projectile.h"
#include "PWAbilityModule_MultiProjectile.generated.h"

UENUM(BlueprintType)
enum class EProjectileSpreadPattern : uint8
{
	LeftToRight UMETA(DisplayName="Left → Right"),
	RightToLeft UMETA(DisplayName="Right → Left"),
	CenterToSides UMETA(DisplayName="Center → Delay → Sides"),
	SidesToCenter UMETA(DisplayName="Sides → Delay → Center"),
	CenterOut UMETA(DisplayName="Center → Delay → Outwards (paired sides)"),
	OutIn UMETA(DisplayName="Outwards → Delay → Center (paired sides)"),
	CenterFanOutRight UMETA(DisplayName="Center → Right → Left (delayed fan)"),
	CenterFanOutLeft UMETA(DisplayName="Center → Left → Right (delayed fan)"),
	OutFanInRight UMETA(DisplayName="OutRight → Inward (delayed fan)"),
	OutFanInLeft UMETA(DisplayName="OutLeft → Inward (delayed fan)"),
	Random2D UMETA(DisplayName="Random 2D"),
	Random3D UMETA(DisplayName="Random 3D (with pitch)")
};

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWAbilityModule_MultiProjectile : public UPWAbilityModule_Projectile
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 ProjectileCount = 1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SpreadAngle = 20.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SpreadDelay = 0.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EProjectileSpreadPattern SpreadPattern = EProjectileSpreadPattern::LeftToRight;

	UFUNCTION(BlueprintCallable, Category="Module")
	void SpreadSpawnProjectiles(const FPWTargetingResult& Target);
	UFUNCTION(BlueprintCallable, Category="Module")
	void SpreadSpawnNextStep(const FPWTargetingResult& Target);

	virtual AActor* SpawnActorWithTargetingResult(const FPWTargetingResult& Target) override;
	virtual void GetPreviewAimPoints(const FPWTargetingResult& Target, TArray<FVector>& OutAimPoints) const override;

private:
	int32 CurrentSpreadStep = 0;
	TArray<TArray<int32>> SpreadSteps;
	FTimerHandle SpreadTimerHandle;
	FGuid CastId;

protected:
	virtual USpellParams* MakeSpellParams_Implementation(const FPWTargetingResult& Target) override;

	void BuildSpreadSteps();
	FPWTargetingResult MakeSpreadTarget(const FPWTargetingResult& Target, int32 SpreadIndex) const;
	FRotator ComputeSpreadRotation(int32 SpreadIndex) const;
};
