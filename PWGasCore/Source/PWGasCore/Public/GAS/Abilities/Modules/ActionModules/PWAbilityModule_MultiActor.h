// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWAbilityModule_Actor.h"
#include "ScalableFloat.h"
#include "Targeting/Data/PWTargetingData.h"
#include "PWAbilityModule_MultiActor.generated.h"

class UPWSpawnPattern;

UENUM(BlueprintType)
enum class EActorPatternOrientation : uint8
{
    Caster UMETA(DisplayName="Relative To Caster"),
    World UMETA(DisplayName="Relative To World"),
    Camera UMETA(DisplayName="Relative To Camera")
};

UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWAbilityModule_MultiActor : public UPWAbilityModule_Actor
{
    GENERATED_BODY()

public:
    UPWAbilityModule_MultiActor() { bSpawnAtTarget = true; }

    // How many actors are spawned per cast.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin=1))
    int32 ActorCount = 3;

    // Added to Actor Count, by ability level.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FScalableFloat BonusActors = 0.f;

    // Seconds between each actor; 0 spawns them all at once.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin=0.0f))
    float SpawnDelay = 0.f;

    // Shape the actors are arranged in around the target point.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category="Pattern")
    TObjectPtr<UPWSpawnPattern> Pattern;

    // What the pattern's forward direction follows: the caster, the world or the camera.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
    EActorPatternOrientation PatternOrientation = EActorPatternOrientation::Camera;

    // Moves the whole pattern from the target point, in the pattern's own space.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
    FVector PatternOffset = FVector::ZeroVector;

    // Rotates the whole pattern around the target point.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
    FRotator PatternRotation = FRotator::ZeroRotator;

    // Each actor faces the pattern's center; off makes them all face the pattern's forward.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
    bool bFaceCenter = true;

    // Drops each actor onto the ground below its position.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
    bool bSnapToGround = false;

    // How far above and below a position the ground is searched for.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern", meta=(EditCondition="bSnapToGround", ClampMin=0.0f))
    float GroundTraceDistance = 1000.f;

    // Height above the ground the actor is placed at.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern", meta=(EditCondition="bSnapToGround"))
    float GroundOffset = 0.f;

    // Collision channel that counts as ground.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern", meta=(EditCondition="bSnapToGround"))
    TEnumAsByte<ECollisionChannel> GroundChannel = ECC_WorldStatic;

    UPROPERTY()
    float Radius = 0.f;

    UFUNCTION(BlueprintPure, Category="Module")
    int32 GetActorCount() const;

    UFUNCTION(BlueprintCallable, Category="Module")
    void SpawnMultipleActors(const FPWTargetingResult& Target);

    UFUNCTION(BlueprintCallable, Category="Module")
    void SpawnOneActorInSequence(const FPWTargetingResult& Target);

    virtual AActor* SpawnActorWithTargetingResult(const FPWTargetingResult& Target) override;
    virtual void GetPreviewTransforms(const FPWTargetingResult& Target, TArray<FTransform>& OutTransforms) const override;
    virtual void GetRequiredDataModules_Implementation(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules) override;

private:
    void BuildSpawnTransforms(const FPWTargetingResult& Target, TArray<FTransform>& OutTransforms) const;
    float GetEffectiveRadius() const;
    FQuat GetPatternBasis(const FPWTargetingResult& Target) const;
    FVector SnapToGround(const FVector& Location) const;

    int32 CurrentSpawnIndex = 0;
    TArray<FTransform> CachedSpawnTransforms;
    FTimerHandle SpawnTimerHandle;
};
