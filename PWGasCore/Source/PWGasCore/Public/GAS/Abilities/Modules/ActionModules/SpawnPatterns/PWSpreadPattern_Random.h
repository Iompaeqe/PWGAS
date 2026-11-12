// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWSpreadPattern.h"
#include "PWSpreadPattern_Random.generated.h"

UCLASS(meta=(DisplayName="Random"))
class PWGASCORE_API UPWSpreadPattern_Random : public UPWSpreadPattern
{
	GENERATED_BODY()

public:
	// Also spreads the projectiles up and down, within the spread angle.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
	bool bRandomPitch = false;

	virtual TArray<FPWSpreadShot> BuildShots_Implementation(int32 Count, float SpreadAngle) const override;
};
