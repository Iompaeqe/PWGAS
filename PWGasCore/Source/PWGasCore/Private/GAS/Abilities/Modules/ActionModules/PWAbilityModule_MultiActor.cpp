// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "GAS/Abilities/Modules/ActionModules/PWAbilityModule_MultiActor.h"

#include "GAS/Abilities/Modules/ActionModules/SpawnPatterns/PWSpawnPattern_Circle.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Radius.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "Engine/CollisionProfile.h"
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
    {
        if (IsDebugging())
            DebugPrint(TEXT("actor count is 1, spawning one actor without the pattern"));
        return Super::SpawnActorWithTargetingResult(Target);
    }

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

    BuildSpawnTransforms(Target, CachedSpawnTransforms, true);
    CurrentSpawnIndex = 0;

    if (IsDebugging())
    {
        const int32 Level = OwnerAbility->GetAbilityLevel();
        FString PatternName = Pattern ? Pattern->GetClass()->GetName() : TEXT("Circle (default)");
        PatternName.RemoveFromStart(TEXT("PWSpawnPattern_"));
        PatternName.RemoveFromEnd(TEXT("_C"));
        const FString Timing = SpawnDelay > 0.f
            ? FString::Printf(TEXT("one every %.2fs, the last in %.2fs"), SpawnDelay, SpawnDelay * (CachedSpawnTransforms.Num() - 1))
            : FString(TEXT("all at once"));
        DebugPrint(FString::Printf(TEXT("%d actors (Actor Count %d + %d bonus at level %d), %s pattern gave %d positions, oriented to %s, %s"),
            GetActorCount(), ActorCount, FMath::RoundToInt(BonusActors.GetValueAtLevel(Level)), Level, *PatternName, CachedSpawnTransforms.Num(),
            *StaticEnum<EPWActorPatternOrientation>()->GetNameStringByValue(static_cast<int64>(PatternOrientation)), *Timing));

        const TCHAR* RadiusSource = Radius > 0.f ? TEXT("its own Radius field") : OwnerAbility->GetRadiusModule() ? TEXT("the Radius module") : TEXT("nothing, no Radius module");
        const FString Ground = bSnapToGround
            ? FString::Printf(TEXT("ground snap on (trace %.0f up and down, %.0f above the ground, channel %s)"), GroundTraceDistance, GroundOffset,
                *UCollisionProfile::Get()->ReturnChannelNameFromContainerIndex(GroundChannel.GetValue()).ToString())
            : FString(TEXT("ground snap off"));
        DebugPrint(FString::Printf(TEXT("radius %.0f from %s, offset %s, rotation %s, %s, %s"), GetEffectiveRadius(), RadiusSource, *PatternOffset.ToCompactString(),
            *PatternRotation.ToCompactString(), bFaceCenter ? TEXT("facing the center") : TEXT("facing the pattern's forward"), *Ground));
    }

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

    if (SpawnDelay > 0.f && IsDebugging())
        DebugPrint(FString::Printf(TEXT("spawning %d of %d"), CurrentSpawnIndex + 1, CachedSpawnTransforms.Num()));

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

