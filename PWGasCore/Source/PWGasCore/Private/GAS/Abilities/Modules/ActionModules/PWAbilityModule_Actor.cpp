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

namespace PWActorModuleDebug
{
	static FString ClassName(const UClass* Class)
	{
		FString Name = GetNameSafe(Class);
		Name.RemoveFromEnd(TEXT("_C"));
		return Name;
	}

	static FString DescribeFilter(const FPWTeamTargetFilter& Filter)
	{
		if (Filter.IsEmpty())
			return TEXT("filter empty, collides with every target");

		FString Out = TEXT("filter");
		if (!Filter.AnyOf.IsEmpty())
			Out += FString::Printf(TEXT(" any of [%s]"), *Filter.AnyOf.ToStringSimple());
		if (!Filter.AllOf.IsEmpty())
			Out += FString::Printf(TEXT(" all of [%s]"), *Filter.AllOf.ToStringSimple());
		if (!Filter.NoneOf.IsEmpty())
			Out += FString::Printf(TEXT(" none of [%s]"), *Filter.NoneOf.ToStringSimple());
		return Out;
	}
}

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
	{
		if (IsDebugging())
			DebugPrint(TEXT("no Actor Class set, nothing spawned"), FLinearColor::Red);
		return nullptr;
	}

	AActor* Avatar = OwnerAbility->GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		if (IsDebugging())
			DebugPrint(TEXT("no avatar, nothing spawned"), FLinearColor::Red);
		return nullptr;
	}
	if (!Avatar->HasAuthority())
	{
		// Only the server spawns; this side still draws where its own aim puts the actor.
		if (IsDebugging())
		{
			DebugSpawn(Target, BuildSpawnTransform(Target), nullptr);
			DebugPrint(FString::Printf(TEXT("no authority, the server spawns %s; drawn where this side's aim puts it"), *PWActorModuleDebug::ClassName(ActorClass)),
				FLinearColor::Gray, PWModuleDebug::Duration, DebugKey(1));
		}
		return nullptr;
	}

	UWorld* World = Avatar->GetWorld();
	if (!World)
		return nullptr;

	FTransform SpawnTM = BuildSpawnTransform(Target);

	AActor* Spawned = World->SpawnActorDeferred<AActor>(ActorClass, SpawnTM, Avatar, Cast<APawn>(Avatar), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Spawned)
	{
		if (IsDebugging())
			DebugPrint(FString::Printf(TEXT("spawning %s at %s failed"), *PWActorModuleDebug::ClassName(ActorClass), *SpawnTM.GetLocation().ToCompactString()), FLinearColor::Red);
		return nullptr;
	}

	if (!Spawned->GetClass()->ImplementsInterface(UPWSpellParamsInterface::StaticClass()))
	{
		UE_LOG(LogPWGASCoreAbilityModule, Error, TEXT("%s must implement IPWSpellParamsInterface (ActorModule)."), *ActorClass->GetName());
		if (IsDebugging())
			DebugPrint(FString::Printf(TEXT("%s does not implement PWSpellParamsInterface, destroyed it"), *PWActorModuleDebug::ClassName(ActorClass)), FLinearColor::Red);
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

	if (UPWSpellParams* ParamsObj = MakeSpellParams(Target))
	{
		IPWSpellParamsInterface::Execute_SetSpellParams(Spawned, ParamsObj);
	}

	SpawnedActors.Add(Spawned);

	Spawned->FinishSpawning(SpawnTM);
	if (IsDebugging())
		DebugSpawn(Target, SpawnTM, Spawned);

	if (bAttachToOwner && Avatar)
	{
		if (SpawnSocket != NAME_None)
		{
			if (UMeshComponent* Mesh = UPWGasFunctions::FindSocketMesh(Avatar, SpawnSocket))
			{
				Spawned->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SpawnSocket);
				if (IsDebugging())
				{
					// Snapping moves the actor onto the socket, away from where it spawned.
					DebugLine(SpawnTM.GetLocation(), Spawned->GetActorLocation(), PWModuleDebug::Position);
					DebugPrint(FString::Printf(TEXT("attached to socket '%s' on %s, snapped to %s, %.0f from the spawn point"), *SpawnSocket.ToString(), *GetNameSafe(Mesh),
						*Spawned->GetActorLocation().ToCompactString(), FVector::Dist(SpawnTM.GetLocation(), Spawned->GetActorLocation())));
				}
			}
			else
			{
				UE_LOG(LogPWGASCoreAbilityModule, Warning, TEXT("Attach socket '%s' missing on %s."), *SpawnSocket.ToString(), *Avatar->GetName());
				if (IsDebugging())
					DebugPrint(FString::Printf(TEXT("attach socket '%s' missing on %s, left unattached"), *SpawnSocket.ToString(), *Avatar->GetName()), FLinearColor::Yellow);
			}
		}
		else
		{
			Spawned->AttachToActor(Avatar, FAttachmentTransformRules::KeepWorldTransform);
			if (IsDebugging())
				DebugPrint(FString::Printf(TEXT("attached to %s, keeping its world transform"), *Avatar->GetName()));
		}
	}

	return Spawned;
}

