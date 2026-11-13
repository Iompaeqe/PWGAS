// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "Precast/PWPrecastControllerComponent.h"

#include "AbilitySystemGlobals.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "GAS/Abilities/Modules/ActionModules/PWActionModule.h"
#include "GAS/Abilities/Modules/ActionModules/PWAbilityModule_Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Engine/EngineTypes.h"
#include "Libraries/PWGasFunctions.h"
#include "Ability/PWSpellCollision.h"
#include "Ability/PWAffectable.h"
#include "TeamFunctions.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "Precast/Visualizers/PWPrecastVisualizer.h"
#include "Targeting/Data/PWTargetingData.h"
#include "PWGasCore/Public/Targeting/Types/PWTargetPolicyBase.h"

UPWPrecastControllerComponent::UPWPrecastControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	// After movement and the camera, so the preview starts from where the pawn is this frame.
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
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
	bHasReleaseOrigin = false;
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

	// The preview collides with the size of the real projectile, so both touch a surface at the same point.
	ActorRadius = 0.f;
	ActorSphere = nullptr;
	ActorGravityScale = 1.f;
	if (const TSubclassOf<AActor> ActorClass = InActionModule ? InActionModule->GetPreviewActorClass() : nullptr)
		AActor::ForEachComponentOfActorClassDefault<USphereComponent>(ActorClass, [this](const USphereComponent* Sphere)
		{
			const float Radius = Sphere->GetUnscaledSphereRadius() * Sphere->GetRelativeScale3D().GetAbsMax();
			if (Radius > ActorRadius)
			{
				ActorRadius = Radius;
				ActorSphere = Sphere;
			}
			return true;
		});
	if (const TSubclassOf<AActor> ActorClass = InActionModule ? InActionModule->GetPreviewActorClass() : nullptr)
		AActor::ForEachComponentOfActorClassDefault<UProjectileMovementComponent>(ActorClass, [this](const UProjectileMovementComponent* Movement)
		{
			ActorGravityScale = Movement->ProjectileGravityScale;
			return false;
		});
}

void UPWPrecastControllerComponent::SetReleasePose(const UAnimMontage* Montage, const float Time)
{
	bHasReleaseOrigin = false;

	const APawn* Pawn = GetPawn();
	const USkeletalMeshComponent* Skel = Pawn ? Pawn->FindComponentByClass<USkeletalMeshComponent>() : nullptr;
	if (!Skel || !Montage || OriginSocket.IsNone() || Montage->SlotAnimTracks.IsEmpty())
		return;

	const FAnimSegment* Segment = Montage->SlotAnimTracks[0].AnimTrack.GetSegmentAtTime(Time);
	const UAnimSequence* Sequence = Segment ? Cast<UAnimSequence>(Segment->GetAnimReference()) : nullptr;
	const USkeleton* Skeleton = Sequence ? Sequence->GetSkeleton() : nullptr;
	if (!Skeleton)
		return;

	FName BoneName = OriginSocket;
	FTransform Pose = FTransform::Identity;
	if (const USkeletalMeshSocket* Socket = Skel->GetSocketByName(OriginSocket))
	{
		BoneName = Socket->BoneName;
		Pose = Socket->GetSocketLocalTransform();
	}

	// Walk from the bone up to the root, taking each bone from the animation at that time.
	const FReferenceSkeleton& Reference = Skeleton->GetReferenceSkeleton();
	const double AnimTime = Segment->ConvertTrackPosToAnimPos(Time);
	int32 Bone = Reference.FindBoneIndex(BoneName);
	if (Bone == INDEX_NONE)
		return;

	for (; Bone != INDEX_NONE; Bone = Reference.GetParentIndex(Bone))
	{
		FTransform BoneLocal = Reference.GetRefBonePose()[Bone];
		Sequence->GetBoneTransform(BoneLocal, FSkeletonPoseBoneIndex(Bone), AnimTime, false);
		Pose = Pose * BoneLocal;
	}

	ReleaseOrigin = Pose.GetLocation();
	bHasReleaseOrigin = true;
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
	if (Visualizer)
	{
		FPWPrecastVisualUpdate VisualUpdate;
		VisualUpdate.Radius = PreviewRadius;
		VisualUpdate.ImpactPoint = Result.Location;
		VisualUpdate.ImpactNormal = FVector::UpVector;

		TArray<FPWTargetingResult> Impacts;
		// projectile path computation
		if (ProjCfg.InitialSpeed > 0.f)
			BuildProjectileVisual(VisualUpdate, Impacts);
		if (Impacts.IsEmpty())
			Impacts.Add(Result);
		GatherTargets(Impacts, VisualUpdate.Targets);
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
			if (bHasReleaseOrigin)
				return Skel->GetComponentTransform().TransformPosition(ReleaseOrigin);
			return Skel->GetSocketLocation(OriginSocket);
		}
	}
	const USceneComponent* Root = P->GetRootComponent();
	return Root ? Root->GetComponentLocation() : P->GetActorLocation();
}

