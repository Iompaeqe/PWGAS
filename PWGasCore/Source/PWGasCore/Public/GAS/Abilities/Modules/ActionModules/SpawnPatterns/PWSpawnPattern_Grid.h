// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWSpawnPattern.h"
#include "PWSpawnPattern_Grid.generated.h"

UENUM(BlueprintType)
enum class EPWGridOrder : uint8
{
	RowByRow,
	ColumnByColumn,
	CenterOut,
	EdgesIn
};

UCLASS(meta=(DisplayName="Grid"))
class PWGASCORE_API UPWSpawnPattern_Grid : public UPWSpawnPattern
{
	GENERATED_BODY()

public:
	// 0 makes the grid as square as the count allows.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern", meta=(ClampMin=0))
	int32 Rows = 0;

	// Order in which the cells of the grid are filled.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
	EPWGridOrder Order = EPWGridOrder::RowByRow;

	// Reverses every other row or column so the order zigzags.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern", meta=(EditCondition="Order == EPWGridOrder::RowByRow || Order == EPWGridOrder::ColumnByColumn", EditConditionHides))
	bool bSnake = false;

	virtual TArray<FVector> BuildPositions_Implementation(int32 Count, float Radius) const override;
};
