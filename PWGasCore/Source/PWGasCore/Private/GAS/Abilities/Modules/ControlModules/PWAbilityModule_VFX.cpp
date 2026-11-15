// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/Abilities/Modules/ControlModules/PWAbilityModule_VFX.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "Components/MeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Radius.h"
#include "GAS/Abilities/Values/PWAbilityValue.h"
#include "GAS/ASC/PWAbilitySystemComponent.h"
#include "GAS/ASC/PWASC_VfxLifecycle.h"
#include "Libraries/PWGasFunctions.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

void UPWAbilityModule_VFX::PostLoad()
{
	Super::PostLoad();

	// Abilities saved while this was the Niagara module kept a plain list of systems.
	for (const FPWNiagaraEntry& Old : NiagaraEntries_DEPRECATED)
	{
		FPWVFXEntry& Entry = Entries.AddDefaulted_GetRef();
		Entry.NiagaraSystem = Old.NiagaraSystem;
		Entry.bLooping = Old.bLooping;
		Entry.bAttach = Old.bAttachToActor;
		Entry.Socket = Old.AttachSocketName;
		Entry.LocationOffset = Old.RelativeOffset;
	}
	NiagaraEntries_DEPRECATED.Reset();
}

int32 UPWAbilityModule_VFX::ComputeUniqueId(const int32 Index) const
{
	if (!OwnerAbility)
		return INDEX_NONE;

	// The spec handle with the entry index in the low byte.
	return (FCString::Atoi(*OwnerAbility->GetCurrentAbilitySpecHandle().ToString()) << 8) | (Index & 0xFF);
}

float UPWAbilityModule_VFX::GetParameterValue(const int32 Index, const int32 ParameterIndex) const
{
	if (!Entries.IsValidIndex(Index) || !Entries[Index].Parameters.IsValidIndex(ParameterIndex))
		return 0.f;

	const UPWAbilityValue* Value = Entries[Index].Parameters[ParameterIndex].Value;
	return Value ? Value->Evaluate(OwnerAbility) : 0.f;
}

FPWNiagaraEntry UPWAbilityModule_VFX::MakePlay(const int32 Index) const
{
	const FPWVFXEntry& Entry = Entries[Index];
	FPWNiagaraEntry Played;
	Played.NiagaraSystem = Entry.NiagaraSystem;
	Played.bLooping = Entry.bLooping;
	Played.Scale = Entry.Scale;
	Played.UniqueId = ComputeUniqueId(Index);
	for (int32 i = 0; i < Entry.Parameters.Num(); ++i)
	{
		if (Entry.Parameters[i].Name.IsNone())
			continue;
		FPWNiagaraFloat& Parameter = Played.FloatParameters.AddDefaulted_GetRef();
		Parameter.Name = Entry.Parameters[i].Name;
		Parameter.Value = GetParameterValue(Index, i);
	}
	return Played;
}

UNiagaraComponent* UPWAbilityModule_VFX::Play(const int32 Index)
{
	const AActor* Avatar = OwnerAbility ? OwnerAbility->GetAvatarActorFromActorInfo() : nullptr;
	if (!Avatar || !Entries.IsValidIndex(Index))
	{
		if (IsDebugging())
			DebugPrint(FString::Printf(TEXT("play [%d] skipped: %s"), Index, Avatar ? *FString::Printf(TEXT("there are %d entries"), Entries.Num()) : TEXT("no caster")), FLinearColor::Yellow);
		return nullptr;
	}

	const FPWVFXEntry& Entry = Entries[Index];
	FPWNiagaraEntry Played = MakePlay(Index);
	if (Entry.bAttach)
	{
		Played.bAttachToActor = true;
		Played.AttachSocketName = Entry.Socket;
		Played.RelativeOffset = Entry.LocationOffset;
		Played.RelativeRotation = Entry.RotationOffset;
		return InternalPlay(Index, Played, TEXT("attached to the caster"));
	}

	const UMeshComponent* Mesh = UPWGasFunctions::FindSocketMesh(Avatar, Entry.Socket);
	const FVector Origin = Mesh ? Mesh->GetSocketLocation(Entry.Socket) : Avatar->GetActorLocation();
	const FQuat Facing = Avatar->GetActorQuat();
	Played.WorldTransform = FTransform(Facing * Entry.RotationOffset.Quaternion(), Origin + Facing.RotateVector(Entry.LocationOffset));
	return InternalPlay(Index, Played, Mesh ? TEXT("at the socket, in the caster's facing") : TEXT("at the caster"));
}

