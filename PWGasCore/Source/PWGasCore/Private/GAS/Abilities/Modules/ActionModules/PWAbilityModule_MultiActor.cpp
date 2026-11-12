// copyright mkurtt96

#include "GAS/Abilities/Modules/ActionModules/PWAbilityModule_MultiActor.h"

#include "GAS/Abilities/Modules/ActionModules/SpawnPatterns/PWSpawnPatterns.h"
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
    
    const FVector ToCenterRaw = CachedCenterLocation - ModifiedTarget.Location;
    const FVector ToCenter = bSnapToGround ? ToCenterRaw.GetSafeNormal2D() : ToCenterRaw.GetSafeNormal();
    ModifiedTarget.Rotation = bFaceCenter && !ToCenter.IsZero() ? ToCenter.Rotation() : GetPatternBasis(Target).Rotator();

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
    const UPWSpawnPattern* UsedPattern = Pattern ? Pattern.Get() : GetDefault<UPWSpawnPattern_Circle>();
    CachedSpawnPositions = UsedPattern->BuildPositions(ActorCount, InRadius);

    const FQuat Basis = GetPatternBasis(CachedTargetResult);
    CachedCenterLocation = CenterLocation + Basis.RotateVector(PatternOffset);
    for (FVector& Position : CachedSpawnPositions)
        Position = CachedCenterLocation + Basis.RotateVector(Position);

    if (!bSnapToGround)
        return;

    CachedCenterLocation = SnapToGround(CachedCenterLocation);
    for (FVector& Position : CachedSpawnPositions)
        Position = SnapToGround(Position);
}

FVector UPWAbilityModule_MultiActor::SnapToGround(const FVector& Location) const
{
    const UWorld* World = OwnerAbility->GetWorld();
    if (!World)
        return Location;

    FCollisionQueryParams Params(SCENE_QUERY_STAT(PWPatternGround), false, OwnerAbility->GetAvatarActorFromActorInfo());
    const FVector Reach = FVector::UpVector * GroundTraceDistance;

    FHitResult Hit;
    if (World->LineTraceSingleByChannel(Hit, Location + Reach, Location - Reach, GroundChannel, Params))
        return Hit.ImpactPoint + FVector::UpVector * GroundOffset;

    return Location;
}

FQuat UPWAbilityModule_MultiActor::GetPatternBasis(const FPWTargetingResult& Target) const
{
    FQuat Basis = FQuat::Identity;
    if (PatternOrientation != EActorPatternOrientation::World)
    {
        FVector Direction = Target.Rotation.Vector();
        if (const APawn* Pawn = OwnerAbility->GetPawn())
        {
            Direction = Pawn->GetActorForwardVector();
            if (PatternOrientation == EActorPatternOrientation::Camera && Pawn->GetController())
                Direction = Pawn->GetControlRotation().Vector();
        }

        Direction = Direction.GetSafeNormal2D();
        if (!Direction.IsZero())
            Basis = Direction.ToOrientationQuat();
    }

    return Basis * PatternRotation.Quaternion();
}
