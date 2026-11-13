// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Modules/PWAbilityModule.h"
#include "PWDataModule.generated.h"

class UPWSpellParams;
UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWDataModule : public UPWAbilityModule
{
	GENERATED_BODY()

public:
	virtual void ExtendParams(UPWSpellParams* Params) const PURE_VIRTUAL(UPWActionModule::ExtendParams, );

	// The value this module owns for a params key, the same one it writes in ExtendParams. Lets policies read it without spell params.
	virtual bool GetValue(FName Key, float& Out) const { return false; }
};
