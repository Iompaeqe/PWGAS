// copyright mkurtt96

#include "GAS/Abilities/Modules/ActionModules/PWAbilityModule_MultiActor.h"

#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Radius.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "Engine/World.h"
#include "TimerManager.h"

AActor* UPWAbilityModule_MultiActor::SpawnActorWithTargetingResult(const FPWTargetingResult& Target)
{
    if (ActorCount <= 1)
    {
        return Super::SpawnActorWithTargetingResult(Target);
    }
    
    SpawnMultipleActors(Target);
    return nullptr;
}

void UPWAbilityModule_MultiActor::SpawnMultipleActors(const FPWTargetingResult& Target)
{
    if (ActorCount <= 1)
    {
        Super::SpawnActorWithTargetingResult(Target);
        return;
    }

    CachedTargetResult = Target;

    float EffectiveRadius = GetEffectiveRadius();
    BuildSpawnPositions(Target.Location, EffectiveRadius);
    CurrentSpawnIndex = 0;
    CachedCenterLocation = Target.Location;

    SpawnOneActorInSequence(Target);

    // Schedule remaining actors if there's a delay
    if (SpawnDelay > 0.f && ActorCount > 1)
    {
        OwnerAbility->GetWorld()->GetTimerManager().SetTimer(
            SpawnTimerHandle,
            FTimerDelegate::CreateWeakLambda(this, [this, Target]() { SpawnOneActorInSequence(Target); }),
            SpawnDelay,
            true
        );
    }
    else
    {
        for (int32 i = 1; i < ActorCount; ++i)
        {
            SpawnOneActorInSequence(Target);
        }
    }
}

void UPWAbilityModule_MultiActor::SpawnOneActorInSequence(const FPWTargetingResult& Target)
{
    if (CurrentSpawnIndex >= CachedSpawnPositions.Num())
        return;

    FPWTargetingResult ModifiedTarget = Target;
    ModifiedTarget.Location = CachedSpawnPositions[CurrentSpawnIndex];
    
    FVector ToCenter = (CachedCenterLocation - ModifiedTarget.Location).GetSafeNormal();
    ModifiedTarget.Rotation = ToCenter.Rotation();

    Super::SpawnActorWithTargetingResult(ModifiedTarget);
    CurrentSpawnIndex++;

    if (CurrentSpawnIndex >= CachedSpawnPositions.Num())
        OwnerAbility->GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
}

float UPWAbilityModule_MultiActor::GetEffectiveRadius() const
{
    if (Radius > 0.f)
        return Radius;
        
    if (UPWAbilityModule_Radius* RadiusModule = OwnerAbility->GetRadiusModule())
    {
        return RadiusModule->GetRadiusValue();
    }
    
    return 0.f;
}

void UPWAbilityModule_MultiActor::BuildSpawnPositions(const FVector& CenterLocation, float InRadius)
{
    CachedSpawnPositions.Empty(ActorCount);
    
    switch (SpawnPattern)
    {
    case EActorSpawnPattern::CircleClockwise: Build_CircleClockwise(CenterLocation, InRadius); break;
    case EActorSpawnPattern::CircleCounterClockwise: Build_CircleCounterClockwise(CenterLocation, InRadius); break;
    case EActorSpawnPattern::SpreadOutFromCenter: Build_SpreadOutFromCenter(CenterLocation, InRadius); break;
    case EActorSpawnPattern::SpreadIntoCenter: Build_SpreadIntoCenter(CenterLocation, InRadius); break;
    case EActorSpawnPattern::GatherAtCenter: Build_GatherAtCenter(CenterLocation, InRadius); break;
    case EActorSpawnPattern::GridPattern: Build_GridPattern(CenterLocation, InRadius); break;
    case EActorSpawnPattern::LineHorizontal: Build_LineHorizontal(CenterLocation, InRadius); break;
    case EActorSpawnPattern::LineVertical: Build_LineVertical(CenterLocation, InRadius); break;
    case EActorSpawnPattern::LineHorizontalForward: Build_LineHorizontalForward(CenterLocation, InRadius); break;
    case EActorSpawnPattern::LineHorizontalBackward: Build_LineHorizontalBackward(CenterLocation, InRadius); break;
    case EActorSpawnPattern::LineHorizontalSpreadOut: Build_LineHorizontalSpreadOut(CenterLocation, InRadius); break;
    case EActorSpawnPattern::LineHorizontalSpreadIn: Build_LineHorizontalSpreadIn(CenterLocation, InRadius); break;
    case EActorSpawnPattern::LineVerticalForward: Build_LineVerticalForward(CenterLocation, InRadius); break;
    case EActorSpawnPattern::LineVerticalBackward: Build_LineVerticalBackward(CenterLocation, InRadius); break;
    case EActorSpawnPattern::LineVerticalSpreadOut: Build_LineVerticalSpreadOut(CenterLocation, InRadius); break;
    case EActorSpawnPattern::LineVerticalSpreadIn: Build_LineVerticalSpreadIn(CenterLocation, InRadius); break;
    case EActorSpawnPattern::Random: Build_Random(CenterLocation, InRadius); break;
    default: Build_CircleClockwise(CenterLocation, InRadius); break;
    }
}

