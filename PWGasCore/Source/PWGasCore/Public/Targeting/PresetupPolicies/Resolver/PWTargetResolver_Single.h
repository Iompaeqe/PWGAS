// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "Targeting/Data/PWTargetingData.h"
#include "PWGasCore/Public/Targeting/Types/PWTargetPolicyBase.h"
#include "UObject/Object.h"
#include "PWTargetResolver_Single.generated.h"

UCLASS()
class PWGASCORE_API UPWTargetResolver_Single : public UPWTargetResolver
{
	GENERATED_BODY()

public:
	virtual void Resolve_Implementation(const AActor* Actor, const FPWTargetingResult& Target, TArray<AActor*>& OutActors) const override
	{
		const FHitResult Hit = Target.Hit;
		if (AActor* HitActor = Hit.GetActor())
			OutActors.Add(HitActor);
	}
};
