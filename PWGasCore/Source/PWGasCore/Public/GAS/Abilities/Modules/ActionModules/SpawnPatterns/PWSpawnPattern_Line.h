// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWSpawnPattern.h"
#include "PWSpawnPattern_Line.generated.h"

UENUM(BlueprintType)
enum class EPWLineAxis : uint8
{
	Across UMETA(DisplayName="Across (Left To Right)"),
	Along UMETA(DisplayName="Along (Back To Front)")
};

UENUM(BlueprintType)
enum class EPWLineOrder : uint8
{
	StartToEnd,
	EndToStart,
	CenterOut,
	EdgesIn
};

UCLASS(meta=(DisplayName="Line"))
class PWGASCORE_API UPWSpawnPattern_Line : public UPWSpawnPattern
{
	GENERATED_BODY()

public:
	// Direction the line runs in, relative to the pattern's forward.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
	EPWLineAxis Axis = EPWLineAxis::Across;

	// Order in which the positions along the line are filled.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
	EPWLineOrder Order = EPWLineOrder::StartToEnd;

	// Centers the line on the target point; off starts it there.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
	bool bCentered = true;

	virtual TArray<FVector> BuildPositions_Implementation(int32 Count, float Radius) const override;
};
