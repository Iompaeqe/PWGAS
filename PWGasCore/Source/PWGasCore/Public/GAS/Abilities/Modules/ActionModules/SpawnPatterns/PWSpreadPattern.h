// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PWSpreadPattern.generated.h"

USTRUCT(BlueprintType)
struct FPWSpreadShot
{
	GENERATED_BODY()

	// Aim offset from the cast direction. Yaw turns right, pitch turns up.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pattern")
	FRotator Offset = FRotator::ZeroRotator;

	// Shots with the same step fire together. Steps are spaced by the module's spread delay.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pattern")
	int32 Step = 0;
};

UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWSpreadPattern : public UObject
{
	GENERATED_BODY()

public:
	// One shot per projectile, with the direction it is fired in and the step it is fired on.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Pattern")
	TArray<FPWSpreadShot> BuildShots(int32 Count, float SpreadAngle) const;

protected:
	// Yaw of one shot in an even fan across the spread angle.
	static float FanYaw(int32 Index, int32 Count, float SpreadAngle);
};
