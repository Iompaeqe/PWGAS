// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Modules/PWAbilityModule.h"
#include "ScalableFloat.h"
#include "PWDataModule.generated.h"

class UPWSpellParams;

// A module value a graph can change for one cast. Until it does, and again from each activation, it reads the configured value.
USTRUCT()
struct PWGASCORE_API FPWModuleValue
{
	GENERATED_BODY()

	float Get(const FScalableFloat& Configured, const float Level) const { return (bSet ? Current : Configured).GetValueAtLevel(Level); }
	void Set(const FScalableFloat& Value) { Current = Value; bSet = true; }
	void Reset() { Current = FScalableFloat(); bSet = false; }

private:
	// A property so the curve table this cast set stays loaded.
	UPROPERTY()
	FScalableFloat Current;
	bool bSet = false;
};

UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWDataModule : public UPWAbilityModule
{
	GENERATED_BODY()

public:
	virtual void ExtendParams(UPWSpellParams* Params) const PURE_VIRTUAL(UPWActionModule::ExtendParams, );

	// The value this module owns for a params key, the same one it writes in ExtendParams. Lets policies read it without spell params.
	virtual bool GetValue(FName Key, float& Out) const { return false; }
};
