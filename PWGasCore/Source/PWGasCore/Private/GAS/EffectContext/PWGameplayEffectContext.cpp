// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "GAS/EffectContext/PWGameplayEffectContext.h"


UScriptStruct* FPWGameplayEffectContext::GetScriptStruct() const
{
	return StaticStruct();
}

FPWGameplayEffectContext* FPWGameplayEffectContext::Duplicate() const
{
	FPWGameplayEffectContext* NewContext = new FPWGameplayEffectContext();
	*NewContext = *this;

	if (GetHitResult())
	{
		NewContext->AddHitResult(*GetHitResult(), true);
	}
	return NewContext;
}

bool FPWGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	Super::NetSerialize(Ar, Map, bOutSuccess);

	uint32 RepBits = 0;
	if (Ar.IsSaving())
	{
		if (bHasSurfaceHit) RepBits |= 1 << 0;
	}
	Ar.SerializeBits(&RepBits, 1);

	if (RepBits & (1 << 0))
	{
		bHasSurfaceHit = true;
		CachedHit.NetSerialize(Ar, Map, bOutSuccess);
	}
	else
	{
		bHasSurfaceHit = false;
	}
	bOutSuccess = true;
	return true;
}