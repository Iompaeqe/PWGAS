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
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float InitialSpeed = 1200.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float GravityZOverride = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ProjectileRadius = 8.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Range = 0.f;
};

UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWAbilityModule_Projectile : public UPWAbilityModule_Actor
{
	GENERATED_BODY()

public:
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