void UPWAbilityModule_Actor::SpawnActor()
{
	if (!OwnerAbility)
		return;

	if (IsDebugging())
		DebugPrint(FString::Printf(TEXT("%s: spawn at target %s, socket %s, offset %s, zero pitch %s, attach %s, %s"), *PWActorModuleDebug::ClassName(ActorClass),
			bSpawnAtTarget ? TEXT("on") : TEXT("off"), *SpawnSocket.ToString(), *SpawnRelativeOffset.ToCompactString(), bZeroPitch ? TEXT("on") : TEXT("off"),
			bAttachToOwner ? TEXT("on") : TEXT("off"), *PWActorModuleDebug::DescribeFilter(TargetFilter)));

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

		if (IsDebugging())
			DebugPrint(bSpawnAtTarget ? TEXT("no Targeting module, the caster is the target: spawning on it") : TEXT("no Targeting module, the caster is the target: aiming 10000 straight ahead"));

		StartAction();
		SpawnActorWithTargetingResult(Target);
		FinishAction();
		return;
	}

	ActiveTargetTask = UPWAbilityTask_TargetFromSource::TargetFromSource(OwnerAbility);
	if (!ActiveTargetTask)
	{
		if (IsDebugging())
			DebugPrint(TEXT("could not create the targeting task, nothing spawned"), FLinearColor::Red);
		return;
	}

	ActiveTargetTask->ValidData.AddDynamic(this, &UPWAbilityModule_Actor::OnTargetDataReady);

	if (IsDebugging())
		DebugPrint(OwnerAbility->IsLocallyControlled() ? TEXT("Targeting module aims on this side, spawning when it has a target") : TEXT("waiting for the client's aim from the Targeting module"));

	StartAction();
	ActiveTargetTask->ReadyForActivation();
}

void UPWAbilityModule_Actor::DestroySpawnedActors()
{
	int32 Destroyed = 0;
	for (auto SpawnedActor : SpawnedActors)
	{
		if (SpawnedActor.IsValid())
		{
			if (IsDebugging())
				DebugText(SpawnedActor->GetActorLocation(), TEXT("destroyed"), PWModuleDebug::Target);
			SpawnedActor->Destroy();
			Destroyed++;
		}
	}
	if (IsDebugging())
		DebugPrint(FString::Printf(TEXT("destroyed %d spawned actors, %d were already gone"), Destroyed, SpawnedActors.Num() - Destroyed));
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
			if (const UMeshComponent* Mesh = UPWGasFunctions::FindSocketMesh(Pawn, SpawnSocket))
			{
				Loc = Mesh->GetSocketLocation(SpawnSocket);
				Rot = (Target.Location - Loc).Rotation();
			}
		}
	}

	if (bZeroPitch)
		Rot.Pitch = 0.f;

	Loc += Rot.RotateVector(SpawnRelativeOffset);
	return FTransform(Rot, Loc);
}

