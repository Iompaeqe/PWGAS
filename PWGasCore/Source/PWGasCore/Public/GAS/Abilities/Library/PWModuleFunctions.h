// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PWModuleFunctions.generated.h"

class UPWProjectileSpellParams;
class UPWActorSpellParams;
class UPWEffectSpellParams;
class UPWSpellParams;
UCLASS()
class PWGASCORE_API UPWModuleFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="GASCore|SpellParams")
	static UPWEffectSpellParams* GetEffectParams(const UPWSpellParams* Params);
	UFUNCTION(BlueprintPure, Category="GASCore|SpellParams")
	static UPWProjectileSpellParams* GetProjectileParams(const UPWSpellParams* Params);
	UFUNCTION(BlueprintPure, Category="GASCore|SpellParams")
	static UPWActorSpellParams* GetActorParams(const UPWSpellParams* Params);
	UFUNCTION(BlueprintPure, BlueprintPure, Category="Accessors")
	static float GetRange(const UPWSpellParams* Params);
	UFUNCTION(BlueprintPure, BlueprintPure, Category="Accessors")
	static float GetRadius(const UPWSpellParams* Params);
};
