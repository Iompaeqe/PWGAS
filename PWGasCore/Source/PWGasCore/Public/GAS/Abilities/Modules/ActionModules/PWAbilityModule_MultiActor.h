// copyright mkurtt96

#pragma once

#include "CoreMinimal.h"
#include "PWAbilityModule_Actor.h"
#include "Targeting/Data/PWTargetingData.h"
#include "PWAbilityModule_MultiActor.generated.h"

UENUM(BlueprintType)
enum class EActorSpawnPattern : uint8
{
    CircleClockwise UMETA(DisplayName="Circle → Clockwise"),
    CircleCounterClockwise UMETA(DisplayName="Circle ← Counter-Clockwise"),
    SpreadOutFromCenter UMETA(DisplayName="Center → Spread Out (Steps)"),
    SpreadIntoCenter UMETA(DisplayName="Edges → Spread Into Center (Steps)"),
    GatherAtCenter UMETA(DisplayName="Gather At Center"),
    GridPattern UMETA(DisplayName="Grid Pattern"),
    LineHorizontal UMETA(DisplayName="Horizontal Line (Perpendicular to cast)"),
    LineVertical UMETA(DisplayName="Vertical Line (Along cast direction)"),
    LineHorizontalForward UMETA(DisplayName="Horizontal Forward"),
    LineHorizontalBackward UMETA(DisplayName="Horizontal Backward"),
    LineHorizontalSpreadOut UMETA(DisplayName="Horizontal Spread Out"),
    LineHorizontalSpreadIn UMETA(DisplayName="Horizontal Spread In"),
    LineVerticalForward UMETA(DisplayName="Vertical Forward"),
    LineVerticalBackward UMETA(DisplayName="Vertical Backward"),
    LineVerticalSpreadOut UMETA(DisplayName="Vertical Spread Out"),
    LineVerticalSpreadIn UMETA(DisplayName="Vertical Spread In"),
    Random UMETA(DisplayName="Random Positions")
};

UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWAbilityModule_MultiActor : public UPWAbilityModule_Actor
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin=1))
    int32 ActorCount = 3;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    EActorSpawnPattern SpawnPattern = EActorSpawnPattern::CircleClockwise;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin=0.0f))
    float SpawnDelay = 0.f;
    
	UPROPERTY()
	float Radius = 0.f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin=1, ToolTip="Step size for spread patterns - how many actors per ring/line segment"))
    int32 StepCount = 2;

    UFUNCTION(BlueprintCallable, Category="Module")
    void SpawnMultipleActors(const FPWTargetingResult& Target);
    
    UFUNCTION(BlueprintCallable, Category="Module") 
    void SpawnOneActorInSequence(const FPWTargetingResult& Target);

    virtual AActor* SpawnActorWithTargetingResult(const FPWTargetingResult& Target) override;

private:
    FPWTargetingResult CachedTargetResult;
    FVector GetCastDirection(const FPWTargetingResult& Target) const;
    
    int32 CurrentSpawnIndex = 0;
    TArray<FVector> CachedSpawnPositions;
    FVector CachedCenterLocation;
    FTimerHandle SpawnTimerHandle;
    
    void BuildSpawnPositions(const FVector& CenterLocation, float InRadius);
    float GetEffectiveRadius() const;
    
    // Pattern builders
    void Build_CircleClockwise(const FVector& Center, float InRadius);
    void Build_CircleCounterClockwise(const FVector& Center, float InRadius);
    void Build_SpreadOutFromCenter(const FVector& Center, float InRadius);
    void Build_SpreadIntoCenter(const FVector& Center, float InRadius);
    void Build_GatherAtCenter(const FVector& Center, float InRadius);
    void Build_GridPattern(const FVector& Center, float InRadius);
    void Build_LineHorizontal(const FVector& Center, float InRadius);
    void Build_LineVertical(const FVector& Center, float InRadius);
    void Build_LineHorizontalForward(const FVector& Center, float InRadius);
    void Build_LineHorizontalBackward(const FVector& Center, float InRadius);
    void Build_LineHorizontalSpreadOut(const FVector& Center, float InRadius);
    void Build_LineHorizontalSpreadIn(const FVector& Center, float InRadius);
    void Build_LineVerticalForward(const FVector& Center, float InRadius);
    void Build_LineVerticalBackward(const FVector& Center, float InRadius);
    void Build_LineVerticalSpreadOut(const FVector& Center, float InRadius);
    void Build_LineVerticalSpreadIn(const FVector& Center, float InRadius);
    void Build_Random(const FVector& Center, float InRadius);
};
