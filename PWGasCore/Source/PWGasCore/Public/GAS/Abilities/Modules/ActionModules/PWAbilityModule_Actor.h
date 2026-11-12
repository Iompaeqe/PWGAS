// copyright mkurtt96

#pragma once

#include "CoreMinimal.h"
#include "PWActionModule.h"
#include "Ability/SpellParams.h"
#include "PWAbilityModule_Actor.generated.h"

struct FGameplayAbilityTargetDataHandle;
class UPWAbilityTask_TargetFromSource;
struct FPWTargetingResult;

UCLASS(BlueprintType)
class PWGASCORE_API UActorSpellParamsExtension : public USpellParamsExtension
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector SpawnLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FRotator SpawnRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 TargetMask = (int32)ERelationMask::All;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Duration = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bAttachToOwner = false;

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Bitmask, BitmaskEnum="/Script/PWSharedTypes.ERelationMask"))
	int32 TargetMask = (int32)ERelationMask::All;
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bDebugSpawnPosition = false;

	
	virtual USpellParams* MakeSpellParams_Implementation(const FPWTargetingResult& Target) override;

	UFUNCTION(BlueprintCallable, Category="Module")
	virtual AActor* SpawnActorWithTargetingResult(const FPWTargetingResult& Target);
	UFUNCTION(BlueprintCallable, Category="Module")
	void SpawnActor();

	UFUNCTION(BlueprintCallable, Category="Module")
	void DestroySpawnedActors();

protected:
	virtual FTransform BuildSpawnTransform(const FPWTargetingResult& Target) const;
	
	TArray<TWeakObjectPtr<AActor>> SpawnedActors = {};

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