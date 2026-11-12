// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "GAS/Abilities/Modules/ActionModules/PWAbilityModule_MultiActor.h"

#include "GAS/Abilities/Modules/ActionModules/SpawnPatterns/PWSpawnPatterns.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Radius.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "Engine/World.h"
#include "TimerManager.h"

int32 UPWAbilityModule_MultiActor::GetActorCount() const
{
    const int32 Level = OwnerAbility ? OwnerAbility->GetAbilityLevel() : 1;
    return FMath::Max(1, ActorCount + FMath::RoundToInt(BonusActors.GetValueAtLevel(Level)));
}

AActor* UPWAbilityModule_MultiActor::SpawnActorWithTargetingResult(const FPWTargetingResult& Target)
{
    if (GetActorCount() <= 1)
        return Super::SpawnActorWithTargetingResult(Target);

    SpawnMultipleActors(Target);
    return nullptr;
}

void UPWAbilityModule_MultiActor::SpawnMultipleActors(const FPWTargetingResult& Target)
{
    if (GetActorCount() <= 1)
    {
        Super::SpawnActorWithTargetingResult(Target);
        return;
    }

    BuildSpawnTransforms(Target, CachedSpawnTransforms);
    CurrentSpawnIndex = 0;

    SpawnOneActorInSequence(Target);

    if (SpawnDelay > 0.f)
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
        while (CurrentSpawnIndex < CachedSpawnTransforms.Num())
            SpawnOneActorInSequence(Target);
    }
}

void UPWAbilityModule_MultiActor::SpawnOneActorInSequence(const FPWTargetingResult& Target)
{
    if (CurrentSpawnIndex >= CachedSpawnTransforms.Num())
        return;

    FPWTargetingResult ModifiedTarget = Target;
    ModifiedTarget.Location = CachedSpawnTransforms[CurrentSpawnIndex].GetLocation();
    ModifiedTarget.Rotation = CachedSpawnTransforms[CurrentSpawnIndex].Rotator();

    Super::SpawnActorWithTargetingResult(ModifiedTarget);
    CurrentSpawnIndex++;

    if (CurrentSpawnIndex >= CachedSpawnTransforms.Num())
        OwnerAbility->GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
}

void UPWAbilityModule_MultiActor::GetPreviewTransforms(const FPWTargetingResult& Target, TArray<FTransform>& OutTransforms) const
{
    if (GetActorCount() <= 1)
    {
        Super::GetPreviewTransforms(Target, OutTransforms);
        return;
    }

    TArray<FTransform> Transforms;
    BuildSpawnTransforms(Target, Transforms);
    for (const FTransform& Transform : Transforms)
    {
        FPWTargetingResult ModifiedTarget = Target;
        ModifiedTarget.Location = Transform.GetLocation();
        ModifiedTarget.Rotation = Transform.Rotator();
        OutTransforms.Add(BuildSpawnTransform(ModifiedTarget));
    }
}

void UPWAbilityModule_MultiActor::BuildSpawnTransforms(const FPWTargetingResult& Target, TArray<FTransform>& OutTransforms) const
{
    OutTransforms.Reset();

    const int32 Count = GetActorCount();
    const UPWSpawnPattern* UsedPattern = Pattern ? Pattern.Get() : GetDefault<UPWSpawnPattern_Circle>();
    TArray<FVector> Positions = UsedPattern->BuildPositions(Count, GetEffectiveRadius());

    const FQuat Basis = GetPatternBasis(Target);
    FVector Center = Target.Location + Basis.RotateVector(PatternOffset);
    for (FVector& Position : Positions)
        Position = Center + Basis.RotateVector(Position);

    if (bSnapToGround)
    {
        Center = SnapToGround(Center);
        for (FVector& Position : Positions)
            Position = SnapToGround(Position);
    }

    for (const FVector& Position : Positions)
    {
        const FVector ToCenterRaw = Center - Position;
        const FVector ToCenter = bSnapToGround ? ToCenterRaw.GetSafeNormal2D() : ToCenterRaw.GetSafeNormal();
        OutTransforms.Add(FTransform(bFaceCenter && !ToCenter.IsZero() ? ToCenter.Rotation() : Basis.Rotator(), Position));
    }
}

float UPWAbilityModule_MultiActor::GetEffectiveRadius() const
{
    if (Radius > 0.f)
        return Radius;

    if (const UPWAbilityModule_Radius* RadiusModule = OwnerAbility->GetRadiusModule())
        return RadiusModule->GetRadiusValue();

    return 0.f;
}

void UPWAbilityModule_MultiActor::GetRequiredDataModules_Implementation(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules)
{
    Super::GetRequiredDataModules_Implementation(OutRequiredModules);
    OutRequiredModules.Add(UPWAbilityModule_Radius::StaticClass());
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
