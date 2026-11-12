// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWSpawnPattern.h"
#include "PWSpawnPattern_Rings.generated.h"

UENUM(BlueprintType)
enum class EPWRingDistribution : uint8
{
	SamePerRing,
	ByRingSize UMETA(DisplayName="By Ring Size (More On Outer Rings)"),
	Custom UMETA(DisplayName="Custom (Count Per Ring)")
};

UCLASS(meta=(DisplayName="Rings"))
class PWGASCORE_API UPWSpawnPattern_Rings : public UPWSpawnPattern
{
	GENERATED_BODY()

public:
	// How the actors are divided between the rings.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
	EPWRingDistribution Distribution = EPWRingDistribution::SamePerRing;

	// Number of actors on every ring; the ring count follows from the total.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern", meta=(ClampMin=1, EditCondition="Distribution == EPWRingDistribution::SamePerRing", EditConditionHides))
	int32 ActorsPerRing = 2;

	// Number of rings; outer rings receive more actors than inner ones.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern", meta=(ClampMin=1, EditCondition="Distribution == EPWRingDistribution::ByRingSize", EditConditionHides))
	int32 RingCount = 3;

	// From the outer ring to the inner one. Actors left over go to the outer ring.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern", meta=(EditCondition="Distribution == EPWRingDistribution::Custom", EditConditionHides))
	TArray<int32> RingCounts;

	// Puts one actor in the middle before filling the rings.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
	bool bCenterActor = true;

	// Degrees each ring is turned relative to the one outside it.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
	float TwistPerRing = 0.f;

	// Spawns from the center outwards; off spawns from the outer ring inwards.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
	bool bCenterOut = false;

	virtual TArray<FVector> BuildPositions_Implementation(int32 Count, float Radius) const override;

private:
	TArray<int32> GetRingCounts(int32 Count) const;
};