void UPWAbilityModule_MultiActor::Build_CircleClockwise(const FVector& Center, float InRadius)
{
    if (InRadius <= 0.f)
    {
        for (int32 i = 0; i < ActorCount; ++i)
        {
            CachedSpawnPositions.Add(Center);
        }
        return;
    }

    for (int32 i = 0; i < ActorCount; ++i)
    {
        float Angle = (360.f / ActorCount) * i;
        float RadianAngle = FMath::DegreesToRadians(Angle);
        
        FVector Offset = FVector(
            FMath::Cos(RadianAngle) * InRadius,
            FMath::Sin(RadianAngle) * InRadius,
            0.f
        );
        
        CachedSpawnPositions.Add(Center + Offset);
    }
}

void UPWAbilityModule_MultiActor::Build_CircleCounterClockwise(const FVector& Center, float InRadius)
{
    Build_CircleClockwise(Center, InRadius);
    Algo::Reverse(CachedSpawnPositions);
}

void UPWAbilityModule_MultiActor::Build_SpreadOutFromCenter(const FVector& Center, float InRadius)
{
    if (InRadius <= 0.f)
    {
        Build_GatherAtCenter(Center, InRadius);
        return;
    }

    // Calculate ring distribution: center gets 1, then rings get StepCount each
    TArray<int32> RingCounts;
    int32 RemainingActors = ActorCount;
    
    // Center ring gets 1 actor
    if (RemainingActors > 0)
    {
        RingCounts.Add(1);
        RemainingActors--;
    }
    
    // Distribute remaining actors in rings of StepCount
    int32 RingIndex = 1;
    while (RemainingActors > 0)
    {
        int32 ActorsInThisRing = FMath::Min(StepCount, RemainingActors);
        RingCounts.Add(ActorsInThisRing);
        RemainingActors -= ActorsInThisRing;
        RingIndex++;
    }
    
    // Spawn from center outward
    for (int32 Ring = 0; Ring < RingCounts.Num(); ++Ring)
    {
        if (Ring == 0)
        {
            // Center
            CachedSpawnPositions.Add(Center);
        }
        else
        {
            float RingRadius = (InRadius / RingCounts.Num()) * Ring;
            for (int32 i = 0; i < RingCounts[Ring]; ++i)
            {
                float Angle = (360.f / RingCounts[Ring]) * i;
                float RadianAngle = FMath::DegreesToRadians(Angle);
                
                FVector Offset = FVector(
                    FMath::Cos(RadianAngle) * RingRadius,
                    FMath::Sin(RadianAngle) * RingRadius,
                    0.f
                );
                
                CachedSpawnPositions.Add(Center + Offset);
            }
        }
    }
}

void UPWAbilityModule_MultiActor::Build_SpreadIntoCenter(const FVector& Center, float InRadius)
{
    if (InRadius <= 0.f)
    {
        Build_GatherAtCenter(Center, InRadius);
        return;
    }

    // Calculate ring distribution from outside in
    TArray<int32> RingCounts;
    int32 RemainingActors = ActorCount;
    
    // Center gets 1 actor if odd remainder
    int32 CenterActors = RemainingActors % StepCount == 1 ? 1 : 0;
    if (CenterActors > 0)
    {
        RemainingActors--;
    }
    
    // Calculate rings from outside
    TArray<int32> OuterRings;
    while (RemainingActors > 0)
    {
        int32 ActorsInRing = FMath::Min(StepCount, RemainingActors);
        OuterRings.Add(ActorsInRing);
        RemainingActors -= ActorsInRing;
    }
    
    // Spawn from outside in
    for (int32 Ring = 0; Ring < OuterRings.Num(); ++Ring)
    {
        float RingRadius = InRadius - ((InRadius / (OuterRings.Num() + (CenterActors > 0 ? 1 : 0))) * Ring);
        
        for (int32 i = 0; i < OuterRings[Ring]; ++i)
        {
            float Angle = (360.f / OuterRings[Ring]) * i;
            float RadianAngle = FMath::DegreesToRadians(Angle);
            
            FVector Offset = FVector(
                FMath::Cos(RadianAngle) * RingRadius,
                FMath::Sin(RadianAngle) * RingRadius,
                0.f
            );
            
            CachedSpawnPositions.Add(Center + Offset);
        }
    }
    
    // Add center actor last
    if (CenterActors > 0)
    {
        CachedSpawnPositions.Add(Center);
    }
}

