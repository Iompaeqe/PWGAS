// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "Targeting/Data/PWTargetingData.h"
#include "PWGasCore/Public/Targeting/Types/PWTargetPolicyBase.h"
#include "UObject/Object.h"
#include "CursorAOETargeting.generated.h"

UCLASS()
class PWGASCORE_API UCursorAOETargeting : public UPWTargetingSource
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
		Out.Rotation = FRotator::ZeroRotator;
		Out.Hit = Hit;
		Out.bHasHit = true;

#if WITH_EDITOR
		DrawDebugSphere(PC->GetWorld(), Hit.ImpactPoint, 15.f, 10, FColor::Green, false, 0.2f);
#endif
		return true;
	}
};