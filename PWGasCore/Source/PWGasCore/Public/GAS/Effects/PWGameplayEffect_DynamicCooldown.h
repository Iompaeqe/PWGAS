// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "PWGameplayEffect_DynamicCooldown.generated.h"

// The cooldown every PW ability uses unless it names another: lasts the ability's Cooldown seconds and carries the ability's cooldown tag.
UCLASS()
class PWGASCORE_API UPWGameplayEffect_DynamicCooldown : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UPWGameplayEffect_DynamicCooldown();
};