UNiagaraComponent* UPWAbilityModule_VFX::PlayAtLocation(const int32 Index, const FTransform& WorldTransform)
{
	if (!Entries.IsValidIndex(Index))
	{
		if (IsDebugging())
			DebugPrint(FString::Printf(TEXT("play [%d] at location skipped: there are %d entries"), Index, Entries.Num()), FLinearColor::Yellow);
		return nullptr;
	}

	const FPWVFXEntry& Entry = Entries[Index];
	FPWNiagaraEntry Played = MakePlay(Index);
	const FQuat Rotation = WorldTransform.GetRotation();
	Played.WorldTransform = FTransform(Rotation * Entry.RotationOffset.Quaternion(), WorldTransform.GetLocation() + Rotation.RotateVector(Entry.LocationOffset));
	Played.Scale *= WorldTransform.GetScale3D();
	return InternalPlay(Index, Played, TEXT("at the given transform"));
}

UNiagaraComponent* UPWAbilityModule_VFX::PlayOnActor(const int32 Index, AActor* TargetActor)
{
	if (!TargetActor || !Entries.IsValidIndex(Index))
	{
		if (IsDebugging())
			DebugPrint(FString::Printf(TEXT("play [%d] on actor skipped: %s"), Index, TargetActor ? *FString::Printf(TEXT("there are %d entries"), Entries.Num()) : TEXT("no target actor")), FLinearColor::Yellow);
		return nullptr;
	}

	// The caster with Attach on follows it, like Play.
	if (Entries[Index].bAttach && OwnerAbility && TargetActor == OwnerAbility->GetAvatarActorFromActorInfo())
		return Play(Index);
	return PlayAtLocation(Index, FTransform(TargetActor->GetActorQuat(), TargetActor->GetActorLocation()));
}

UNiagaraComponent* UPWAbilityModule_VFX::InternalPlay(const int32 Index, FPWNiagaraEntry& Played, const TCHAR* From)
{
	const FGameplayAbilityActorInfo* Info = OwnerAbility ? OwnerAbility->GetCurrentActorInfo() : nullptr;
	UPWAbilitySystemComponent* ASC = OwnerAbility ? Cast<UPWAbilitySystemComponent>(OwnerAbility->GetAbilitySystemComponentFromActorInfo()) : nullptr;
	if (!Info || !ASC || !Played.NiagaraSystem)
	{
		if (IsDebugging())
			DebugPrint(FString::Printf(TEXT("play [%d] skipped: %s"), Index, !Played.NiagaraSystem ? TEXT("the entry has no Niagara System") : TEXT("the caster has no PW ability system component")), FLinearColor::Yellow);
		return nullptr;
	}

	if (Played.bLooping)
		ActiveIndexToId.FindOrAdd(Index) = Played.UniqueId;

	const bool bIsLocallyControlled = Info->IsLocallyControlled();
	UNiagaraComponent* LocalComp = nullptr;

	// The caster plays its own copy at once; the authority sends it to everyone else.
	if (bIsLocallyControlled)
		LocalComp = ASC->Vfx().PlayNiagara_Local(Played);

	const bool bOwnerPlays = bIsLocallyControlled || OwnerAbility->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	if (Info->IsNetAuthority())
		ASC->Multicast_PlayNiagara(Played, bOwnerPlays);

	if (IsDebugging())
		DebugPlay(Index, Played, From, LocalComp, bOwnerPlays);

	return LocalComp;
}

