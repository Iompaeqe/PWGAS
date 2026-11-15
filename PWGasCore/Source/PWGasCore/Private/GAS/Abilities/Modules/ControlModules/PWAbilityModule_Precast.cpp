// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "PWGasCore/Public/GAS/Abilities/Modules/ControlModules/PWAbilityModule_Precast.h"

#include "Animation/AnimMontage.h"
#include "Components/MeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "GAS/Abilities/Modules/ActionModules/PWActionModule.h"
#include "GAS/Abilities/Modules/ActionModules/PWAbilityModule_Projectile.h"
#include "GAS/Abilities/Modules/ControlModules/PWAbilityModule_Resolver.h"
#include "GAS/Abilities/Modules/ControlModules/PWAbilityModule_Targeting.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Radius.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Range.h"
#include "Libraries/PWGasFunctions.h"
#include "Precast/PWPrecastControllerComponent.h"
#include "Precast/Visualizers/PWPrecastVisualizer.h"
#include "Precast/Visualizers/PWPrecastVisualizer_Group.h"
#include "TimerManager.h"

// The debug redraws this often; each drawing lasts a little longer so it doesn't flicker.
static constexpr float PrecastDebugInterval = 0.05f;
static constexpr float PrecastDebugLifetime = 0.07f;
static constexpr float PrecastDebugScreenTime = 0.25f;

static FString DescribePrecastVisualizer(const UPWPrecastVisualizer* Visualizer)
{
	if (!Visualizer)
		return TEXT("none");

	FString Name = Visualizer->GetClass()->GetName();
	if (const UPWPrecastVisualizer_Group* Group = Cast<UPWPrecastVisualizer_Group>(Visualizer))
	{
		TArray<FString> Children;
		for (const UPWPrecastVisualizer* Child : Group->Visualizers)
			Children.Add(DescribePrecastVisualizer(Child));
		Name += FString::Printf(TEXT(" (%s)"), *FString::Join(Children, TEXT(", ")));
	}
	return Name;
}

void UPWAbilityModule_Precast::StartPrecast()
{
	if (!OwnerAbility) return;

	const APlayerController* PC = GetPC();
	if (!PC || !PC->IsLocalController())
	{
		if (IsDebugging())
			DebugPrint(TEXT("no precast here, it only runs on the local player's controller"));
		return;
	}

	if (UPWPrecastControllerComponent* C = GetPrecastController())
	{
		UPWTargetingSource* TargetingSource = nullptr;
		UPWRangePolicy* RangePolicy = nullptr;
		UPWTargetResolver* TargetResolver = nullptr;
		FName OriginSocket = NAME_None;
		FPWProjectileSimConfig ProjectileConfig;
		float ProjectileSpeed = 0.f;
		// Area and range come from their modules; an ability without them has neither.
		float PreviewRadius = 0.f;
		float PreviewRange = 0.f;

		if (const UPWAbilityModule_Targeting* TargetingModule = OwnerAbility->GetTargetingModule())
		{
			TargetingModule->InitializePolicies();
			TargetingSource = TargetingModule->TargetingSource;
		}

		if (const UPWAbilityModule_Resolver* ResolverModule = OwnerAbility->GetResolverModule())
		{
			TargetResolver = ResolverModule->Resolver;
			if (TargetResolver)
				TargetResolver->Initialize(OwnerAbility);
		}
		if (const UPWAbilityModule_Range* RangeModule = OwnerAbility->GetRangeModule())
		{
			RangePolicy = RangeModule->RangePolicy;
			PreviewRange = RangeModule->GetRange();
		}

		if (const UPWAbilityModule_Projectile* ProjectileModule = OwnerAbility->GetProjectileModule())
		{
			OriginSocket = ProjectileModule->SpawnSocket;
			ProjectileConfig = ProjectileModule->GetProjectileConfig();
			ProjectileSpeed = ProjectileModule->GetProjectileSpeed();
		}
		// The controller knows no ability level, so it gets the speed worked out for this one.
		ProjectileConfig.InitialSpeed = ProjectileSpeed;

		if (const UPWAbilityModule_Radius* RadiusModule = OwnerAbility->GetRadiusModule())
		{
			PreviewRadius = RadiusModule->GetRadius();
		}

		C->StartPrecast(
			TargetingSource,
			RangePolicy,
			TargetResolver,
			Visualizer,
			PreviewRadius,
			PreviewRange,
			OriginSocket,
			ProjectileConfig
		);
		C->SetActionModule(OwnerAbility->GetActionModule<UPWActionModule>());

		UAnimMontage* Montage = nullptr;
		if (ReleaseAnimationIndex >= 0)
		{
			float Rate = 1.f;
			OwnerAbility->GetAnimMontageFromActor(Montage, Rate, ReleaseAnimationIndex);
			C->SetReleasePose(Montage, ReleaseTime);
		}
		bRunning = true;

		if (IsDebugging())
		{
			const UPWActionModule* ActionModule = OwnerAbility->GetActionModule<UPWActionModule>();
			const FString Gravity = FMath::IsNearlyZero(ProjectileConfig.GravityZOverride)
				? FString(TEXT("world gravity times the projectile's scale"))
				: FString::Printf(TEXT("%.0f"), ProjectileConfig.GravityZOverride);
			const FString Projectile = ProjectileSpeed > 0.f
				? FString::Printf(TEXT("projectile speed %.0f, gravity %s, %.1f s of path"), ProjectileSpeed, *Gravity, ProjectileConfig.MaxSimTime)
				: FString(TEXT("no projectile path"));
			const FString Release = ReleaseAnimationIndex >= 0
				? FString::Printf(TEXT("release pose from animation %d (%s) at %.2f s"), ReleaseAnimationIndex, *GetNameSafe(Montage), ReleaseTime)
				: FString(TEXT("previews from where the hand is now"));

			DebugPrint(FString::Printf(TEXT("start: visualizer %s, preview radius %.0f, range %.0f"), *DescribePrecastVisualizer(Visualizer.Get()), PreviewRadius, PreviewRange));
			DebugPrint(FString::Printf(TEXT("origin socket %s, %s, %s"), *OriginSocket.ToString(), *Projectile, *Release));
			DebugPrint(FString::Printf(TEXT("source %s, range policy %s, resolver %s, action %s"),
				*GetNameSafe(TargetingSource ? TargetingSource->GetClass() : nullptr), *GetNameSafe(RangePolicy ? RangePolicy->GetClass() : nullptr),
				*GetNameSafe(TargetResolver ? TargetResolver->GetClass() : nullptr), *GetNameSafe(ActionModule ? ActionModule->GetClass() : nullptr)));

			if (UWorld* World = OwnerAbility->GetWorld())
				World->GetTimerManager().SetTimer(DebugTimer, this, &UPWAbilityModule_Precast::DebugTick, PrecastDebugInterval, true);
		}
	}
	else if (IsDebugging())
		DebugPrint(FString::Printf(TEXT("no precast: %s has no Precast Controller Component"), *GetNameSafe(PC)), FLinearColor::Yellow);
}

