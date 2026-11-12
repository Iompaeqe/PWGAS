// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWSpreadPattern.h"
#include "PWSpreadPattern_Fan.generated.h"

UENUM(BlueprintType)
enum class EPWSpreadOrder : uint8
{
	LeftToRight,
	RightToLeft,
	CenterOut,
	EdgesIn
};

UENUM(BlueprintType)
enum class EPWSpreadGrouping : uint8
{
	OneByOne,
	Pairs UMETA(DisplayName="Pairs (Both Sides Together)"),
	CenterAndSides UMETA(DisplayName="Center And Sides (All Sides Together)")
};

UCLASS(meta=(DisplayName="Fan"))
class PWGASCORE_API UPWSpreadPattern_Fan : public UPWSpreadPattern
{
	GENERATED_BODY()

public:
	// Order in which the projectiles of the fan are fired.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
	EPWSpreadOrder Order = EPWSpreadOrder::LeftToRight;

	// Which projectiles share a step when firing from or towards the center.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern", meta=(EditCondition="Order == EPWSpreadOrder::CenterOut || Order == EPWSpreadOrder::EdgesIn", EditConditionHides))
	EPWSpreadGrouping Grouping = EPWSpreadGrouping::OneByOne;

	// Of two projectiles at the same distance from the center, the right one fires first.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern", meta=(EditCondition="(Order == EPWSpreadOrder::CenterOut || Order == EPWSpreadOrder::EdgesIn) && Grouping == EPWSpreadGrouping::OneByOne", EditConditionHides))
	bool bRightFirst = true;

	virtual TArray<FPWSpreadShot> BuildShots_Implementation(int32 Count, float SpreadAngle) const override;
};
