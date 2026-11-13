// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "GAS/EffectContext/PWEffectContextFunctions.h"


FPWGameplayEffectContext* UPWEffectContextFunctions::GetPWContext(const FGameplayEffectContextHandle& Handle)
{
	FGameplayEffectContext* Context = const_cast<FGameplayEffectContext*>(Handle.Get());
	if (Context && Context->GetScriptStruct()->IsChildOf(FPWGameplayEffectContext::StaticStruct()))
		return static_cast<FPWGameplayEffectContext*>(Context);
	return nullptr;
}

void UPWEffectContextFunctions::CopyHitResultToContext(FGameplayEffectContextHandle& Handle, const FHitResult& Hit)
{
	if (FPWGameplayEffectContext* Ctx = GetPWContext(Handle))
	{
		Ctx->SetCachedHit(Hit);
	}
}

bool UPWEffectContextFunctions::GetHitResult(const FGameplayEffectContextHandle& Handle, FHitResult& OutHit)
{
	if (const FPWGameplayEffectContext* Ctx = GetPWContext(Handle))
	{
		if (const FHitResult* Hit = Ctx->GetCachedHit())
		{
			OutHit = *Hit;
			return true;
		}
	}
	return false;
}