// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "GameplayEffectTypes.h"
#include "ScalableFloat.h"
#include "PWGameplayEffectContext.generated.h"

enum class ERelevancy : uint8;
class UGameplayEffect;

USTRUCT(BlueprintType)
struct PWGASCORE_API FPWGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

public:
	virtual UScriptStruct* GetScriptStruct() const override;
	virtual FPWGameplayEffectContext* Duplicate() const override;
	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;

protected:
	UPROPERTY()
	bool bHasSurfaceHit = false;

	UPROPERTY()
	FHitResult CachedHit;

public:
	bool HasSurfaceHit() const { return bHasSurfaceHit; }
	const FHitResult* GetCachedHit() const { return bHasSurfaceHit ? &CachedHit : nullptr; }
	void SetCachedHit(const FHitResult& Hit) { CachedHit = Hit; bHasSurfaceHit = true; }
};

template <>
struct TStructOpsTypeTraits<FPWGameplayEffectContext> : TStructOpsTypeTraitsBase2<FPWGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};
