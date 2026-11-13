// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "Ability/PWSpellCollision.h"
#include "PWAuraSphereComponent.generated.h"

// The aura's volume. Marked so projectiles fly through it instead of exploding on its edge.
UCLASS()
class PWGASCORE_API UPWAuraSphereComponent : public USphereComponent, public IPWDontTriggerCollision
{
	GENERATED_BODY()
};
