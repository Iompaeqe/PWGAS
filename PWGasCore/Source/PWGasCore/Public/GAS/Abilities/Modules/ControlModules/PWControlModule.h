// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Modules/PWAbilityModule.h"
#include "PWControlModule.generated.h"

UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWControlModule : public UPWAbilityModule
{
	GENERATED_BODY()

public:
	// Lets a control module add to the spell params given to spawned actors.
	virtual void ExtendParams(class USpellParams* Params) const {}
};
