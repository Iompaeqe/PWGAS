// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/ASC/PWASC_VfxLifecycle.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GAS/ASC/PWAbilitySystemComponent.h"
#include "GAS/Data/PWNiagaraEntry.h"
#include "PWGASCoreLogChannels.h"
#include "Engine/Engine.h"

static TAutoConsoleVariable<bool> CVarDebugNiagaraVfx(TEXT("PW.Debug.NiagaraVfx"), false, TEXT("Prints every Niagara effect an ability system component plays or stops, on screen and in the log."));

static void DebugVfx(const UPWAbilitySystemComponent& ASC, const TCHAR* Action, const FString& What)
{
	if (!CVarDebugNiagaraVfx.GetValueOnGameThread())
		return;

	const FString Message = FString::Printf(TEXT("[%s] %s %s for %s"), *GetDebugStringForWorld(ASC.GetWorld()), Action, *What, *GetNameSafe(ASC.GetAvatarActor()));
	UE_LOG(LogPWGASCoreAbilitySystem, Log, TEXT("%s"), *Message);
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.f, FColor::Cyan, Message);
}

UNiagaraComponent* FPWASC_VfxLifecycle::PlayNiagara_Local(const FPWNiagaraEntry& Entry)
{
	//UE_LOG(LogPWGASCoreAbilitySystem, Log, TEXT("UPWAbilitySystemComponent::PlayNiagara_Local, Auth: %hhd, UniqueId: %d"), ASC.GetOwner()->HasAuthority(), Entry.UniqueId);
	if (!Entry.NiagaraSystem)
		return nullptr;

	AActor* Avatar = ASC.GetAvatarActor();
	if (!Avatar)
		return nullptr;

	UWorld* World = Avatar->GetWorld();
	if (!World)
		return nullptr;

	UNiagaraComponent* Comp = nullptr;

	if (Entry.bAttachToActor)
	{
		USceneComponent* AttachComp = Avatar->GetRootComponent();
		if (!AttachComp)
		{
			return nullptr;
		}

		const FVector LocOffset = Entry.RelativeOffset;
		const FRotator Rot = AttachComp->GetComponentRotation();

		Comp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			Entry.NiagaraSystem,
			AttachComp,
			Entry.AttachSocketName,
			LocOffset,
			Rot,
			EAttachLocation::KeepRelativeOffset,
			/*bAutoDestroy=*/!Entry.bLooping);
	}
	else
	{
		const FTransform& T = Entry.WorldTransform;

		Comp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			Entry.NiagaraSystem,
			T.GetLocation(),
			T.Rotator(),
			T.GetScale3D(),
			/*bAutoDestroy=*/!Entry.bLooping);
	}

	if (!Comp)
		return nullptr;

	DebugVfx(ASC, TEXT("Play"), FString::Printf(TEXT("%s (id %d)"), *GetNameSafe(Entry.NiagaraSystem), Entry.UniqueId));
	

	const bool bShouldTrackLoop = Entry.bLooping && Entry.UniqueId != INDEX_NONE;
	if (bShouldTrackLoop)
	{
		Comp->SetAutoDestroy(false);

		if (const TWeakObjectPtr<UNiagaraComponent>* Existing = ActiveNiagaraVfx.Find(Entry.UniqueId))
		{
			if (Existing->IsValid())
				Existing->Get()->DestroyComponent();
		}

		ActiveNiagaraVfx.FindOrAdd(Entry.UniqueId) = Comp;
	}

	return Comp;
}

void FPWASC_VfxLifecycle::StopNiagara_Local(const int32 UniqueId)
{
	//UE_LOG(LogPWGASCoreAbilitySystem, Log, TEXT("UPWAbilitySystemComponent::StopNiagara_Local, Auth: %hhd, UniqueId: %d"), ASC.GetOwner()->HasAuthority(), UniqueId);
	if (UniqueId == INDEX_NONE)
		return;

	if (const TWeakObjectPtr<UNiagaraComponent>* Found = ActiveNiagaraVfx.Find(UniqueId))
	{
		//UE_LOG(LogPWGASCoreAbilitySystem, Log, TEXT("UPWAbilitySystemComponent::StopNiagara_Local, Auth: %hhd, UniqueId: %d, Niagara Found, cancelling"), ASC.GetOwner()->HasAuthority(), UniqueId);
		DebugVfx(ASC, TEXT("Stop"), FString::Printf(TEXT("id %d"), UniqueId));
		if (Found->IsValid())
			Found->Get()->DestroyComponent();
		ActiveNiagaraVfx.Remove(UniqueId);
	}
}

void FPWASC_VfxLifecycle::StopNiagaraList_Local(const TArray<int32>& Ids)
{
	for (int32 Id : Ids)
	{
		//UE_LOG(LogPWGASCoreAbilitySystem, Log, TEXT("UPWAbilitySystemComponent::StopNiagara_Local, Auth: %hhd, UniqueId: %d, Niagara Found, cancelling"), ASC.GetOwner()->HasAuthority(), Id);
		StopNiagara_Local(Id);
	}
}