void UPWAbilityModule_VFX::PlayTimed(const EPWVFXTiming Timing)
{
	TArray<int32> Timed;
	for (int32 i = 0; i < Entries.Num(); ++i)
		if (Entries[i].PlayOn == Timing)
			Timed.Add(i);
	if (Timed.IsEmpty())
		return;

	if (IsDebugging())
		DebugPrint(FString::Printf(TEXT("%s: playing %s"), *StaticEnum<EPWVFXTiming>()->GetNameStringByValue(static_cast<int64>(Timing)),
			*FString::JoinBy(Timed, TEXT(", "), [](const int32 i) { return FString::Printf(TEXT("[%d]"), i); })));
	for (const int32 i : Timed)
		Play(i);
}

void UPWAbilityModule_VFX::OnAbilityActivated_Implementation()
{
	Super::OnAbilityActivated_Implementation();
	PlayTimed(EPWVFXTiming::Activated);
}

void UPWAbilityModule_VFX::OnAbilityPhase_Implementation(const EPWAbilityPhase Phase)
{
	Super::OnAbilityPhase_Implementation(Phase);
	switch (Phase)
	{
	case EPWAbilityPhase::Started:
		PlayTimed(EPWVFXTiming::Started);
		break;
	case EPWAbilityPhase::Pulse:
		PlayTimed(EPWVFXTiming::Pulse);
		break;
	case EPWAbilityPhase::Completed:
		PlayTimed(EPWVFXTiming::Completed);
		break;
	case EPWAbilityPhase::Cancelled:
		PlayTimed(EPWVFXTiming::Cancelled);
		break;
	}
}

void UPWAbilityModule_VFX::OnAbilityEnded_Implementation()
{
	Super::OnAbilityEnded_Implementation();

	TArray<int32> Ids;
	for (auto It = ActiveIndexToId.CreateIterator(); It; ++It)
	{
		if (Entries.IsValidIndex(It.Key()) && Entries[It.Key()].bStopWhenAbilityEnds)
		{
			Ids.Add(It.Value());
			It.RemoveCurrent();
		}
	}
	InternalStopByIds(Ids);
}

void UPWAbilityModule_VFX::DebugPlay(const int32 Index, const FPWNiagaraEntry& Played, const TCHAR* From, const UNiagaraComponent* LocalComp, const bool bOwnerPlays) const
{
	const AActor* Avatar = OwnerAbility->GetAvatarActorFromActorInfo();
	const FString System = GetNameSafe(Played.NiagaraSystem.Get());

	FString Placement = From;
	FTransform Placed = Played.WorldTransform;
	if (Played.bAttachToActor && Avatar)
	{
		// Where the VFX lifecycle attaches it: the mesh with the socket, else the root.
		const UMeshComponent* Mesh = UPWGasFunctions::FindSocketMesh(Avatar, Played.AttachSocketName);
		if (const USceneComponent* Parent = Mesh ? Mesh : Avatar->GetRootComponent())
			Placed = FTransform(Played.RelativeRotation, Played.RelativeOffset) * Parent->GetSocketTransform(Mesh ? Played.AttachSocketName : NAME_None);
		if (!Played.AttachSocketName.IsNone())
			Placement += Mesh ? FString::Printf(TEXT(", socket %s on %s"), *Played.AttachSocketName.ToString(), *GetNameSafe(Mesh))
				: FString::Printf(TEXT(", socket %s not found, so on the root"), *Played.AttachSocketName.ToString());
	}
	if (LocalComp)
		Placed = LocalComp->GetComponentTransform();

	FString Size = FString::Printf(TEXT("scale %s"), *Played.Scale.ToCompactString());
	for (const FPWVFXParameter& Parameter : Entries[Index].Parameters)
	{
		if (Parameter.Name.IsNone())
			continue;
		// Names the system doesn't expose are set anyway and do nothing, which is easy to miss.
		const bool bExposed = Played.NiagaraSystem && Played.NiagaraSystem->GetExposedParameters().FindParameterOffset(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), Parameter.Name)) != nullptr;
		const float Value = Parameter.Value ? Parameter.Value->Evaluate(OwnerAbility) : 0.f;
		Size += FString::Printf(TEXT(", %s = %.2f from %s%s"), *Parameter.Name.ToString(), Value,
			Parameter.Value ? *Parameter.Value->Describe(OwnerAbility) : TEXT("nothing set"), bExposed ? TEXT("") : TEXT(" (not exposed by the system)"));
	}
	if (const UPWAbilityModule_Radius* Radius = OwnerAbility->GetRadiusModule())
		DebugCircle(Placed.GetLocation(), Radius->GetRadius(), PWModuleDebug::Radius);

	const FGameplayAbilityActorInfo* Info = OwnerAbility->GetCurrentActorInfo();
	const bool bLocal = Info && Info->IsLocallyControlled();
	const bool bAuthority = Info && Info->IsNetAuthority();
	TArray<FString> Paths;
	if (bLocal)
		Paths.Add(LocalComp ? TEXT("played here, locally controlled") : TEXT("locally controlled, but nothing spawned"));
	if (bAuthority)
		Paths.Add(bOwnerPlays ? TEXT("multicast from the authority, the owner skips it") : TEXT("multicast from the authority to everyone"));
	if (bAuthority && GetDebugWorld()->GetNetMode() == NM_DedicatedServer)
		Paths.Add(TEXT("the dedicated server plays none itself"));
	if (Paths.IsEmpty())
		Paths.Add(TEXT("not played: neither locally controlled nor the authority"));

	DebugPrint(FString::Printf(TEXT("play [%d] %s %s at %s, %s, %s, id %d: %s"), Index, *System, *Placement, *Placed.GetLocation().ToCompactString(), *Size,
		Played.bLooping ? TEXT("looping") : TEXT("one shot"), Played.UniqueId, *FString::Join(Paths, TEXT("; "))));
	DebugAxes(FTransform(Placed.GetRotation(), Placed.GetLocation()), 60.f);
	DebugText(Placed.GetLocation() + FVector(0.f, 0.f, 30.f), FString::Printf(TEXT("[%d] %s"), Index, *System), PWModuleDebug::Text);
}

