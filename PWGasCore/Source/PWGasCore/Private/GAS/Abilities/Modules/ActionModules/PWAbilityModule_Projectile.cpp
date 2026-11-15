// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/Abilities/Modules/ActionModules/PWAbilityModule_Projectile.h"

#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "GAS/Abilities/Modules/ControlModules/PWAbilityModule_Precast.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Range.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/GameplayStaticsTypes.h"

UPWSpellParams* UPWAbilityModule_Projectile::MakeSpellParams_Implementation(const FPWTargetingResult& Target)
{
	if (!OwnerAbility)
		return nullptr;

	UPWSpellParams* Params = OwnerAbility->MakeSpellParams();
	UPWProjectileSpellParams* ProjectileParams = Params->AddExtension<UPWProjectileSpellParams>();
	ProjectileParams->SpawnLocation = Target.Location;
	ProjectileParams->SpawnRotation = BuildSpawnTransform(Target).Rotator();
	ProjectileParams->bAttachToOwner = bAttachToOwner;
	ProjectileParams->AttachSocket = SpawnSocket;
	
	ProjectileParams->InitialSpeed = GetProjectileSpeed();
	ProjectileParams->GravityZOverride = GetProjectileConfig().GravityZOverride;
	Params->CollisionFilter = TargetFilter;

	ExtendParamsWithData(Params);

	return Params;
}

void UPWAbilityModule_Projectile::DebugSpawn(const FPWTargetingResult& Target, const FTransform& SpawnTM, AActor* Spawned) const
{
	Super::DebugSpawn(Target, SpawnTM, Spawned);

	AActor* Avatar = OwnerAbility ? OwnerAbility->GetAvatarActorFromActorInfo() : nullptr;
	const UWorld* World = Avatar ? Avatar->GetWorld() : nullptr;
	if (!IsDebugging() || !World)
		return;

	const UPWAbilityModule_Range* RangeModule = OwnerAbility->GetRangeModule();
	if (RangeModule)
		RangeModule->DebugDrawFrom(Avatar->GetActorLocation(), TEXT("projectile"));

	// Size, collision channel and gravity scale come from the projectile class, as in the precast path.
	float CollisionRadius = 0.f;
	ECollisionChannel Channel = GetProjectileConfig().TraceChannel.GetValue();
	AActor::ForEachComponentOfActorClassDefault<USphereComponent>(ActorClass, [&](const USphereComponent* Sphere)
	{
		const float SphereRadius = Sphere->GetUnscaledSphereRadius() * Sphere->GetRelativeScale3D().GetAbsMax();
		if (SphereRadius > CollisionRadius)
		{
			CollisionRadius = SphereRadius;
			Channel = Sphere->GetCollisionObjectType();
		}
		return true;
	});
	const UProjectileMovementComponent* DefaultMovement = AActor::GetActorClassDefaultComponent<UProjectileMovementComponent>(ActorClass);
	const float GravityScale = DefaultMovement ? DefaultMovement->ProjectileGravityScale : 1.f;
	const bool bGravityOverride = !FMath::IsNearlyZero(GetProjectileConfig().GravityZOverride);
	const float GravityZ = bGravityOverride ? GetProjectileConfig().GravityZOverride : World->GetGravityZ() * GravityScale;

	const float Speed = GetProjectileSpeed();
	const FVector Start = SpawnTM.GetLocation();
	const FVector Direction = SpawnTM.GetRotation().GetForwardVector();
	const FVector ArrowTip = Start + Direction * FMath::Max(Speed * 0.25f, 50.f);
	DebugArrow(Start, ArrowTip, PWModuleDebug::Direction);
	DebugText(ArrowTip, FString::Printf(TEXT("%.0f/s"), Speed), PWModuleDebug::Direction);

	FString PathText = TEXT("no path, speed is 0");
	if (Speed > 0.f)
	{
		FPredictProjectilePathParams PathParams(CollisionRadius, Start, Direction * Speed, GetProjectileConfig().MaxSimTime, Channel, Avatar);
		PathParams.SimFrequency = FMath::Max(GetProjectileConfig().SimFrequency, 1.f);
		// PredictProjectilePath reads 0 as world gravity, so a projectile without gravity gets an unnoticeable one.
		PathParams.OverrideGravityZ = FMath::IsNearlyZero(GravityZ) ? -UE_KINDA_SMALL_NUMBER : GravityZ;
		if (Spawned)
			PathParams.ActorsToIgnore.Add(Spawned);

		FPredictProjectilePathResult Path;
		const bool bHit = UGameplayStatics::PredictProjectilePath(Avatar, PathParams, Path);
		for (int32 i = 1; i < Path.PathData.Num(); ++i)
			DebugLine(Path.PathData[i - 1].Location, Path.PathData[i].Location, PWModuleDebug::Direction);

		const FVector End = bHit ? FVector(Path.HitResult.ImpactPoint) : (Path.PathData.Num() ? Path.PathData.Last().Location : Start);
		const float FlightTime = Path.PathData.Num() ? Path.PathData.Last().Time : 0.f;
		const float Distance = FVector::Dist(Avatar->GetActorLocation(), End);
		DebugPoint(End, PWModuleDebug::Target);
		DebugText(End + FVector(0.f, 0.f, 30.f), FString::Printf(TEXT("%s %.2fs"), bHit ? TEXT("hit") : TEXT("end"), FlightTime), PWModuleDebug::Target);
		PathText = bHit
			? FString::Printf(TEXT("predicted to hit %s after %.2fs, %.0f from the caster"), *GetNameSafe(Path.HitResult.GetActor()), FlightTime, Distance)
			: FString::Printf(TEXT("predicted to hit nothing in %.1fs, ending %.0f from the caster"), GetProjectileConfig().MaxSimTime, Distance);
	}

	if (!Spawned)
		return;

	const FString Gravity = bGravityOverride
		? FString::Printf(TEXT("gravity override %.0f"), GravityZ)
		: FString::Printf(TEXT("gravity %.0f (world times %.2f)"), GravityZ, GravityScale);
	FString Line = FString::Printf(TEXT("launched at %.0f/s, %s, %s"), Speed, *Gravity, *PathText);
	if (RangeModule)
		Line += FString::Printf(TEXT(", range %.0f"), RangeModule->GetRange());
	DebugPrint(Line);

	// Whether the projectile's own movement took the launch it was given.
	if (const UProjectileMovementComponent* Movement = Spawned->FindComponentByClass<UProjectileMovementComponent>())
	{
		const float MovementSpeed = Movement->Velocity.Size();
		const float Angle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(Movement->Velocity.GetSafeNormal(), Direction), -1.0, 1.0)));
		if (!FMath::IsNearlyEqual(MovementSpeed, Speed, 1.f) || Angle > 1.f)
			DebugPrint(FString::Printf(TEXT("its movement component flies at %.0f/s, %.1f degrees off the launch direction, right after spawning"), MovementSpeed, Angle), FLinearColor::Yellow);
	}
}

