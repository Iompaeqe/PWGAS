// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWGameplayEffectContext.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EffectContextFunctions.generated.h"

struct FGameplayEffectContextHandle;
/**
 * 
 */
UCLASS()
class PWGASCORE_API UEffectContextFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="GASCore|EffectContext")
	static bool IsValid(const FGameplayEffectContextHandle& Handle){ return Handle.IsValid(); }

	template <typename T>
	static T* Get(const FGameplayEffectContextHandle& Handle) { return static_cast<T*>(Handle.Get()); }

	UFUNCTION(BlueprintCallable, Category="GASCore|EffectContext")
	static void CopyHitResultToContext(UPARAM(ref) FGameplayEffectContextHandle& Handle, const FHitResult& Hit);

	UFUNCTION(BlueprintPure, Category="GASCore|EffectContext")
	static bool GetHitResult(const FGameplayEffectContextHandle& Handle, FHitResult& OutHit);
};