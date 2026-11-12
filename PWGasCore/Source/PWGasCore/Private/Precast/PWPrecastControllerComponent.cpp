// copyright mkurtt96


#include "Precast/PWPrecastControllerComponent.h"

#include "GAS/Abilities/Modules/ActionModules/PWActionModule.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "Precast/Data/PWPrecastTypes.h"
#include "Targeting/Data/PWTargetingData.h"
#include "PWGasCore/Public/Targeting/Types/PWTargetPolicyBase.h"

UPWPrecastControllerComponent::UPWPrecastControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPWPrecastControllerComponent::StartPrecast(UPWTargetingSource* InTargetingSource, UPWRangePolicy* InRangePolicy, UPWTargetResolver* InTargetResolver, UPWPrecastVisualizer* InVisualizer, float InPreviewRadius, float InPreviewRange, FName InOriginSocket, FPWProjectileSimConfig InProjectileConfig)
{
	APlayerController* PC = GetPC();
	if (!PC || !PC->IsLocalController()) return;

	StopPrecast();

	TargetingSource = InTargetingSource;
	RangePolicy = InRangePolicy;
	TargetResolver = InTargetResolver;
	Visualizer = InVisualizer;
	
	PreviewRadius = InPreviewRadius;
	PreviewRange = InPreviewRange;
	OriginSocket = InOriginSocket;
	ProjCfg = InProjectileConfig;

	if (Visualizer)
		Visualizer->Ensure(PC, PreviewRadius, OriginSocket);

	bRunning = true;
	SetComponentTickEnabled(true);
}

void UPWPrecastControllerComponent::StopPrecast()
{
	SetComponentTickEnabled(false);
	if (Visualizer)
		Visualizer->Hide();
	
	TargetingSource = nullptr;
	RangePolicy = nullptr;
	TargetResolver = nullptr;
	Visualizer = nullptr;
	ActionModule = nullptr;
	
	Result = FPWTargetingResult();
	bRunning = false;
}

void UPWPrecastControllerComponent::SetActionModule(UPWActionModule* InActionModule)
{
	ActionModule = InActionModule;
}

void UPWPrecastControllerComponent::UpdateProjectileSimConfig(const FPWProjectileSimConfig& Cfg)
{
	ProjCfg = Cfg;
}

void UPWPrecastControllerComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(false);
}

void UPWPrecastControllerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!bRunning) return;
	
	APlayerController* PC = GetPC();
	if (!PC)
	{
		StopPrecast();
		return;
	}

	if (TargetingSource) TargetingSource->Sample(PC, Result);
	if (RangePolicy) RangePolicy->Apply(GetPawn(), PreviewRange, Result);
	TArray<AActor*> OutActors;
	if (TargetResolver) TargetResolver->Resolve(PC, Result, OutActors);
	if (Visualizer)
	{
		FPWPrecastVisualUpdate VisualUpdate;
		VisualUpdate.Radius = PreviewRadius;
		VisualUpdate.ImpactPoint = Result.Location;
		VisualUpdate.ImpactNormal = FVector::UpVector;

		// projectile path computation
		if (ProjCfg.InitialSpeed > 0.f)
			BuildProjectileVisual(VisualUpdate);
		if (ActionModule.IsValid())
		{
			ActionModule->GetPreviewTransforms(Result, VisualUpdate.SpawnTransforms);
			VisualUpdate.ActorClass = ActionModule->GetPreviewActorClass();
		}
		Visualizer->Update(VisualUpdate);
	}
}

APlayerController* UPWPrecastControllerComponent::GetPC() const
{
	return Cast<APlayerController>(GetOwner());
}

APawn* UPWPrecastControllerComponent::GetPawn() const
{
	return GetPC() ? GetPC()->GetPawn() : nullptr;
}

FVector UPWPrecastControllerComponent::GetOriginLocation() const
{
	const APawn* P = GetPawn();
	if (!P) return FVector::ZeroVector;

	if (!OriginSocket.IsNone())
	{
		if (const USkeletalMeshComponent* Skel = P->FindComponentByClass<USkeletalMeshComponent>())
		{
			return Skel->GetSocketLocation(OriginSocket);
		}
	}
	const USceneComponent* Root = P->GetRootComponent();
	return Root ? Root->GetComponentLocation() : P->GetActorLocation();
}

void UPWPrecastControllerComponent::BuildProjectileVisual(FPWPrecastVisualUpdate& OutVisualUpdate)
{
	const FVector Start = GetOriginLocation();

	TArray<FVector> AimPoints;
	if (ActionModule.IsValid())
		ActionModule->GetPreviewAimPoints(Result, AimPoints);
	if (AimPoints.IsEmpty())
		AimPoints.Add(Result.Location);

	const int32 Middle = AimPoints.Num() / 2;
	OutVisualUpdate.Paths.Reset();
	for (int32 i = 0; i < AimPoints.Num(); ++i)
	{
		FHitResult Hit;
		OutVisualUpdate.Paths.Add(SimulatePath(Start, AimPoints[i], Hit));
		if (i == Middle)
		{
			Result.bHasHit = Hit.bBlockingHit;
			Result.Hit = Hit;
		}
	}

	const FPWPrecastPath& Main = OutVisualUpdate.Paths[Middle];
	OutVisualUpdate.PathPoints = Main.Points;
	OutVisualUpdate.ImpactPoint = Main.ImpactPoint;
	OutVisualUpdate.ImpactNormal = Main.ImpactNormal;
}

FPWPrecastPath UPWPrecastControllerComponent::SimulatePath(const FVector& Start, const FVector& AimPoint, FHitResult& OutHit) const
{
	FPredictProjectilePathParams Params;
	Params.bTraceWithCollision = true;
	Params.ProjectileRadius = ProjCfg.ProjectileRadius;
	Params.MaxSimTime = ProjCfg.MaxSimTime;
	Params.SimFrequency = ProjCfg.SimFrequency;
	Params.TraceChannel = ProjCfg.TraceChannel;
	Params.DrawDebugType = EDrawDebugTrace::None;
	if (!FMath::IsNearlyZero(ProjCfg.GravityZOverride))
		Params.OverrideGravityZ = ProjCfg.GravityZOverride;
	if (const APawn* Pawn = GetPawn())
		Params.ActorsToIgnore.Add(const_cast<APawn*>(Pawn));

	Params.StartLocation = Start;
	Params.LaunchVelocity = (AimPoint - Start).GetSafeNormal() * ProjCfg.InitialSpeed;

	FPredictProjectilePathResult PathResult;
	UGameplayStatics::PredictProjectilePath(GetWorld(), Params, PathResult);
	OutHit = PathResult.HitResult;

	FPWPrecastPath Path;
	for (const FPredictProjectilePathPointData& Point : PathResult.PathData)
		Path.Points.Add(Point.Location);

	Path.bHit = OutHit.bBlockingHit;
	Path.ImpactPoint = Path.bHit ? OutHit.ImpactPoint : (Path.Points.Num() ? Path.Points.Last() : AimPoint);
	Path.ImpactNormal = (Path.bHit && ProjCfg.bAlignToGroundNormal) ? OutHit.ImpactNormal : FVector::UpVector;
	return Path;
}
