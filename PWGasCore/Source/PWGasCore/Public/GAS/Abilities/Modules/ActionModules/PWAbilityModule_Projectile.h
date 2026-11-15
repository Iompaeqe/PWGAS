// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWAbilityModule_Actor.h"
#include "PWActionModule.h"
#include "Targeting/Data/PWTargetingData.h"
#include "PWAbilityModule_Projectile.generated.h"

UCLASS(BlueprintType)
class PWGASCORE_API UPWProjectileSpellParams : public UPWActorSpellParams
{
	GENERATED_BODY()

public:
	// Launch speed in units per second, copied from the module's projectile config.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float InitialSpeed = 1200.f;

	// Gravity applied to the projectile; 0 uses the projectile's own setting.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float GravityZOverride = 0.f;

	// Distance the projectile is meant to travel; 0 leaves it to the actor.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Range = 0.f;
};

// Spawns a projectile launched with the speed and gravity of its config, which also draw the precast path.
// Graphs change them for one cast with Set Projectile Config or Set Projectile Speed; every activation goes back to the configured ones.
UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWAbilityModule_Projectile : public UPWAbilityModule_Actor
{
	GENERATED_BODY()

public:
	// The configured speed and gravity of the projectile, also used to draw its precast path. Every activation starts from it.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FPWProjectileSimConfig ProjectileConfig;

	UFUNCTION(BlueprintCallable, Category="Module")
	virtual AActor* SpawnProjectileWithTargetingResult(const FPWTargetingResult& Target);
	UFUNCTION(BlueprintCallable, Category="Module")
	virtual void SpawnProjectile();

	// The config in use: what this cast set, otherwise the configured one.
	UFUNCTION(BlueprintPure, Category="Module")
	const FPWProjectileSimConfig& GetProjectileConfig() const;

	// Uses this config for the rest of the cast, and for the precast path if one is drawing. The next activation goes back to the configured one.
	UFUNCTION(BlueprintCallable, Category="Module")
	void SetProjectileConfig(const FPWProjectileSimConfig& NewConfig);

	// The launch speed in use at the ability's level: what this cast set, otherwise the configured one.
	UFUNCTION(BlueprintPure, Category="Module")
	float GetProjectileSpeed() const;

	// The configured launch speed at the ability's level, whatever this cast set.
	UFUNCTION(BlueprintPure, Category="Module")
	float GetBaseProjectileSpeed() const;

	// The launch speed in use, read at any level.
	UFUNCTION(BlueprintPure, Category="Module")
	float GetProjectileSpeedAtLevel(int32 Level) const;

	// The configured launch speed at any level.
	UFUNCTION(BlueprintPure, Category="Module")
	float GetBaseProjectileSpeedAtLevel(int32 Level) const;

	// Launches at this speed at every level for the rest of the cast. The next activation goes back to the configured one.
	UFUNCTION(BlueprintCallable, Category="Module")
	void SetProjectileSpeed(float NewSpeed);

	// Uses this value and curve for the rest of the cast, so it still scales with level. The next activation goes back to the configured one.
	UFUNCTION(BlueprintCallable, Category="Module")
	void SetProjectileSpeedCurve(const FScalableFloat& NewSpeed);

	virtual void ResetCurrentValues() override;

protected:
	virtual UPWSpellParams* MakeSpellParams_Implementation(const FPWTargetingResult& Target) override;
	virtual void DebugSpawn(const FPWTargetingResult& Target, const FTransform& SpawnTM, AActor* Spawned) const override;

private:
	// A property so a curve table this cast set stays loaded.
	UPROPERTY(Transient)
	FPWProjectileSimConfig CurrentConfig;
	bool bHasCurrentConfig = false;
};
