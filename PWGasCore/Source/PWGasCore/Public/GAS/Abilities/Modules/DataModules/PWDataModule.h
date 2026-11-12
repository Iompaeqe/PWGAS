// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Modules/PWAbilityModule.h"
#include "PWDataModule.generated.h"

class USpellParams;
UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWDataModule : public UPWAbilityModule
{
	GENERATED_BODY()

public:
	virtual void ExtendParams(USpellParams* Params) const PURE_VIRTUAL(UPWActionModule::ExtendParams, );
};
