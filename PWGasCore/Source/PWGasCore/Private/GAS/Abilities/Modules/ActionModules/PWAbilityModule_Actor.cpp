// copyright mkurtt96


#include "PWGasCore/Public/GAS/Abilities/Modules/ActionModules/PWAbilityModule_Actor.h"

#include "Ability/PWSpellParamsInterface.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "GAS/Abilities/Modules/ControlModules/PWAbilityModule_Targeting.h"
#include "GAS/AbilityTasks/PWAbilityTask_TargetFromSource.h"


USpellParams* UPWAbilityModule_Actor::MakeSpellParams_Implementation(const FPWTargetingResult& Target)
{
	if (!OwnerAbility)
		return nullptr;

	USpellParams* Params = OwnerAbility->MakeSpellParams();
	UActorSpellParamsExtension* ActorParams = Params->AddExtension<UActorSpellParamsExtension>();
	ActorParams->SpawnLocation = Target.Location;
	ActorParams->SpawnRotation = (Target.Location - OwnerAbility->GetPawn()->GetActorLocation()).Rotation();
	ActorParams->bAttachToOwner = bAttachToOwner;
	ActorParams->AttachSocket = SpawnSocket;
	Params->TargetCollisionMask = TargetMask;

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
		UE_LOG(LogTemp, Error, TEXT("%s must implement IPWSpellParamsInterface (ActorModule)."), *ActorClass->GetName());
		Spawned->Destroy();
		return nullptr;
	}

	if (bDebugSpawnPosition)
	{
		DrawDebugSphere(GetWorld(), SpawnTM.GetLocation(), 5, 16, FColor::Red, false, 3);
	}

	if (USpellParams* ParamsObj = MakeSpellParams(Target))
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
					UE_LOG(LogTemp, Warning, TEXT("Attach socket '%s' missing on %s."), *SpawnSocket.ToString(), *Avatar->GetName());
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
		// no targeting fallback
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
	FVector Loc;
	FRotator Rot;

	AActor* Avatar = OwnerAbility ? OwnerAbility->GetAvatarActorFromActorInfo() : nullptr;
	if (!Avatar)
		return FTransform::Identity;


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
	else
	{
		Loc = Avatar->GetActorLocation();
		Rot = (Target.Location - Loc).Rotation();
	}

	if (bZeroPitch)
		Rot.Pitch = 0.f;

	Loc += Rot.RotateVector(SpawnRelativeOffset);
	return FTransform(Rot, Loc);
}

void UPWAbilityModule_Actor::GetRequiredDataModules_Implementation(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules)
{
	Super::GetRequiredDataModules_Implementation(OutRequiredModules);

	OutRequiredModules.Add(UPWAbilityModule_Targeting::StaticClass());
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
