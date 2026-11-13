// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "PWGasCore/Public/GAS/Abilities/Modules/ActionModules/PWAbilityModule_Actor.h"

#include "Ability/PWSpellParamsInterface.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "GAS/Abilities/Modules/ControlModules/PWAbilityModule_Targeting.h"
#include "GAS/AbilityTasks/PWAbilityTask_TargetFromSource.h"
#include "PWGASCoreLogChannels.h"
#include "HAL/IConsoleManager.h"
#include "Core/PWTeamsSubsystem.h"
#include "Agents/PWTeamAgentComponent.h"
#include "Libraries/PWGasFunctions.h"


void UPWAbilityModule_Actor::PostLoad()
{
	Super::PostLoad();

	// Abilities saved before tag filters kept a number.
	if (TargetMask_DEPRECATED == 15)
		return;
	if (TargetFilter.IsEmpty())
		TargetFilter = UPWGasFunctions::FilterFromLegacyMask(TargetMask_DEPRECATED);
	TargetMask_DEPRECATED = 15;
}

UPWSpellParams* UPWAbilityModule_Actor::MakeSpellParams_Implementation(const FPWTargetingResult& Target)
{
	if (!OwnerAbility)
		return nullptr;

	UPWSpellParams* Params = OwnerAbility->MakeSpellParams();
	UPWActorSpellParams* ActorParams = Params->AddExtension<UPWActorSpellParams>();
	ActorParams->SpawnLocation = Target.Location;
	// The rotation the actor really spawns with, from its socket, so anything that launches along it flies at the target.
	ActorParams->SpawnRotation = BuildSpawnTransform(Target).Rotator();
	ActorParams->bAttachToOwner = bAttachToOwner;
	ActorParams->AttachSocket = SpawnSocket;
	Params->CollisionFilter = TargetFilter;

	ExtendParamsWithData(Params);

	return Params;
}

AActor* UPWAbilityModule_Actor::SpawnActorWithTargetingResult(const FPWTargetingResult& Target)
{
	if (!ActorClass || !OwnerAbility)
		return nullptr;

	AActor* Avatar = OwnerAbility->GetAvatarActorFromActorInfo();
	if (!Avatar || !Avatar->HasAuthority())
		return nullptr;

	UWorld* World = Avatar->GetWorld();
	if (!World)
		return nullptr;

	FTransform SpawnTM = BuildSpawnTransform(Target);

	AActor* Spawned = World->SpawnActorDeferred<AActor>(ActorClass, SpawnTM, Avatar, Cast<APawn>(Avatar), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Spawned)
		return nullptr;

	if (!Spawned->GetClass()->ImplementsInterface(UPWSpellParamsInterface::StaticClass()))
	{
		UE_LOG(LogPWGASCoreAbilityModule, Error, TEXT("%s must implement IPWSpellParamsInterface (ActorModule)."), *ActorClass->GetName());
		Spawned->Destroy();
		return nullptr;
	}

	// A summon, totem or turret with a team agent is attributed to the caster's participant, whoever ends up controlling it.
	if (UPWTeamAgentComponent* Agent = Spawned->FindComponentByClass<UPWTeamAgentComponent>(); Agent && !Agent->GetOwnerParticipant().IsValid())
		Agent->SetOwnerParticipant(UPWTeamsSubsystem::GetAgentOwner(Avatar));

	if (const IConsoleVariable* DebugTargeting = IConsoleManager::Get().FindConsoleVariable(TEXT("PW.Debug.Targeting")); DebugTargeting && DebugTargeting->GetInt())
	{
		const FVector Aim = (Target.Location - Avatar->GetActorLocation()).GetSafeNormal();
		UE_LOG(LogPWGASCoreAbilityModule, Warning, TEXT("[Targeting] %s SPAWN for %s: caster=%s facing=%s spawnAt=%s rotation=%s target=%s | aim against facing=%.2f (negative is behind)"),
			*GetNameSafe(OwnerAbility), *GetNameSafe(Avatar), *Avatar->GetActorLocation().ToCompactString(), *Avatar->GetActorRotation().ToCompactString(), *SpawnTM.GetLocation().ToCompactString(),
			*SpawnTM.Rotator().ToCompactString(), *Target.Location.ToCompactString(), FVector::DotProduct(Aim, Avatar->GetActorForwardVector()));
	}

	if (bDebugSpawnPosition)
	{
		DrawDebugSphere(GetWorld(), SpawnTM.GetLocation(), 5, 16, FColor::Red, false, 3);
	}

	if (UPWSpellParams* ParamsObj = MakeSpellParams(Target))
	{
		IPWSpellParamsInterface::Execute_SetSpellParams(Spawned, ParamsObj);
	}

	SpawnedActors.Add(Spawned);

	Spawned->FinishSpawning(SpawnTM);

	if (bAttachToOwner && Avatar)
	{
		if (SpawnSocket != NAME_None)
		{
			if (USkeletalMeshComponent* Mesh = Avatar->FindComponentByClass<USkeletalMeshComponent>())
			{
				if (Mesh->DoesSocketExist(SpawnSocket))
					Spawned->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SpawnSocket);
				else
					UE_LOG(LogPWGASCoreAbilityModule, Warning, TEXT("Attach socket '%s' missing on %s."), *SpawnSocket.ToString(), *Avatar->GetName());
			}
		}
		else
		{
			Spawned->AttachToActor(Avatar, FAttachmentTransformRules::KeepWorldTransform);
		}
	}

	return Spawned;
}