void UPWAbilityModule_Precast::StopPrecast()
{
	ClearDebugTimer();

	const APlayerController* PC = GetPC();
	if (!PC || !PC->IsLocalController()) return;

	if (UPWPrecastControllerComponent* C = GetPrecastController())
	{
		CachedResult = C->GetTargetingResult();
		C->StopPrecast();

		if (IsDebugging())
		{
			if (CachedResult.IsValid())
			{
				const APawn* Pawn = GetPawn();
				const FVector Caster = Pawn ? Pawn->GetActorLocation() : CachedResult.Location;
				DebugPoint(CachedResult.Location, PWModuleDebug::Target);
				DebugText(CachedResult.Location + FVector(0.f, 0.f, 40.f), TEXT("precast result"), PWModuleDebug::Target);
				DebugPrint(FString::Printf(TEXT("stop: result %s, %.0f from the caster, rotation %s, hit %s, %d hit actors"),
					*CachedResult.Location.ToCompactString(), FVector::Dist(Caster, CachedResult.Location), *CachedResult.Rotation.ToCompactString(),
					CachedResult.Hit.bBlockingHit ? *GetNameSafe(CachedResult.Hit.GetActor()) : TEXT("nothing"), CachedResult.HitActors.Num()));
			}
			else
				DebugPrint(TEXT("stop: nothing was aimed at"), FLinearColor::Yellow);
		}
	}
	bRunning = false;
}

void UPWAbilityModule_Precast::OnAbilityEnded_Implementation()
{
	if (bRunning)
		StopPrecast();
	ClearDebugTimer();
}

void UPWAbilityModule_Precast::UpdateProjectileSimConfig(const FPWProjectileSimConfig& Cfg)
{
	FPWProjectileSimConfig AtLevel = Cfg;
	const float Speed = Cfg.InitialSpeed.GetValueAtLevel(GetOwnerLevel());
	AtLevel.InitialSpeed = Speed;
	if (UPWPrecastControllerComponent* C = GetPrecastController())
		C->UpdateProjectileSimConfig(AtLevel);

	if (IsDebugging())
		DebugPrint(FString::Printf(TEXT("projectile config now speed %.0f, gravity %.0f, %.1f s of path"), Speed, Cfg.GravityZOverride, Cfg.MaxSimTime));
}

UPWPrecastControllerComponent* UPWAbilityModule_Precast::GetPrecastController() const
{
	if (const APlayerController* PC = GetPC())
		return PC->FindComponentByClass<UPWPrecastControllerComponent>();
	return nullptr;
}