AActor* UPWAbilityModule_Projectile::SpawnProjectileWithTargetingResult(const FPWTargetingResult& Target)
{
	return SpawnActorWithTargetingResult(Target);
}

void UPWAbilityModule_Projectile::SpawnProjectile()
{
	SpawnActor();
}

const FPWProjectileSimConfig& UPWAbilityModule_Projectile::GetProjectileConfig() const
{
	return bHasCurrentConfig ? CurrentConfig : ProjectileConfig;
}

void UPWAbilityModule_Projectile::SetProjectileConfig(const FPWProjectileSimConfig& NewConfig)
{
	CurrentConfig = NewConfig;
	bHasCurrentConfig = true;
	if (UPWAbilityModule_Precast* Precast = OwnerAbility ? OwnerAbility->GetPrecastModule() : nullptr)
		Precast->UpdateProjectileSimConfig(NewConfig);
	if (IsDebugging())
		DebugPrint(FString::Printf(TEXT("projectile config set for this cast: speed %.0f (configured %.0f), gravity %.0f (configured %.0f)"),
			GetProjectileSpeed(), GetBaseProjectileSpeed(), NewConfig.GravityZOverride, ProjectileConfig.GravityZOverride));
}

float UPWAbilityModule_Projectile::GetProjectileSpeed() const
{
	return GetProjectileSpeedAtLevel(GetOwnerLevel());
}

float UPWAbilityModule_Projectile::GetBaseProjectileSpeed() const
{
	return GetBaseProjectileSpeedAtLevel(GetOwnerLevel());
}

float UPWAbilityModule_Projectile::GetProjectileSpeedAtLevel(const int32 Level) const
{
	return GetProjectileConfig().InitialSpeed.GetValueAtLevel(Level);
}

float UPWAbilityModule_Projectile::GetBaseProjectileSpeedAtLevel(const int32 Level) const
{
	return ProjectileConfig.InitialSpeed.GetValueAtLevel(Level);
}

void UPWAbilityModule_Projectile::SetProjectileSpeed(const float NewSpeed)
{
	SetProjectileSpeedCurve(FScalableFloat(NewSpeed));
}

void UPWAbilityModule_Projectile::SetProjectileSpeedCurve(const FScalableFloat& NewSpeed)
{
	FPWProjectileSimConfig Config = GetProjectileConfig();
	Config.InitialSpeed = NewSpeed;
	SetProjectileConfig(Config);
}

void UPWAbilityModule_Projectile::ResetCurrentValues()
{
	Super::ResetCurrentValues();
	CurrentConfig = FPWProjectileSimConfig();
	bHasCurrentConfig = false;
}
