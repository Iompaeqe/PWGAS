// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/Abilities/Modules/ControlModules/PWAbilityModule_Niagara.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "GAS/ASC/PWAbilitySystemComponent.h"
#include "GAS/ASC/PWASC_VfxLifecycle.h"
#include "GAS/Data/PWNiagaraEntry.h"
#include "PWGASCoreLogChannels.h"

int32 UPWAbilityModule_Niagara::ComputeUniqueId(const int32 Index) const
{
	if (!OwnerAbility)
		return INDEX_NONE;

	FGameplayAbilitySpecHandle Handle = OwnerAbility->GetCurrentAbilitySpecHandle();

	// Pack handle + index, 8 bits for index is plenty
	return (FCString::Atoi(*Handle.ToString()) << 8) | (Index & 0xFF);
}

FPWNiagaraEntry* UPWAbilityModule_Niagara::GetEntryMutable(int32 Index)
{
	if (!NiagaraEntries.IsValidIndex(Index))
		return nullptr;

	FPWNiagaraEntry& Entry = NiagaraEntries[Index];

	UPWAbilitySystemComponent* ASC = Cast<UPWAbilitySystemComponent>(OwnerAbility->GetAbilitySystemComponentFromActorInfo());
	if (!ASC) return nullptr;
	
	Entry.UniqueId = ComputeUniqueId(Index);

	return &Entry;
}

UNiagaraComponent* UPWAbilityModule_Niagara::InternalPlay(int32 Index, const FTransform* WorldOverride, AActor* TargetActor)
{
	//const FGameplayAbilityActorInfo* Info = OwnerAbility->GetCurrentActorInfo();
	
	if (!OwnerAbility)
		return nullptr;

	UPWAbilitySystemComponent* ASC = Cast<UPWAbilitySystemComponent>(OwnerAbility->GetAbilitySystemComponentFromActorInfo());
	if (!ASC)
		return nullptr;

	FPWNiagaraEntry* Entry = GetEntryMutable(Index);
	if (!Entry || !Entry->NiagaraSystem)
		return nullptr;
	
	//UE_LOG(LogPWGASCoreAbilityModule, Log, TEXT("UPWAbilityModule_Niagara::InternalPlay, Auth: %hhd, Index: %d, UniqueId: %d"), Info->IsNetAuthority(), Index, Entry->UniqueId);

	if (!Entry->bAttachToActor)
	{
		if (WorldOverride)
			Entry->WorldTransform = *WorldOverride;
		else if (TargetActor)
			Entry->WorldTransform = FTransform(TargetActor->GetActorRotation(), TargetActor->GetActorLocation() + Entry->RelativeOffset);
		else
			Entry->WorldTransform = FTransform::Identity;
	}

	// Track active looping VFX.
	if (Entry->bLooping)
		ActiveIndexToId.FindOrAdd(Index) = Entry->UniqueId;

	const FGameplayAbilityActorInfo* Info = OwnerAbility->GetCurrentActorInfo();
	const bool bIsLocallyControlled = Info->IsLocallyControlled();
	const bool bIsAuthority = Info->IsNetAuthority();

	UNiagaraComponent* LocalComp = nullptr;

	// Client prediction: spawn locally on owning client.
	if (bIsLocallyControlled)
		LocalComp = ASC->Vfx().PlayNiagara_Local(*Entry);

	// Server: multicast to everyone (including server).
	// The owner plays its own copy when it runs the ability, so the multicast skips it.
	const bool bOwnerPlays = bIsLocallyControlled || OwnerAbility->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	if (bIsAuthority)
		ASC->Multicast_PlayNiagara(*Entry, bOwnerPlays);

	return LocalComp;
}

UNiagaraComponent* UPWAbilityModule_Niagara::PlayAtLocation(int32 Index, const FTransform& WorldTransform)
{
	return InternalPlay(Index, &WorldTransform, /*TargetActor=*/nullptr);
}

UNiagaraComponent* UPWAbilityModule_Niagara::PlayOnActor(int32 Index, AActor* TargetActor)
{
	if (!TargetActor)
		return nullptr;

	return InternalPlay(Index, /*WorldOverride=*/nullptr, TargetActor);
}

void UPWAbilityModule_Niagara::InternalStopByIds(const TArray<int32>& Ids) const
{
	//const FGameplayAbilityActorInfo* Info = OwnerAbility->GetCurrentActorInfo();
	//UE_LOG(LogPWGASCoreAbilityModule, Log, TEXT("UPWAbilityModule_Niagara::InternalStopByIds, %hhd"), Info->IsNetAuthority());
	
	if (!OwnerAbility || Ids.Num() == 0)
		return;

	const FGameplayAbilityActorInfo* Info = OwnerAbility->GetCurrentActorInfo();
	if (!Info || !Info->AbilitySystemComponent.IsValid())
		return;

	UPWAbilitySystemComponent* ASC = Cast<UPWAbilitySystemComponent>(Info->AbilitySystemComponent.Get());
	if (!ASC) return;

	// Always stop locally (prediction, host, etc.)
	ASC->Vfx().StopNiagaraList_Local(Ids);

	// If we are the server, also broadcast to everyone else
	if (Info->IsNetAuthority())
	{
		ASC->Multicast_StopNiagara(Ids);
	}
}

void UPWAbilityModule_Niagara::Stop(int32 Index)
{
	//const FGameplayAbilityActorInfo* Info = OwnerAbility->GetCurrentActorInfo();
	//UE_LOG(LogPWGASCoreAbilityModule, Log, TEXT("UPWAbilityModule_Niagara::Stop, %hhd"), Info->IsNetAuthority());
	
	int32* IdPtr = ActiveIndexToId.Find(Index);
	if (!IdPtr) return;

	const int32 Id = *IdPtr;
	ActiveIndexToId.Remove(Index);

	TArray<int32> Ids;
	Ids.Add(Id);

	InternalStopByIds(Ids);
}

void UPWAbilityModule_Niagara::StopAll()
{
	//const FGameplayAbilityActorInfo* Info = OwnerAbility->GetCurrentActorInfo();
	//UE_LOG(LogPWGASCoreAbilityModule, Log, TEXT("UPWAbilityModule_Niagara::StopAll, %hhd"), Info->IsNetAuthority());
	
	if (ActiveIndexToId.Num() == 0)
	{
		return;
	}

	TArray<int32> Ids;
	Ids.Reserve(ActiveIndexToId.Num());

	for (const TPair<int32, int32>& Pair : ActiveIndexToId)
		Ids.Add(Pair.Value);

	ActiveIndexToId.Empty();

	InternalStopByIds(Ids);
}