void UPWAbilityModule_Actor::DebugSpawn(const FPWTargetingResult& Target, const FTransform& SpawnTM, AActor* Spawned) const
{
	const AActor* Avatar = OwnerAbility ? OwnerAbility->GetAvatarActorFromActorInfo() : nullptr;
	if (!IsDebugging() || !Avatar)
		return;

	// The point BuildSpawnTransform starts from.
	FVector From = Avatar->GetActorLocation();
	FString Source = bSpawnAtTarget ? TEXT("at the target") : TEXT("at the caster");
	if (!bSpawnAtTarget && SpawnSocket != NAME_None)
	{
		const APawn* Pawn = OwnerAbility->GetPawn();
		if (const UMeshComponent* Mesh = Pawn ? UPWGasFunctions::FindSocketMesh(Pawn, SpawnSocket) : nullptr)
		{
			From = Mesh->GetSocketLocation(SpawnSocket);
			Source = FString::Printf(TEXT("from socket '%s'"), *SpawnSocket.ToString());
		}
		else
			Source = FString::Printf(TEXT("at the caster, socket '%s' not found"), *SpawnSocket.ToString());
	}

	const FString ClassName = PWActorModuleDebug::ClassName(ActorClass);
	const FVector Location = SpawnTM.GetLocation();
	const bool bTargetAway = FVector::Dist(Location, Target.Location) > 1.f;

	DebugAxes(SpawnTM, 60.f);
	DebugPoint(Location, PWModuleDebug::Position);
	if (!From.Equals(Location, 1.f))
		DebugArrow(From, Location, PWModuleDebug::Position);
	DebugPoint(Target.Location, PWModuleDebug::Target);
	DebugArrow(Location, bTargetAway ? Target.Location : Location + SpawnTM.GetRotation().GetForwardVector() * 100.f, PWModuleDebug::Direction);
	if (bTargetAway)
		DebugText(Target.Location + FVector(0.f, 0.f, 30.f), TEXT("target"), PWModuleDebug::Target);
	DebugText(Location + FVector(0.f, 0.f, 60.f), Spawned ? ClassName : ClassName + TEXT(" (server spawns)"));

	if (!Spawned)
		return;

	FString Line = FString::Printf(TEXT("spawned %s at %s facing %s, %s"), *ClassName, *Location.ToCompactString(), *SpawnTM.Rotator().ToCompactString(), *Source);
	if (!SpawnRelativeOffset.IsZero())
		Line += FString::Printf(TEXT(", offset %s"), *SpawnRelativeOffset.ToCompactString());
	if (bZeroPitch)
		Line += TEXT(", pitch zeroed");
	if (bTargetAway)
		Line += FString::Printf(TEXT(", target %.0f away at %s"), FVector::Dist(Location, Target.Location), *Target.Location.ToCompactString());
	if (const AActor* TargetActor = Target.GetPrimaryActor())
		Line += FString::Printf(TEXT(", aim hit %s"), *TargetActor->GetName());
	DebugPrint(Line);

	if (!IsValid(Spawned))
		DebugPrint(FString::Printf(TEXT("%s destroyed itself while spawning"), *ClassName), FLinearColor::Red);
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
	if (!bSpawnOnActivated)
		return;

	if (!OwnerAbility->DefersAction())
		SpawnActor();
	else if (IsDebugging())
		DebugPrint(TEXT("Spawn On Activated is on, but the ability defers its action, so it spawns when the action runs"), FLinearColor::Yellow);
}

void UPWAbilityModule_Actor::GetPreviewTransforms(const FPWTargetingResult& Target, TArray<FTransform>& OutTransforms) const
{
	if (bSpawnAtTarget)
		OutTransforms.Add(BuildSpawnTransform(Target));
}
