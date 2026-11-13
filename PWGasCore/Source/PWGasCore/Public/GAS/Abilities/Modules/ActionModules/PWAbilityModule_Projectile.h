// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWAbilityModule_Actor.h"
#include "PWActionModule.h"
#include "Targeting/Data/PWTargetingData.h"
#include "PWAbilityModule_Projectile.generated.h"

UCLASS(BlueprintType)
class PWGASCORE_API UProjectileSpellParamsExtension : public UActorSpellParamsExtension
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

UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWAbilityModule_Projectile : public UPWAbilityModule_Actor
{
	GENERATED_BODY()

public:
	// Speed and gravity of the projectile, also used to draw its precast path.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FPWProjectileSimConfig ProjectileConfig;
	
	UFUNCTION(BlueprintCallable, Category="Module")
	virtual AActor* SpawnProjectileWithTargetingResult(const FPWTargetingResult& Target);
	UFUNCTION(BlueprintCallable, Category="Module")
	virtual void SpawnProjectile();

	UFUNCTION(BlueprintPure, Category="Module")
	const FPWProjectileSimConfig& GetProjectileConfig() const;

protected:
	virtual USpellParams* MakeSpellParams_Implementation(const FPWTargetingResult& Target) override;
};
