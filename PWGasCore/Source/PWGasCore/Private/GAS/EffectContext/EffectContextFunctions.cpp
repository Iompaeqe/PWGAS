// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "GAS/EffectContext/EffectContextFunctions.h"


void UEffectContextFunctions::CopyHitResultToContext(FGameplayEffectContextHandle& Handle, const FHitResult& Hit)
{
	if (FPWGameplayEffectContext* Ctx = static_cast<FPWGameplayEffectContext*>(Handle.Get()))
	{
		Ctx->SetCachedHit(Hit);
	}
}

inline bool UEffectContextFunctions::GetHitResult(const FGameplayEffectContextHandle& Handle, FHitResult& OutHit)
{
	if (const FPWGameplayEffectContext* Ctx = static_cast<const FPWGameplayEffectContext*>(Handle.Get()))
	{
		if (const FHitResult* Hit = Ctx->GetCachedHit())
		{
			OutHit = *Hit;
			return true;
		}
	}
	return false;
}