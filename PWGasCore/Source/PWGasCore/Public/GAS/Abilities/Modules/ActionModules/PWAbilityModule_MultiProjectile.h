// copyright mkurtt96

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
	void SpreadSpawnOneProjectile(const FPWTargetingResult& Target);

	virtual AActor* SpawnActorWithTargetingResult(const FPWTargetingResult& Target) override;

private:
	int32 CurrentSpreadIndex = 0;
	TArray<int32> CachedSpreadOrder;
	FTimerHandle SpreadTimerHandle;
	FGuid CastId;
	
protected:
	virtual USpellParams* MakeSpellParams_Implementation(const FPWTargetingResult& Target) override;

	void BuildSpreadOrder();
	FRotator ComputeSpreadRotation(int32 SpreadIndex) const;
	
	void Build_LeftToRight();
	void Build_RightToLeft();
	void Build_CenterOut();
	void Build_OutIn();
	void Build_CenterFanOutRight();
	void Build_CenterFanOutLeft();
	void Build_OutFanInRight();
	void Build_OutFanInLeft();
	void Build_Random(bool bThreeDimensional);
};
