// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"
#include "Targeting/Data/PWTargetingData.h"
#include "PWGasCore/Public/Targeting/Types/PWTargetPolicyBase.h"
#include "UObject/Object.h"
#include "ScreenCenterTargeting.generated.h"

UCLASS()
class PWGASCORE_API UScreenCenterTargeting : public UPWTargetingSource
{
	GENERATED_BODY()

public:
	virtual bool Sample_Implementation(APlayerController* PC, FPWTargetingResult& Out) const override
	{
		if (!PC || !PC->IsLocalController())
			return false;

		int32 W, H;
		PC->GetViewportSize(W, H);

		FVector World, Dir;
		if (!UGameplayStatics::DeprojectScreenToWorld(PC, FVector2D(W * 0.5f, H * 0.5f), World, Dir))
			return false;

		const FVector Start = World;
		const FVector End = Start + Dir * 100000.f;

		FHitResult Hit;
		PC->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility);

		const FVector Aim = Hit.bBlockingHit ? Hit.ImpactPoint : End;

		if (!Hit.bBlockingHit)
		{
			Hit.TraceStart  = Start;
			Hit.TraceEnd    = End;
			Hit.Location    = Aim;        // important for replication
			Hit.ImpactPoint = Aim;        // also useful if you ever read ImpactPoint
		}
		
		Out.Location = Aim;
		Out.Rotation = Dir.Rotation();
		Out.Hit = Hit;
		Out.bHasHit = Hit.bBlockingHit;

		return true;
	}
};
