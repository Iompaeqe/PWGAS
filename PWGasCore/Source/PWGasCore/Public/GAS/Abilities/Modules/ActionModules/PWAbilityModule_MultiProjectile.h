// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWAbilityModule_Projectile.h"
#include "SpawnPatterns/PWSpreadPattern.h"
#include "PWAbilityModule_MultiProjectile.generated.h"

UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWAbilityModule_MultiProjectile : public UPWAbilityModule_Projectile
{
	GENERATED_BODY()

public:
	// How many projectiles are fired per cast.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 ProjectileCount = 1;
	// Total angle in degrees the projectiles fan across.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SpreadAngle = 20.f;
	// Seconds between the steps of the spread pattern; 0 fires everything at once.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SpreadDelay = 0.f;
	// Where each projectile is aimed and when it fires; empty uses a left to right fan.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category="Pattern")
	TObjectPtr<UPWSpreadPattern> Pattern;

	UFUNCTION(BlueprintCallable, Category="Module")
	void SpreadSpawnProjectiles(const FPWTargetingResult& Target);
	UFUNCTION(BlueprintCallable, Category="Module")
	void SpreadSpawnNextStep(const FPWTargetingResult& Target);

	virtual AActor* SpawnActorWithTargetingResult(const FPWTargetingResult& Target) override;
	virtual void GetPreviewAimPoints(const FPWTargetingResult& Target, TArray<FVector>& OutAimPoints) const override;

private:
	int32 CurrentSpreadStep = 0;
	TArray<FPWSpreadShot> Shots;
	TArray<TArray<int32>> SpreadSteps;
	FTimerHandle SpreadTimerHandle;
	FGuid CastId;

protected:
	virtual USpellParams* MakeSpellParams_Implementation(const FPWTargetingResult& Target) override;

	void BuildSpreadSteps();
	FPWTargetingResult MakeSpreadTarget(const FPWTargetingResult& Target, const FRotator& Offset) const;
	TArray<FPWSpreadShot> BuildShots() const;
};