void UPWAbilityModule_Precast::DebugTick()
{
	const UPWPrecastControllerComponent* C = GetPrecastController();
	const APawn* Pawn = GetPawn();
	if (!bRunning || !C || !Pawn || !IsDebugging())
	{
		ClearDebugTimer();
		return;
	}

	const FPWTargetingResult Result = C->GetTargetingResult();
	if (!Result.IsValid())
	{
		DebugPrint(TEXT("aiming: no target yet"), FLinearColor::Yellow, PrecastDebugScreenTime, DebugKey());
		return;
	}

	const float Time = PrecastDebugLifetime;
	const FVector Caster = Pawn->GetActorLocation();
	const UPWAbilityModule_Projectile* ProjectileModule = OwnerAbility->GetProjectileModule();

	// A projectile's preview sits where its path lands, not where it is aimed.
	FPWTargetingResult Preview = Result;
	const bool bLands = ProjectileModule && ProjectileModule->GetProjectileSpeed() > 0.f && Result.Hit.bBlockingHit;
	if (bLands)
		Preview.Location = Result.Hit.ImpactPoint;

	DebugArrow(Caster, Result.Location, PWModuleDebug::Direction, Time);
	DebugPoint(Result.Location, PWModuleDebug::Target, Time);
	if (bLands)
	{
		DebugLine(Result.Location, Preview.Location, PWModuleDebug::Target, Time);
		DebugPoint(Preview.Location, PWModuleDebug::Target, Time);
	}

	if (const UPWAbilityModule_Radius* RadiusModule = OwnerAbility->GetRadiusModule())
	{
		const float Radius = RadiusModule->GetRadius();
		RadiusModule->DebugDrawAt(Preview.Location, TEXT("precast"), Time);
		DebugSphere(Preview.Location, Radius, PWModuleDebug::Radius, Time);
		DebugCircle(Preview.Location, Radius, PWModuleDebug::Radius, Time);
	}
	if (const UPWAbilityModule_Range* RangeModule = OwnerAbility->GetRangeModule())
	{
		RangeModule->DebugDrawFrom(Caster, TEXT("precast"), Time);
		DebugCircle(Caster, RangeModule->GetRange(), PWModuleDebug::Range, Time);
	}

	FVector Origin = Caster;
	const UMeshComponent* Mesh = ProjectileModule && !ProjectileModule->SpawnSocket.IsNone() ? UPWGasFunctions::FindSocketMesh(Pawn, ProjectileModule->SpawnSocket) : nullptr;
	if (Mesh)
	{
		Origin = Mesh->GetSocketLocation(ProjectileModule->SpawnSocket);
		DebugPoint(Origin, PWModuleDebug::Position, Time);
	}

	if (const UPWActionModule* ActionModule = OwnerAbility->GetActionModule<UPWActionModule>())
	{
		TArray<FVector> AimPoints;
		ActionModule->GetPreviewAimPoints(Result, AimPoints);
		for (const FVector& Point : AimPoints)
		{
			DebugLine(Origin, Point, PWModuleDebug::Direction, Time);
			DebugPoint(Point, PWModuleDebug::Direction, Time);
		}

		TArray<FTransform> SpawnTransforms;
		ActionModule->GetPreviewTransforms(Result, SpawnTransforms);
		for (const FTransform& Transform : SpawnTransforms)
			DebugAxes(Transform, 50.f, Time);
	}

	FString Line = FString::Printf(TEXT("aiming %.0f away"), FVector::Dist(Caster, Result.Location));
	if (bLands)
		Line += FString::Printf(TEXT(", lands %.0f away"), FVector::Dist(Caster, Preview.Location));
	Line += FString::Printf(TEXT(", hit %s"), Result.Hit.bBlockingHit ? *GetNameSafe(Result.Hit.GetActor()) : TEXT("nothing"));

	const UPWAbilityModule_Resolver* ResolverModule = OwnerAbility->GetResolverModule();
	if (ResolverModule && ResolverModule->Resolver)
	{
		TArray<AActor*> Found;
		ResolverModule->Resolver->Resolve(Pawn, Preview, Found);
		for (const AActor* Actor : Found)
			if (IsValid(Actor))
				DebugPoint(Actor->GetActorLocation(), PWModuleDebug::Target, Time);
		Line += FString::Printf(TEXT(", %d found by the resolver"), Found.Num());
	}
	DebugPrint(Line, FLinearColor(0.f, 0.66f, 1.f), PrecastDebugScreenTime, DebugKey());
}

void UPWAbilityModule_Precast::ClearDebugTimer()
{
	if (UWorld* World = OwnerAbility ? OwnerAbility->GetWorld() : nullptr)
		World->GetTimerManager().ClearTimer(DebugTimer);
}
