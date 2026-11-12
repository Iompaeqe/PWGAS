// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "Targeting/Data/PWTargetingData.h"
#include "PWGasCore/Public/Targeting/Types/PWTargetPolicyBase.h"
#include "UObject/Object.h"
#include "SphereTargetResolver.generated.h"

/**
 * 
 */
UCLASS()
class PWGASCORE_API USphereTargetResolver : public UPWTargetResolver
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category="Targeting")
	float Radius = 400.f;

	virtual void Initialize_Implementation(UPWModularGameplayAbility* InOwner) override;
	
	virtual void Resolve_Implementation(const AActor* Actor, const FPWTargetingResult& Target, TArray<AActor*>& OutActors) const override;

	virtual void GetRequiredDataModules_Implementation(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules) override;
};
