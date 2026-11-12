// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWControlModule.h"
#include "PWAbilityModule_Niagara.generated.h"


struct FPWNiagaraEntry;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWAbilityModule_Niagara : public UPWControlModule
{
	GENERATED_BODY()

public:
	/** Per-entry Niagara config*/
	UPROPERTY(EditAnywhere, Category="Niagara")
	TArray<FPWNiagaraEntry> NiagaraEntries;

	/** Play entry at explicit world transform. */
	UFUNCTION(BlueprintCallable, Category="Niagara")
	UNiagaraComponent* PlayAtLocation(int32 Index, const FTransform& WorldTransform);

	/** Play entry on/around a target actor. */
	UFUNCTION(BlueprintCallable, Category="Niagara")
	UNiagaraComponent* PlayOnActor(int32 Index, AActor* TargetActor);

	/** Stop looping VFX for a given entry index. */
	UFUNCTION(BlueprintCallable, Category="Niagara")
	void Stop(int32 Index);

	/** Stop all looping VFX for this module, with a single multicast. */
	UFUNCTION(BlueprintCallable, Category="Niagara")
	void StopAll();

protected:
	/** For active entries: Index -> UniqueId. */
	UPROPERTY(Transient)
	TMap<int32, int32> ActiveIndexToId;

	int32 ComputeUniqueId(int32 Index) const;
	FPWNiagaraEntry* GetEntryMutable(int32 Index);
	UNiagaraComponent* InternalPlay(int32 Index,const FTransform* WorldOverride,AActor* TargetActor);
	void InternalStopByIds(const TArray<int32>& Ids) const;
	
};