void UPWAbilityModule_MultiActor::BuildSpawnTransforms(const FPWTargetingResult& Target, TArray<FTransform>& OutTransforms, const bool bSpawning) const
{
    OutTransforms.Reset();
    const bool bDebug = bSpawning && IsDebugging();

    const int32 Count = GetActorCount();
    const UPWSpawnPattern* UsedPattern = Pattern ? Pattern.Get() : GetDefault<UPWSpawnPattern_Circle>();
    TArray<FVector> Positions = UsedPattern->BuildPositions(Count, GetEffectiveRadius());

    const FQuat Basis = GetPatternBasis(Target);
    FVector Center = Target.Location + Basis.RotateVector(PatternOffset);
    for (FVector& Position : Positions)
        Position = Center + Basis.RotateVector(Position);

    if (bSnapToGround)
    {
        Center = SnapToGround(Center, bDebug);
        for (FVector& Position : Positions)
            Position = SnapToGround(Position, bDebug);
    }

    for (const FVector& Position : Positions)
    {
        const FVector ToCenterRaw = Center - Position;
        const FVector ToCenter = bSnapToGround ? ToCenterRaw.GetSafeNormal2D() : ToCenterRaw.GetSafeNormal();
        OutTransforms.Add(FTransform(bFaceCenter && !ToCenter.IsZero() ? ToCenter.Rotation() : Basis.Rotator(), Position));
    }

    if (!bSpawning)
        return;

    // The Radius module draws under its own Show Debug; the old Radius field belongs to this module.
    if (Radius <= 0.f)
    {
        if (const UPWAbilityModule_Radius* RadiusModule = OwnerAbility->GetRadiusModule())
            RadiusModule->DebugDrawAt(Center, TEXT("pattern"));
    }
    else if (bDebug)
    {
        DebugCircle(Center, Radius, PWModuleDebug::Radius);
        DebugText(Center + FVector(0.f, 0.f, Radius), FString::Printf(TEXT("Radius %.0f (own field, pattern)"), Radius), PWModuleDebug::Radius);
    }

    if (!bDebug)
        return;

    DebugAxes(FTransform(Basis, Center), 100.f);
    DebugPoint(Center, PWModuleDebug::Position);
    DebugText(Center + FVector(0.f, 0.f, 40.f), TEXT("pattern center"));
    DebugPoint(Target.Location, PWModuleDebug::Target);
    if (!PatternOffset.IsZero())
        DebugLine(Target.Location, Center, PWModuleDebug::Target);

    for (int32 i = 0; i < OutTransforms.Num(); ++i)
    {
        const FVector Position = OutTransforms[i].GetLocation();
        DebugLine(Center, Position, PWModuleDebug::Position);
        DebugPoint(Position, PWModuleDebug::Position);
        DebugArrow(Position, Position + OutTransforms[i].GetRotation().GetForwardVector() * 80.f, PWModuleDebug::Direction);
        DebugText(Position + FVector(0.f, 0.f, 30.f), FString::FromInt(i));
    }
}

float UPWAbilityModule_MultiActor::GetEffectiveRadius() const
{
    if (Radius > 0.f)
        return Radius;

    if (const UPWAbilityModule_Radius* RadiusModule = OwnerAbility->GetRadiusModule())
        return RadiusModule->GetRadius();

    return 0.f;
}

void UPWAbilityModule_MultiActor::GetRequiredDataModules_Implementation(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules)
{
    Super::GetRequiredDataModules_Implementation(OutRequiredModules);
    OutRequiredModules.Add(UPWAbilityModule_Radius::StaticClass());
}

FVector UPWAbilityModule_MultiActor::SnapToGround(const FVector& Location, const bool bDebug) const
{
    const UWorld* World = OwnerAbility->GetWorld();
    if (!World)
        return Location;

    FCollisionQueryParams Params(SCENE_QUERY_STAT(PWPatternGround), false, OwnerAbility->GetAvatarActorFromActorInfo());
    const FVector Reach = FVector::UpVector * GroundTraceDistance;

    FHitResult Hit;
    const bool bHit = World->LineTraceSingleByChannel(Hit, Location + Reach, Location - Reach, GroundChannel, Params);
    if (bDebug)
    {
        DebugLine(Location + Reach, bHit ? FVector(Hit.ImpactPoint) : Location - Reach, bHit ? PWModuleDebug::Position : PWModuleDebug::Target);
        if (bHit)
            DebugPoint(Hit.ImpactPoint, PWModuleDebug::Position);
        else
        {
            DebugText(Location, TEXT("no ground"), PWModuleDebug::Target);
            DebugPrint(FString::Printf(TEXT("no ground within %.0f above or below %s, left in place"), GroundTraceDistance, *Location.ToCompactString()), FLinearColor::Yellow);
        }
    }

    if (bHit)
        return Hit.ImpactPoint + FVector::UpVector * GroundOffset;

    return Location;
}

FQuat UPWAbilityModule_MultiActor::GetPatternBasis(const FPWTargetingResult& Target) const
{
    FQuat Basis = FQuat::Identity;
    if (PatternOrientation != EPWActorPatternOrientation::World)
    {
        FVector Direction = Target.Rotation.Vector();
        if (const APawn* Pawn = OwnerAbility->GetPawn())
        {
            Direction = Pawn->GetActorForwardVector();
            if (PatternOrientation == EPWActorPatternOrientation::Camera && Pawn->GetController())
                Direction = Pawn->GetControlRotation().Vector();
        }

        Direction = Direction.GetSafeNormal2D();
        if (!Direction.IsZero())
            Basis = Direction.ToOrientationQuat();
    }

    return Basis * PatternRotation.Quaternion();
}
