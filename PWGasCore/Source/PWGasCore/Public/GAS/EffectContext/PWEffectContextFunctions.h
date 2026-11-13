// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWGameplayEffectContext.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PWEffectContextFunctions.generated.h"

struct FGameplayEffectContextHandle;
UCLASS()
class PWGASCORE_API UPWEffectContextFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="GASCore|EffectContext")
	static bool IsValid(const FGameplayEffectContextHandle& Handle){ return Handle.IsValid(); }

	// Null when the handle holds a context of another type.
	static FPWGameplayEffectContext* GetPWContext(const FGameplayEffectContextHandle& Handle);

	UFUNCTION(BlueprintCallable, Category="GASCore|EffectContext")
	static void CopyHitResultToContext(UPARAM(ref) FGameplayEffectContextHandle& Handle, const FHitResult& Hit);

	UFUNCTION(BlueprintPure, Category="GASCore|EffectContext")
	static bool GetHitResult(const FGameplayEffectContextHandle& Handle, FHitResult& OutHit);
};