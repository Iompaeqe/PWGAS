// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWSpawnPattern.h"
#include "PWSpawnPatterns.generated.h"

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

UENUM(BlueprintType)
enum class EPWRingDistribution : uint8
{
	SamePerRing,
	ByRingSize UMETA(DisplayName="By Ring Size (More On Outer Rings)"),
	Custom UMETA(DisplayName="Custom (Count Per Ring)")
};

UENUM(BlueprintType)
enum class EPWGridOrder : uint8
{
	RowByRow,
	ColumnByColumn,
	CenterOut,
	EdgesIn
};

UCLASS(meta=(DisplayName="Circle"))
class PWGASCORE_API UPWSpawnPattern_Circle : public UPWSpawnPattern
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
	bool bClockwise = true;

	virtual TArray<FVector> BuildPositions_Implementation(int32 Count, float Radius) const override;
};

UCLASS(meta=(DisplayName="Rings"))
class PWGASCORE_API UPWSpawnPattern_Rings : public UPWSpawnPattern
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
	EPWRingDistribution Distribution = EPWRingDistribution::SamePerRing;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern", meta=(ClampMin=1, EditCondition="Distribution == EPWRingDistribution::SamePerRing", EditConditionHides))
	int32 ActorsPerRing = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern", meta=(ClampMin=1, EditCondition="Distribution == EPWRingDistribution::ByRingSize", EditConditionHides))
	int32 RingCount = 3;

	// From the outer ring to the inner one. Actors left over go to the outer ring.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern", meta=(EditCondition="Distribution == EPWRingDistribution::Custom", EditConditionHides))
	TArray<int32> RingCounts;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
	bool bCenterActor = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
	float TwistPerRing = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
	bool bCenterOut = false;

	virtual TArray<FVector> BuildPositions_Implementation(int32 Count, float Radius) const override;

private:
	TArray<int32> GetRingCounts(int32 Count) const;
};

UCLASS(meta=(DisplayName="Line"))
class PWGASCORE_API UPWSpawnPattern_Line : public UPWSpawnPattern
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
	EPWLineAxis Axis = EPWLineAxis::Across;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
	EPWLineOrder Order = EPWLineOrder::StartToEnd;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
	bool bCentered = true;

	virtual TArray<FVector> BuildPositions_Implementation(int32 Count, float Radius) const override;
};

UCLASS(meta=(DisplayName="Grid"))
class PWGASCORE_API UPWSpawnPattern_Grid : public UPWSpawnPattern
{
	GENERATED_BODY()

public:
	// 0 makes the grid as square as the count allows.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern", meta=(ClampMin=0))
	int32 Rows = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
	EPWGridOrder Order = EPWGridOrder::RowByRow;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern", meta=(EditCondition="Order == EPWGridOrder::RowByRow || Order == EPWGridOrder::ColumnByColumn", EditConditionHides))
	bool bSnake = false;

	virtual TArray<FVector> BuildPositions_Implementation(int32 Count, float Radius) const override;
};

UCLASS(meta=(DisplayName="Random"))
class PWGASCORE_API UPWSpawnPattern_Random : public UPWSpawnPattern
{
	GENERATED_BODY()

public:
	virtual TArray<FVector> BuildPositions_Implementation(int32 Count, float Radius) const override;
};
