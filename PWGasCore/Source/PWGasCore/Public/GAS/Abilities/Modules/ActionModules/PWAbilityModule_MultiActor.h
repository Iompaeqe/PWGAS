// copyright mkurtt96

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

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin=1))
    int32 ActorCount = 3;

    // Added to Actor Count, by ability level.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FScalableFloat BonusActors = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin=0.0f))
    float SpawnDelay = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category="Pattern")
    TObjectPtr<UPWSpawnPattern> Pattern;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
    EActorPatternOrientation PatternOrientation = EActorPatternOrientation::Camera;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
    FVector PatternOffset = FVector::ZeroVector;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
    FRotator PatternRotation = FRotator::ZeroRotator;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
    bool bFaceCenter = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern")
    bool bSnapToGround = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern", meta=(EditCondition="bSnapToGround", ClampMin=0.0f))
    float GroundTraceDistance = 1000.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Pattern", meta=(EditCondition="bSnapToGround"))
    float GroundOffset = 0.f;

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
