// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "Targeting/Data/PWTargetingData.h"
#include "Targeting/Types/PWTargetPolicyBase.h"
#include "UObject/Object.h"
#include "CursorSingleTargeting.generated.h"

UCLASS()
class PWGASCORE_API UCursorSingleTargeting : public UPWTargetingSource
{
	GENERATED_BODY()
public:
	virtual bool Sample_Implementation(APlayerController* PC, FPWTargetingResult& Out) const override
	{
		if (!PC) return false;

		FHitResult Hit;
		PC->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery1, false, Hit);
		if (!Hit.bBlockingHit)
			return false;

		Out.Location = Hit.ImpactPoint;
		Out.Hit = Hit;
		Out.bHasHit = true;

		const APawn* P = PC->GetPawn();
		if (P)
			Out.Rotation = (Hit.ImpactPoint - P->GetActorLocation()).Rotation();

#if WITH_EDITOR
		DrawDebugSphere(PC->GetWorld(), Hit.ImpactPoint, 20.f, 12, FColor::Red, false, 0.2f);
#endif
		return true;
	}
};