void UPWAbilityModule_MultiActor::Build_GatherAtCenter(const FVector& Center, float InRadius)
{
    for (int32 i = 0; i < ActorCount; ++i)
    {
        CachedSpawnPositions.Add(Center);
    }
}

void UPWAbilityModule_MultiActor::Build_GridPattern(const FVector& Center, float InRadius)
{
    if (InRadius <= 0.f)
    {
        Build_GatherAtCenter(Center, InRadius);
        return;
    }
    
    int32 GridSize = FMath::CeilToInt(FMath::Sqrt(static_cast<float>(ActorCount)));
    float CellSize = (InRadius * 2.f) / FMath::Max(1, GridSize - 1);
    
    FVector GridStart = Center - FVector(InRadius, InRadius, 0.f);
    
    for (int32 i = 0; i < ActorCount; ++i)
    {
        int32 Row = i / GridSize;
        int32 Col = i % GridSize;
        
        FVector Position = GridStart + FVector(
            Col * CellSize,
            Row * CellSize,
            0.f
        );
        
        CachedSpawnPositions.Add(Position);
    }
}

FVector UPWAbilityModule_MultiActor::GetCastDirection(const FPWTargetingResult& Target) const
{
    if (!Target.Rotation.IsNearlyZero())
    {
        return Target.Rotation.Vector();
    }
    
    if (AActor* OwnerPawn = OwnerAbility->GetOwningActorFromActorInfo())
    {
        return OwnerPawn->GetActorForwardVector();
    }
    
    return FVector::ForwardVector;
}

void UPWAbilityModule_MultiActor::Build_LineHorizontal(const FVector& Center, float InRadius)
{
    if (ActorCount <= 1)
    {
        CachedSpawnPositions.Add(Center);
        return;
    }
    
    if (InRadius <= 0.f)
    {
        Build_GatherAtCenter(Center, InRadius);
        return;
    }
    
    FVector CastDir = GetCastDirection(CachedTargetResult);
    FVector RightVector = FVector::CrossProduct(CastDir, FVector::UpVector).GetSafeNormal();
    
    float LineLength = InRadius * 2.f;
    float Spacing = LineLength / (ActorCount - 1);
    
    FVector LineStart = Center - (RightVector * InRadius);
    
    for (int32 i = 0; i < ActorCount; ++i)
    {
        FVector Position = LineStart + (RightVector * (i * Spacing));
        CachedSpawnPositions.Add(Position);
    }
}

void UPWAbilityModule_MultiActor::Build_LineVertical(const FVector& Center, float InRadius)
{
    if (ActorCount <= 1)
    {
        CachedSpawnPositions.Add(Center);
        return;
    }
    
    if (InRadius <= 0.f)
    {
        Build_GatherAtCenter(Center, InRadius);
        return;
    }
    
    FVector CastDir = GetCastDirection(CachedTargetResult);
    
    float LineLength = InRadius * 2.f;
    float Spacing = LineLength / (ActorCount - 1);
    
    FVector LineStart = Center - (CastDir * InRadius);
    
    for (int32 i = 0; i < ActorCount; ++i)
    {
        FVector Position = LineStart + (CastDir * (i * Spacing));
        CachedSpawnPositions.Add(Position);
    }
}

void UPWAbilityModule_MultiActor::Build_LineHorizontalForward(const FVector& Center, float InRadius)
{
    if (ActorCount <= 1)
    {
        CachedSpawnPositions.Add(Center);
        return;
    }
    
    if (InRadius <= 0.f)
    {
        Build_GatherAtCenter(Center, InRadius);
        return;
    }
    
    FVector CastDir = GetCastDirection(CachedTargetResult);
    FVector RightVector = FVector::CrossProduct(CastDir, FVector::UpVector).GetSafeNormal();
    
    float Spacing = (InRadius * 2.f) / (ActorCount - 1);
    
    for (int32 i = 0; i < ActorCount; ++i)
    {
        FVector Position = Center + (RightVector * (i * Spacing));
        CachedSpawnPositions.Add(Position);
    }
}

void UPWAbilityModule_MultiActor::Build_LineHorizontalBackward(const FVector& Center, float InRadius)
{
    if (ActorCount <= 1)
    {
        CachedSpawnPositions.Add(Center);
        return;
    }
    
    if (InRadius <= 0.f)
    {
        Build_GatherAtCenter(Center, InRadius);
        return;
    }
    
    FVector CastDir = GetCastDirection(CachedTargetResult);
    FVector LeftVector = -FVector::CrossProduct(CastDir, FVector::UpVector).GetSafeNormal();
    
    float Spacing = (InRadius * 2.f) / (ActorCount - 1);
    
    for (int32 i = 0; i < ActorCount; ++i)
    {
        FVector Position = Center + (LeftVector * (i * Spacing));
        CachedSpawnPositions.Add(Position);
    }
}

