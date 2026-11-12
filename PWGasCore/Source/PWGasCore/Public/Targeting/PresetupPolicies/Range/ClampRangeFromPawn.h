// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "Targeting/Data/PWTargetingData.h"
#include "PWGasCore/Public/Targeting/Types/PWTargetPolicyBase.h"
#include "UObject/Object.h"
#include "ClampRangeFromPawn.generated.h"

UCLASS()
class PWGASCORE_API UClampRangeFromPawn : public UPWRangePolicy
{
	GENERATED_BODY()

public:
	virtual void Apply_Implementation(const AActor* Actor, const float MaxRange, FPWTargetingResult& InOut) const override
	{
		if (!Actor || MaxRange <= 0.f) return;

		const FVector Origin = Actor->GetActorLocation();
		const FVector Target = InOut.Location;
		const FVector Dir = (Target - Origin).GetSafeNormal();

		if (const float Dist = FVector::Dist(Origin, Target); Dist > MaxRange)
			InOut.Location = Origin + Dir * MaxRange;
		InOut.Rotation = (InOut.Location - Actor->GetActorLocation()).Rotation();
	}
};
