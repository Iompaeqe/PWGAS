// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWActionModule.h"
#include "Ability/PWSpellParams.h"
#include "PWAbilityModule_Actor.generated.h"

struct FGameplayAbilityTargetDataHandle;
class UPWAbilityTask_TargetFromSource;
struct FPWTargetingResult;

UCLASS(BlueprintType)
class PWGASCORE_API UPWActorSpellParams : public UPWSpellParamsExtension
{
	GENERATED_BODY()

public:
	// Where the actor was asked to spawn, filled in by the module at cast time.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector SpawnLocation = FVector::ZeroVector;

	// Facing the actor was asked to spawn with, filled in by the module at cast time.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FRotator SpawnRotation = FRotator::ZeroRotator;

	// Relations the spawned actor may collide with, copied from the module.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 TargetMask = (int32)ERelationMask::All;

	// Lifetime in seconds for the spawned actor to use; 0 leaves it to the actor.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Duration = 0.f;

	// Whether the spawned actor is attached to the caster.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bAttachToOwner = false;

	// Socket on the caster's mesh the actor is attached to.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName AttachSocket = NAME_None;
};


UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWAbilityModule_Actor : public UPWActionModule
{
	GENERATED_BODY()

public:
	/** Actor class to spawn. Must implement PWSpellParamsInterface. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(MustImplement="PWSpellParamsInterface"))
	TSubclassOf<AActor> ActorClass;
	// Targets the spawned actor collides with; those outside the mask are passed through. Walls and other scenery always stop it.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Bitmask, BitmaskEnum="/Script/PWSharedTypes.ERelationMask"))
	int32 TargetMask = (int32)ERelationMask::All;
	// Spawns as soon as the ability activates, so no graph is needed.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bSpawnOnActivated = false;

	/** If true, the actor spawns at the target location instead of at the ability owner. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bSpawnAtTarget = false;

	/** If true, the spawned actor will be attached to the ability owner. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bAttachToOwner = false;
	/** Socket name to attach to when bAttachToOwner = true. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName SpawnSocket = NAME_None;
	/** Optional local offset applied after forward offset. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector SpawnRelativeOffset = FVector::ZeroVector;
	/** If true, pitch will be flattened (useful for ground spawns). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bZeroPitch = false;
	// Draws a red sphere at the spawn position for three seconds.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bDebugSpawnPosition = false;

	
	virtual UPWSpellParams* MakeSpellParams_Implementation(const FPWTargetingResult& Target) override;

	UFUNCTION(BlueprintCallable, Category="Module")
	virtual AActor* SpawnActorWithTargetingResult(const FPWTargetingResult& Target);
	UFUNCTION(BlueprintCallable, Category="Module")
	void SpawnActor();

	UFUNCTION(BlueprintCallable, Category="Module")
	void DestroySpawnedActors();

protected:
	virtual FTransform BuildSpawnTransform(const FPWTargetingResult& Target) const;
	
	TArray<TWeakObjectPtr<AActor>> SpawnedActors = {};

	virtual void ExecuteAction() override { SpawnActor(); }
	virtual void GetPreviewTransforms(const FPWTargetingResult& Target, TArray<FTransform>& OutTransforms) const override;
	virtual TSubclassOf<AActor> GetPreviewActorClass() const override { return ActorClass; }
	virtual void OnAbilityActivated_Implementation() override;
	virtual void GetRequiredDataModules_Implementation(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules) override;

	UPROPERTY()
	TObjectPtr<UPWAbilityTask_TargetFromSource> ActiveTargetTask;
	/** Called by ability task when targeting completes (client and server). */
	UFUNCTION()
	virtual void OnTargetDataReady(const FGameplayAbilityTargetDataHandle& DataHandle,
								   const FPWTargetingResult& Result);
private:
};