void UPWAbilityModule_VFX::InternalStopByIds(const TArray<int32>& Ids) const
{
	if (!OwnerAbility || Ids.Num() == 0)
		return;

	const FGameplayAbilityActorInfo* Info = OwnerAbility->GetCurrentActorInfo();
	UPWAbilitySystemComponent* ASC = Info ? Cast<UPWAbilitySystemComponent>(Info->AbilitySystemComponent.Get()) : nullptr;
	if (!ASC)
		return;

	// Stops the local copy, and from the authority everyone else's.
	ASC->Vfx().StopNiagaraList_Local(Ids);
	if (Info->IsNetAuthority())
		ASC->Multicast_StopNiagara(Ids);

	if (IsDebugging())
	{
		TArray<FString> Stopped;
		for (const int32 Id : Ids)
		{
			const int32 Index = Id & 0xFF;
			Stopped.Add(FString::Printf(TEXT("[%d] %s (id %d)"), Index, Entries.IsValidIndex(Index) ? *GetNameSafe(Entries[Index].NiagaraSystem.Get()) : TEXT("?"), Id));
		}
		DebugPrint(FString::Printf(TEXT("stopped %s here%s"), *FString::Join(Stopped, TEXT(", ")), Info->IsNetAuthority() ? TEXT(", and multicast the stop from the authority") : TEXT("")));
	}
}

void UPWAbilityModule_VFX::Stop(const int32 Index)
{
	int32 Id = INDEX_NONE;
	if (!ActiveIndexToId.RemoveAndCopyValue(Index, Id))
	{
		if (IsDebugging())
			DebugPrint(FString::Printf(TEXT("stop [%d]: not playing, only looping entries are tracked"), Index), FLinearColor::Yellow);
		return;
	}
	InternalStopByIds({Id});
}

void UPWAbilityModule_VFX::StopAll()
{
	if (ActiveIndexToId.IsEmpty())
	{
		if (IsDebugging())
			DebugPrint(TEXT("stop all: nothing looping is playing"));
		return;
	}

	TArray<int32> Ids;
	ActiveIndexToId.GenerateValueArray(Ids);
	ActiveIndexToId.Empty();
	InternalStopByIds(Ids);
}