void UPWAbilityModule_Actor::SpawnActor()
{
	if (!OwnerAbility)
		return;

	UPWAbilityModule_Targeting* Targeting = OwnerAbility->GetTargetingModule();
	if (!Targeting)
	{
		// Without a targeting module the caster is the target: spawn on it, or aim straight ahead of it.
		const AActor* Avatar = OwnerAbility->GetAvatarActorFromActorInfo();
		if (!Avatar)
			return;

		FPWTargetingResult Target;
		Target.Rotation = Avatar->GetActorRotation();
		Target.Location = Avatar->GetActorLocation() + (bSpawnAtTarget ? FVector::ZeroVector : Avatar->GetActorForwardVector() * 10000.f);

		StartAction();
		SpawnActorWithTargetingResult(Target);
		FinishAction();
		return;
	}

	ActiveTargetTask = UPWAbilityTask_TargetFromSource::TargetFromSource(OwnerAbility);
	if (!ActiveTargetTask)
		return;

	ActiveTargetTask->ValidData.AddDynamic(this, &UPWAbilityModule_Actor::OnTargetDataReady);

	StartAction();
	ActiveTargetTask->ReadyForActivation();
}

void UPWAbilityModule_Actor::DestroySpawnedActors()
{
	for (auto SpawnedActor : SpawnedActors)
	{
		if (SpawnedActor.IsValid())
			SpawnedActor->Destroy();
	}
	SpawnedActors.Empty();
}

FTransform UPWAbilityModule_Actor::BuildSpawnTransform(const FPWTargetingResult& Target) const
{
	AActor* Avatar = OwnerAbility ? OwnerAbility->GetAvatarActorFromActorInfo() : nullptr;
	if (!Avatar)
		return FTransform::Identity;

	FVector Loc = Avatar->GetActorLocation();
	FRotator Rot = (Target.Location - Loc).Rotation();

	if (bSpawnAtTarget)
	{
		Loc = Target.Location;
		Rot = Target.Rotation;
	}
	else if (SpawnSocket != NAME_None)
	{
		if (const APawn* Pawn = OwnerAbility->GetPawn())
		{
			if (const USkeletalMeshComponent* Mesh = Pawn->FindComponentByClass<USkeletalMeshComponent>())
			{
				if (Mesh->DoesSocketExist(SpawnSocket))
				{
					Loc = Mesh->GetSocketLocation(SpawnSocket);
					Rot = (Target.Location - Loc).Rotation();
				}
			}
		}
	}

	if (bZeroPitch)
		Rot.Pitch = 0.f;

	Loc += Rot.RotateVector(SpawnRelativeOffset);
	return FTransform(Rot, Loc);
}

void UPWAbilityModule_Actor::GetRequiredDataModules_Implementation(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules)
{
	Super::GetRequiredDataModules_Implementation(OutRequiredModules);
}

void UPWAbilityModule_Actor::OnTargetDataReady(const FGameplayAbilityTargetDataHandle& DataHandle, const FPWTargetingResult& Result)
{
	SpawnActorWithTargetingResult(Result);
	FinishAction();
}

void UPWAbilityModule_Actor::OnAbilityActivated_Implementation()
{
	if (bSpawnOnActivated && !OwnerAbility->DefersAction())
		SpawnActor();
}

void UPWAbilityModule_Actor::GetPreviewTransforms(const FPWTargetingResult& Target, TArray<FTransform>& OutTransforms) const
{
	if (bSpawnAtTarget)
		OutTransforms.Add(BuildSpawnTransform(Target));
}
