// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "PWNiagaraEntry.generated.h"

class UNiagaraSystem;

// A float User parameter and the value one play sets it to.
USTRUCT(BlueprintType)
struct PWGASCORE_API FPWNiagaraFloat
{
	GENERATED_BODY()

	UPROPERTY()
	FName Name;

	UPROPERTY()
	float Value = 0.f;
};

// One play of a VFX entry, as every machine receives it: the system, where it goes and the size and values it was given.
USTRUCT(BlueprintType)
struct PWGASCORE_API FPWNiagaraEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UNiagaraSystem> NiagaraSystem = nullptr;

	// Follows the caster, at the socket when its meshes have one.
	UPROPERTY()
	bool bAttachToActor = false;

	UPROPERTY()
	FName AttachSocketName = NAME_None;

	// Offset from the socket or caster when attached.
	UPROPERTY()
	FVector RelativeOffset = FVector::ZeroVector;

	UPROPERTY()
	FRotator RelativeRotation = FRotator::ZeroRotator;

	UPROPERTY()
	FVector Scale = FVector::OneVector;

	UPROPERTY()
	bool bLooping = false;

	// Where it plays when not attached; its scale is ignored in favour of Scale.
	UPROPERTY()
	FTransform WorldTransform = FTransform::Identity;

	UPROPERTY()
	TArray<FPWNiagaraFloat> FloatParameters;

	UPROPERTY()
	int32 UniqueId = INDEX_NONE;
};
