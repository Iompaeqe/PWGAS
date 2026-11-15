// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWControlModule.h"
#include "GAS/Data/PWNiagaraEntry.h"
#include "PWAbilityModule_VFX.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class UPWAbilityValue;

// When an entry plays without a graph.
UENUM(BlueprintType)
enum class EPWVFXTiming : uint8
{
	// Only when the graph plays it.
	Manual,
	// When the ability activates.
	Activated,
	// When its charge or channel starts.
	Started,
	// On every channel or charge tick.
	Pulse,
	// When the charge is released or the channel finishes.
	Completed,
	// When the charge or channel is cancelled.
	Cancelled
};

// A float User parameter of the system, set from the ability when the entry plays.
USTRUCT(BlueprintType)
struct PWGASCORE_API FPWVFXParameter
{
	GENERATED_BODY()

	// The parameter's name with its namespace, e.g. User.Radius.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX")
	FName Name;

	// What it is set to: a module's value, the ability level, a variable of the ability, a constant, or math on those. Unset sends 0.
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category="VFX")
	TObjectPtr<UPWAbilityValue> Value;
};

USTRUCT(BlueprintType)
struct PWGASCORE_API FPWVFXEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX")
	TObjectPtr<UNiagaraSystem> NiagaraSystem = nullptr;

	// Plays it at this moment of the ability without a graph; Manual leaves it to Play nodes.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX")
	EPWVFXTiming PlayOn = EPWVFXTiming::Manual;

	// Keeps playing until stopped.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX")
	bool bLooping = false;

	// Stops it when the ability ends.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX", meta=(EditCondition="bLooping"))
	bool bStopWhenAbilityEnds = true;

	// A socket on the caster's meshes for Play and the automatic timings; None uses the caster's location.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX")
	FName Socket = NAME_None;

	// Follows the caster or socket; off plays it once where it is.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX")
	bool bAttach = false;

	// Turns with the caster's facing, the socket when attached to one, or the transform given to Play At Location.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX")
	FVector LocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX")
	FRotator RotationOffset = FRotator::ZeroRotator;

	// The effect component's scale. Local Space emitters shrink or grow with it; World Space emitters keep their speeds and spread in world units,
	// so scale can do little or nothing to them. To size those, drive their values with Parameters.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX")
	FVector Scale = FVector::OneVector;

	// Float User parameters the system exposes, set on every play: typed values, or the ability's radius, range or level.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="VFX")
	TArray<FPWVFXParameter> Parameters;
};

// The ability's visual effects: when they play, where, at what scale, and with which parameter values.
UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced, meta=(DisplayName="PWM VFX"))
class PWGASCORE_API UPWAbilityModule_VFX : public UPWControlModule
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="VFX")
	TArray<FPWVFXEntry> Entries;

	// Plays an entry at its socket or the caster.
	UFUNCTION(BlueprintCallable, Category="VFX")
	UNiagaraComponent* Play(int32 Index);

	// Plays an entry at a world transform; its offsets and size still apply.
	UFUNCTION(BlueprintCallable, Category="VFX")
	UNiagaraComponent* PlayAtLocation(int32 Index, const FTransform& WorldTransform);

	// Plays an entry at an actor's location and facing.
	UFUNCTION(BlueprintCallable, Category="VFX")
	UNiagaraComponent* PlayOnActor(int32 Index, AActor* TargetActor);

	// Stops a looping entry.
	UFUNCTION(BlueprintCallable, Category="VFX")
	void Stop(int32 Index);

	UFUNCTION(BlueprintCallable, Category="VFX")
	void StopAll();

	// The value an entry's parameter is set to.
	UFUNCTION(BlueprintPure, Category="VFX")
	float GetParameterValue(int32 Index, int32 ParameterIndex) const;

	virtual void OnAbilityActivated_Implementation() override;
	virtual void OnAbilityPhase_Implementation(EPWAbilityPhase Phase) override;
	virtual void OnAbilityEnded_Implementation() override;
	virtual void PostLoad() override;

protected:
	// Entries saved before the module became VFX.
	UPROPERTY()
	TArray<FPWNiagaraEntry> NiagaraEntries_DEPRECATED;

	// Looping entries playing now: index to unique id.
	UPROPERTY(Transient)
	TMap<int32, int32> ActiveIndexToId;

	int32 ComputeUniqueId(int32 Index) const;
	void PlayTimed(EPWVFXTiming Timing);
	FPWNiagaraEntry MakePlay(int32 Index) const;
	UNiagaraComponent* InternalPlay(int32 Index, FPWNiagaraEntry& Played, const TCHAR* From);
	void InternalStopByIds(const TArray<int32>& Ids) const;
	void DebugPlay(int32 Index, const FPWNiagaraEntry& Played, const TCHAR* From, const UNiagaraComponent* LocalComp, bool bOwnerPlays) const;
};