void UPWAbilityModule_MultiActor::Build_LineHorizontalSpreadOut(const FVector& Center, float InRadius)
{
    if (ActorCount <= 1)
    {
        CachedSpawnPositions.Add(Center);
        return;
    }
    
    if (InRadius <= 0.f)
    {
        Build_GatherAtCenter(Center, InRadius);
        return;
    }
    
    FVector CastDir = GetCastDirection(CachedTargetResult);
    FVector RightVector = FVector::CrossProduct(CastDir, FVector::UpVector).GetSafeNormal();
    
    CachedSpawnPositions.Add(Center);
    
    for (int32 i = 1; i < ActorCount; ++i)
    {
        float Distance = ((i + 1) / 2) * (InRadius / (ActorCount / 2));
        FVector Direction = (i % 2 == 1) ? RightVector : -RightVector;
        
        FVector Position = Center + (Direction * Distance);
        CachedSpawnPositions.Add(Position);
    }
}

void UPWAbilityModule_MultiActor::Build_LineHorizontalSpreadIn(const FVector& Center, float InRadius)
{
    Build_LineHorizontalSpreadOut(Center, InRadius);
    Algo::Reverse(CachedSpawnPositions);
}

void UPWAbilityModule_MultiActor::Build_LineVerticalForward(const FVector& Center, float InRadius)
{
    if (ActorCount <= 1)
    {
        CachedSpawnPositions.Add(Center);
        return;
    }
    
    if (InRadius <= 0.f)
    {
        Build_GatherAtCenter(Center, InRadius);
        return;
    }
    
    FVector CastDir = GetCastDirection(CachedTargetResult);
    float Spacing = (InRadius * 2.f) / (ActorCount - 1);
    
    for (int32 i = 0; i < ActorCount; ++i)
    {
        FVector Position = Center + (CastDir * (i * Spacing));
        CachedSpawnPositions.Add(Position);
    }
}

void UPWAbilityModule_MultiActor::Build_LineVerticalBackward(const FVector& Center, float InRadius)
{
    if (ActorCount <= 1)
    {
        CachedSpawnPositions.Add(Center);
        return;
    }
    
    if (InRadius <= 0.f)
    {
        Build_GatherAtCenter(Center, InRadius);
        return;
    }
    
    FVector CastDir = -GetCastDirection(CachedTargetResult);
    float Spacing = (InRadius * 2.f) / (ActorCount - 1);
    
    for (int32 i = 0; i < ActorCount; ++i)
    {
        FVector Position = Center + (CastDir * (i * Spacing));
        CachedSpawnPositions.Add(Position);
    }
}

void UPWAbilityModule_MultiActor::Build_LineVerticalSpreadOut(const FVector& Center, float InRadius)
{
    if (ActorCount <= 1)
    {
        CachedSpawnPositions.Add(Center);
        return;
    }
    
    if (InRadius <= 0.f)
    {
        Build_GatherAtCenter(Center, InRadius);
        return;
    }
    
    FVector CastDir = GetCastDirection(CachedTargetResult);
    
    // Start from center, alternate forward/backward going outward
    CachedSpawnPositions.Add(Center);
    
    for (int32 i = 1; i < ActorCount; ++i)
    {
        float Distance = ((i + 1) / 2) * (InRadius / (ActorCount / 2));
        FVector Direction = (i % 2 == 1) ? CastDir : -CastDir;
        
        FVector Position = Center + (Direction * Distance);
        CachedSpawnPositions.Add(Position);
    }
}

void UPWAbilityModule_MultiActor::Build_LineVerticalSpreadIn(const FVector& Center, float InRadius)
{
    Build_LineVerticalSpreadOut(Center, InRadius);
    Algo::Reverse(CachedSpawnPositions);
}

void UPWAbilityModule_MultiActor::Build_Random(const FVector& Center, float InRadius)
{
    if (InRadius <= 0.f)
    {
        Build_GatherAtCenter(Center, InRadius);
        return;
    }
    
    for (int32 i = 0; i < ActorCount; ++i)
    {
        float RandomRadius = FMath::Sqrt(FMath::FRand()) * InRadius;
        float RandomAngle = FMath::FRand() * 360.f;
        float RadianAngle = FMath::DegreesToRadians(RandomAngle);
        
        FVector Offset = FVector(
            FMath::Cos(RadianAngle) * RandomRadius,
            FMath::Sin(RadianAngle) * RandomRadius,
            0.f
        );
        
        CachedSpawnPositions.Add(Center + Offset);
    }
}