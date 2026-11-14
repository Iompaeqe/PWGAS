// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "VisualAndAudio/PWTagEffectsComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/MeshComponent.h"
#include "Libraries/PWGasFunctions.h"
#include "Materials/MaterialInterface.h"
#include "VisualAndAudio/PWTagEffectSet.h"

UPWTagEffectsComponent::UPWTagEffectsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPWTagEffectsComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UAbilitySystemComponent* AbilitySystem = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
		SetAbilitySystem(AbilitySystem);
}

void UPWTagEffectsComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ReleaseAbilitySystem();
	Super::EndPlay(EndPlayReason);
}

bool UPWTagEffectsComponent::IsCosmetic() const
{
	return GetNetMode() != NM_DedicatedServer;
}

void UPWTagEffectsComponent::SetAbilitySystem(UAbilitySystemComponent* AbilitySystem)
{
	if (!IsCosmetic() || BoundAbilitySystem.Get() == AbilitySystem)
		return;

	ReleaseAbilitySystem();
	if (!AbilitySystem)
		return;

	BoundAbilitySystem = AbilitySystem;
	TagChangedHandle = AbilitySystem->RegisterGenericGameplayTagEvent().AddUObject(this, &ThisClass::OnTagChanged);
	StartPresentEffects();
}

void UPWTagEffectsComponent::SetEffectSet(UPWTagEffectSet* NewSet)
{
	if (EffectSet == NewSet)
		return;

	StopAllEffects();
	EffectSet = NewSet;
	StartPresentEffects();
}

void UPWTagEffectsComponent::ReleaseAbilitySystem()
{
	StopAllEffects();
	if (UAbilitySystemComponent* AbilitySystem = BoundAbilitySystem.Get())
		AbilitySystem->RegisterGenericGameplayTagEvent().Remove(TagChangedHandle);
	BoundAbilitySystem.Reset();
	TagChangedHandle.Reset();
}

void UPWTagEffectsComponent::StartPresentEffects()
{
	// Tags that were on before this component started listening.
	const UAbilitySystemComponent* AbilitySystem = BoundAbilitySystem.Get();
	if (!AbilitySystem || !EffectSet)
		return;

	for (const TPair<FGameplayTag, FPWTagEffect>& Pair : EffectSet->Effects)
		if (AbilitySystem->HasMatchingGameplayTag(Pair.Key))
			StartEffect(Pair.Key);
}

void UPWTagEffectsComponent::OnTagChanged(const FGameplayTag Tag, const int32 Count)
{
	if (!EffectSet || !EffectSet->Effects.Contains(Tag))
		return;

	if (Count > 0)
		StartEffect(Tag);
	else
		StopEffect(Tag);
}

void UPWTagEffectsComponent::StartEffect(const FGameplayTag& Tag)
{
	const FPWTagEffect* Effect = EffectSet ? EffectSet->Effects.Find(Tag) : nullptr;
	if (!Effect || !GetOwner() || ActiveNiagara.Contains(Tag))
		return;

	TArray<TWeakObjectPtr<UNiagaraComponent>>& Spawned = ActiveNiagara.Add(Tag);
	for (const FPWTagEffectNiagara& Entry : Effect->Niagara)
	{
		if (!Entry.System)
			continue;

		if (Entry.Sockets.IsEmpty())
			SpawnNiagara(Entry, NAME_None, Spawned);
		for (const FName Socket : Entry.Sockets)
			SpawnNiagara(Entry, Socket, Spawned);
	}

	if (Effect->OverlayMaterial)
	{
		OverlayOrder.Remove(Tag);
		OverlayOrder.Add(Tag);
		RefreshOverlay();
	}

	OnTagAdded.Broadcast(Tag);
}

void UPWTagEffectsComponent::SpawnNiagara(const FPWTagEffectNiagara& Entry, const FName Socket, TArray<TWeakObjectPtr<UNiagaraComponent>>& Spawned) const
{
	AActor* Owner = GetOwner();
	USceneComponent* AttachTo = Owner->GetRootComponent();
	FName AttachSocket = NAME_None;
	if (UMeshComponent* Mesh = UPWGasFunctions::FindSocketMesh(Owner, Socket))
	{
		AttachTo = Mesh;
		AttachSocket = Socket;
	}
	if (!AttachTo)
		return;

	UNiagaraComponent* Niagara = UNiagaraFunctionLibrary::SpawnSystemAttached(Entry.System, AttachTo, AttachSocket, Entry.Offset, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, false);
	if (!Niagara)
		return;
	if (Entry.bWorldRotation)
	{
		Niagara->SetUsingAbsoluteRotation(true);
		Niagara->SetWorldRotation(FRotator::ZeroRotator);
	}
	Spawned.Add(Niagara);
}

void UPWTagEffectsComponent::StopEffect(const FGameplayTag& Tag)
{
	TArray<TWeakObjectPtr<UNiagaraComponent>> Spawned;
	if (!ActiveNiagara.RemoveAndCopyValue(Tag, Spawned))
		return;

	// Stops spawning and lets what is already out fade before the component goes.
	for (const TWeakObjectPtr<UNiagaraComponent>& Niagara : Spawned)
	{
		if (!Niagara.IsValid())
			continue;
		Niagara->SetAutoDestroy(true);
		Niagara->Deactivate();
	}

	if (OverlayOrder.Remove(Tag) > 0)
		RefreshOverlay();

	OnTagRemoved.Broadcast(Tag);
}

void UPWTagEffectsComponent::StopAllEffects()
{
	TArray<FGameplayTag> Tags;
	ActiveNiagara.GetKeys(Tags);
	for (const FGameplayTag& Tag : Tags)
		StopEffect(Tag);
}

void UPWTagEffectsComponent::RefreshOverlay()
{
	UMaterialInterface* Overlay = nullptr;
	for (int32 Index = OverlayOrder.Num() - 1; Index >= 0 && !Overlay; --Index)
		if (const FPWTagEffect* Effect = EffectSet ? EffectSet->Effects.Find(OverlayOrder[Index]) : nullptr)
			Overlay = Effect->OverlayMaterial;

	const AActor* Owner = GetOwner();
	if (!Owner)
		return;

	// The overlay a mesh had of its own comes back once no tag covers it.
	TInlineComponentArray<UMeshComponent*> Meshes(Owner);
	for (UMeshComponent* Mesh : Meshes)
	{
		if (!OriginalOverlays.Contains(Mesh))
			OriginalOverlays.Add(Mesh, Mesh->GetOverlayMaterial());
		Mesh->SetOverlayMaterial(Overlay ? Overlay : OriginalOverlays[Mesh].Get());
	}
}