void UPWPrecastControllerComponent::BuildProjectileVisual(FPWPrecastVisualUpdate& OutVisualUpdate, TArray<FPWTargetingResult>& OutImpacts)
{
	const FVector Start = GetOriginLocation();

	TArray<FVector> AimPoints;
	if (ActionModule.IsValid())
		ActionModule->GetPreviewAimPoints(Result, AimPoints);
	if (AimPoints.IsEmpty())
		AimPoints.Add(Result.Location);

	// The main path is the shot aimed closest to the target, whatever order the pattern fires in.
	int32 Middle = 0;
	for (int32 i = 1; i < AimPoints.Num(); ++i)
		if (FVector::DistSquared(AimPoints[i], Result.Location) < FVector::DistSquared(AimPoints[Middle], Result.Location))
			Middle = i;
	OutVisualUpdate.Paths.Reset();
	for (int32 i = 0; i < AimPoints.Num(); ++i)
	{
		FHitResult Hit;
		OutVisualUpdate.Paths.Add(SimulatePath(Start, AimPoints[i], Hit));

		FPWTargetingResult& Impact = OutImpacts.Add_GetRef(Result);
		Impact.Location = OutVisualUpdate.Paths.Last().ImpactPoint;
		Impact.Hit = Hit;
		Impact.bHasHit = Hit.bBlockingHit;
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

void UPWPrecastControllerComponent::GatherTargets(const TArray<FPWTargetingResult>& Impacts, TArray<TObjectPtr<AActor>>& OutTargets) const
{
	const APawn* Pawn = GetPawn();
	if (!TargetResolver || !Pawn)
		return;

	const UPWModularGameplayAbility* Ability = ActionModule.IsValid() ? ActionModule->OwnerAbility.Get() : nullptr;
	const UPWAbilityModule_Effect* Effect = Ability ? Ability->GetEffectModule() : nullptr;

	for (const FPWTargetingResult& Impact : Impacts)
	{
		TArray<AActor*> Found;
		TargetResolver->Resolve(Pawn, Impact, Found);
		for (AActor* Actor : Found)
		{
			if (!IsValid(Actor) || OutTargets.Contains(Actor))
				continue;
			if (!UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor) && !IPWAffectable::Find(Actor))
				continue;
			if (Effect && !UTeamFunctions::MatchesMask(Pawn, Actor, Effect->GetCombinedMask()))
				continue;
			OutTargets.Add(Actor);
		}
	}
}

FPWPrecastPath UPWPrecastControllerComponent::SimulatePath(const FVector& Start, const FVector& AimPoint, FHitResult& OutHit) const
{
	FPWPrecastPath Path;
	OutHit = FHitResult();

	const UWorld* World = GetWorld();
	if (!World)
		return Path;

	const APawn* Pawn = GetPawn();
	const UPWAbilityModule_Actor* ActorModule = Cast<UPWAbilityModule_Actor>(ActionModule.Get());
	const ECollisionChannel ProjectileChannel = ActorSphere.IsValid() ? ActorSphere->GetCollisionObjectType() : ProjCfg.TraceChannel.GetValue();
	const float GravityZ = FMath::IsNearlyZero(ProjCfg.GravityZOverride) ? World->GetGravityZ() * ActorGravityScale : ProjCfg.GravityZOverride;
	const float Step = 1.f / FMath::Max(ProjCfg.SimFrequency, 1.f);

	const FCollisionShape Shape = FCollisionShape::MakeSphere(FMath::Max(ActorRadius, 1.f));
	const FCollisionObjectQueryParams Objects(FCollisionObjectQueryParams::InitType::AllObjects);
	const FCollisionQueryParams Query(SCENE_QUERY_STAT(PWPrecastPath), false, Pawn);

	// The same rules the real projectile uses to decide what it hits.
	const auto StopsAt = [&](const FHitResult& Hit)
	{
		const UPrimitiveComponent* Component = Hit.GetComponent();
		AActor* Actor = Hit.GetActor();
		if (!Component || !Actor)
			return false;
		if (Component->Implements<UPWDontTriggerCollision>() || Actor->Implements<UPWDontTriggerCollision>())
			return false;
		if (ActorSphere.IsValid() && ActorSphere->GetCollisionResponseToChannel(Component->GetCollisionObjectType()) == ECR_Ignore)
			return false;
		if (Component->GetCollisionResponseToChannel(ProjectileChannel) == ECR_Ignore)
			return false;
		if (!IPWAffectable::Find(Actor))
			return true;
		return !ActorModule || !Pawn || UTeamFunctions::MatchesMask(Pawn, Actor, ActorModule->TargetMask);
	};

	FVector Position = Start;
	FVector Velocity = (AimPoint - Start).GetSafeNormal() * ProjCfg.InitialSpeed;
	Path.Points.Add(Position);

	for (float Time = 0.f; Time < ProjCfg.MaxSimTime && !Path.bHit; Time += Step)
	{
		const FVector Next = Position + Velocity * Step + FVector(0.f, 0.f, 0.5f * GravityZ * Step * Step);
		Velocity.Z += GravityZ * Step;

		TArray<FHitResult> Hits;
		World->SweepMultiByObjectType(Hits, Position, Next, FQuat::Identity, Objects, Shape, Query);
		for (const FHitResult& Hit : Hits)
		{
			if (!StopsAt(Hit))
				continue;
			OutHit = Hit;
			OutHit.bBlockingHit = true;
			Path.bHit = true;
			break;
		}

		Position = Path.bHit ? OutHit.Location : Next;
		Path.Points.Add(Position);
	}

	Path.ImpactPoint = Path.bHit ? OutHit.ImpactPoint : Path.Points.Last();
	Path.ImpactNormal = (Path.bHit && ProjCfg.bAlignToGroundNormal) ? OutHit.ImpactNormal : FVector::UpVector;
	return Path;
}
