// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "PWNiagaraEntry.generated.h"

class UNiagaraSystem;

USTRUCT(BlueprintType)
struct PWGASCORE_API FPWNiagaraEntry
{
	GENERATED_BODY()

	/** Niagara system asset. */
	UPROPERTY(EditAnywhere, Category="Niagara")
	TObjectPtr<UNiagaraSystem> NiagaraSystem = nullptr;

	/** Attach to avatar instead of using world transform. */
	UPROPERTY(EditAnywhere, Category="Niagara")
	bool bAttachToActor = false;

	/** Attach socket/bone on avatar. */
	UPROPERTY(EditAnywhere, Category="Niagara", meta=(EditCondition="bAttachToActor"))
	FName AttachSocketName = NAME_None;

	/** Relative offset from attach point / avatar location. */
	UPROPERTY(EditAnywhere, Category="Niagara", meta=(EditCondition="bAttachToActor"))
	FVector RelativeOffset = FVector::ZeroVector;

	/** Treat this as looping and manage via UniqueId. */
	UPROPERTY(EditAnywhere, Category="Niagara")
	bool bLooping = false;

	/** Filled by the module when needed. */
	UPROPERTY()
	FTransform WorldTransform = FTransform::Identity;

	/** Generated unique id. */
	UPROPERTY()
	int32 UniqueId = INDEX_NONE;
};
