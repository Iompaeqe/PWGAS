// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "Targeting/Data/PWTargetingData.h"
#include "PWGasCore/Public/Targeting/Types/PWTargetPolicyBase.h"
#include "UObject/Object.h"
#include "SphereTargetResolver.generated.h"

UCLASS()
class PWGASCORE_API USphereTargetResolver : public UPWTargetResolver
{
	GENERATED_BODY()

public:
	// Object types the sphere looks for; everything but Pawn is there for damageable props.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Targeting")
	TArray<TEnumAsByte<ECollisionChannel>> ObjectChannels = {ECC_Pawn, ECC_WorldDynamic, ECC_PhysicsBody, ECC_Destructible};

	
	virtual void Resolve_Implementation(const AActor* Actor, const FPWTargetingResult& Target, TArray<AActor*>& OutActors) const override;

	virtual void GetRequiredDataModules_Implementation(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules) override;
};
