// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PWModuleFunctions.generated.h"

class UProjectileSpellParamsExtension;
class UActorSpellParamsExtension;
class UEffectSpellParamsExtension;
class USpellParams;
UCLASS()
class PWGASCORE_API UPWModuleFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="GASCore|SpellParams")
	static UEffectSpellParamsExtension* GetEffectParams(const USpellParams* Params);
	UFUNCTION(BlueprintPure, Category="GASCore|SpellParams")
	static UProjectileSpellParamsExtension* GetProjectileParams(const USpellParams* Params);
	UFUNCTION(BlueprintPure, Category="GASCore|SpellParams")
	static UActorSpellParamsExtension* GetActorParams(const USpellParams* Params);
	UFUNCTION(BlueprintPure, BlueprintPure, Category="Accessors")
	static float GetRange(const USpellParams* Params);
	UFUNCTION(BlueprintPure, BlueprintPure, Category="Accessors")
	static float GetRadius(const USpellParams* Params);
};
