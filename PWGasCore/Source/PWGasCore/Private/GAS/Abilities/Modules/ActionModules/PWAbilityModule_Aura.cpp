// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "PWGasCore/Public/GAS/Abilities/Modules/ActionModules/PWAbilityModule_Aura.h"

#include "AbilitySystemGlobals.h"
#include "Engine/CollisionProfile.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GAS/Abilities/Modules/ActionModules/PWAuraSphereComponent.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Effect.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Range.h"
#include "PWGASCoreLogChannels.h"

namespace PWAuraModuleDebug
{
	// Each drawing outlives the redraw interval a little so the aura doesn't flicker.
	constexpr float RedrawInterval = 0.1f;
	constexpr float RedrawLifetime = RedrawInterval * 1.5f;

	static FString ActorName(const AActor* Actor)
	{
		return Actor ? Actor->GetActorNameOrLabel() : FString(TEXT("none"));
	}

	static FString ActorNames(const TArray<AActor*>& Actors)
	{
		TArray<FString> Names;
		for (const AActor* Actor : Actors)
			Names.Add(ActorName(Actor));
		return FString::Join(Names, TEXT(", "));
	}
}

void UPWAbilityModule_Aura::OnAbilityActivated_Implementation()
{
	APawn* OwnerPawn = GetPawn();
	if (!OwnerPawn || !OwnerAbility)
		return;

	const FGameplayAbilityActorInfo* Info = OwnerAbility->GetCurrentActorInfo();
	if (!Info || !Info->IsNetAuthority())
	{
		if (IsDebugging())
			DebugPrint(TEXT("the aura only runs on the authority, nothing here"), FLinearColor::Gray);
		return;
	}
	
	UWorld* World = OwnerPawn->GetWorld();
	if (!World)
		return;

	// --- Create sphere component dynamically ---
	AuraSphere = NewObject<UPWAuraSphereComponent>(OwnerPawn);
	if (!AuraSphere)
		return;

	AuraSphere->RegisterComponent();
	AuraSphere->AttachToComponent(OwnerPawn->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	float Radius = 500.f;
	if (const auto* Range = OwnerAbility->GetRangeModule())
	{
		Radius = Range->GetRange();
		Range->DebugDrawFrom(OwnerPawn->GetActorLocation(), TEXT("aura"));
	}
	
	AuraSphere->SetSphereRadius(Radius);
	// Only detects the listed object types and ignores everything else, so projectiles and traces pass through it.
	AuraSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AuraSphere->SetCollisionObjectType(ECC_WorldDynamic);
	AuraSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	for (const TEnumAsByte<ECollisionChannel> Channel : ObjectChannels)
		AuraSphere->SetCollisionResponseToChannel(Channel, ECR_Overlap);
	AuraSphere->SetGenerateOverlapEvents(true);

	AuraSphere->OnComponentBeginOverlap.AddDynamic(this, &UPWAbilityModule_Aura::HandleBeginOverlap);
	AuraSphere->OnComponentEndOverlap.AddDynamic(this, &UPWAbilityModule_Aura::HandleEndOverlap);

	// Actors already inside got their overlap before the handlers were bound.
	OverlappingActors.Reset();
	TArray<AActor*> AlreadyInside;
	AuraSphere->UpdateOverlaps();
	AuraSphere->GetOverlappingActors(AlreadyInside);
	for (AActor* Actor : AlreadyInside)
		EnterAura(Actor);

	if (!IsDebugging())
		return;

	TArray<FString> Channels;
	for (const TEnumAsByte<ECollisionChannel> Channel : ObjectChannels)
		Channels.Add(UCollisionProfile::Get()->ReturnChannelNameFromContainerIndex(Channel.GetValue()).ToString());
	DebugPrint(FString::Printf(TEXT("started on the authority: radius %.0f (%s), overlaps %s, %d already inside%s%s"), Radius,
		OwnerAbility->GetRangeModule() ? TEXT("from the Range module") : TEXT("default, no Range module"), *FString::Join(Channels, TEXT(", ")),
		OverlappingActors.Num(), OverlappingActors.IsEmpty() ? TEXT("") : TEXT(": "), *PWAuraModuleDebug::ActorNames(OverlappingActors)));

	DrawDebugAura();
	if (UWorld* DebugWorld = OwnerAbility->GetWorld())
		DebugWorld->GetTimerManager().SetTimer(DebugTimer, this, &UPWAbilityModule_Aura::DrawDebugAura, PWAuraModuleDebug::RedrawInterval, true);
}

void UPWAbilityModule_Aura::OnAbilityEnded_Implementation()
{
	UWorld* World = OwnerAbility ? OwnerAbility->GetWorld() : nullptr;
	if (World && DebugTimer.IsValid())
		World->GetTimerManager().ClearTimer(DebugTimer);

	if (AuraSphere)
	{
		if (IsDebugging())
			DebugPrint(FString::Printf(TEXT("ended with %d inside%s%s"), OverlappingActors.Num(), OverlappingActors.IsEmpty() ? TEXT("") : TEXT(": "), *PWAuraModuleDebug::ActorNames(OverlappingActors)));
		AuraSphere->DestroyComponent();
		AuraSphere = nullptr;
	}
}

void UPWAbilityModule_Aura::GetRequiredDataModules_Implementation(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules)
{
	OutRequiredModules.Add(UPWAbilityModule_Range::StaticClass());
	OutRequiredModules.Add(UPWAbilityModule_Effect::StaticClass());
}

void UPWAbilityModule_Aura::HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	EnterAura(OtherActor);
}

void UPWAbilityModule_Aura::HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OverlappingActors.Contains(OtherActor) || (AuraSphere && AuraSphere->IsOverlappingActor(OtherActor)))
		return;

	if (IsDebugging())
	{
		DebugPrint(FString::Printf(TEXT("%s left, %d inside"), *PWAuraModuleDebug::ActorName(OtherActor), OverlappingActors.Num() - 1));
		DebugPoint(OtherActor->GetActorLocation(), PWModuleDebug::Position);
	}
	ApplyAuraEffect(OtherActor, false);
	OverlappingActors.Remove(OtherActor);
}

void UPWAbilityModule_Aura::EnterAura(AActor* Actor)
{
	if (!Actor || Actor == GetPawn() || OverlappingActors.Contains(Actor))
		return;

	OverlappingActors.Add(Actor);
	// The debug timer starts after the first overlap check; actors found by it are in the start print.
	if (IsDebugging() && DebugTimer.IsValid())
	{
		DebugPrint(FString::Printf(TEXT("%s entered, %d inside"), *PWAuraModuleDebug::ActorName(Actor), OverlappingActors.Num()));
		DebugPoint(Actor->GetActorLocation(), PWModuleDebug::Target);
	}
	ApplyAuraEffect(Actor, true);
}

void UPWAbilityModule_Aura::ApplyAuraEffect(const AActor* Target, const bool bApply) const
{
	if (!Target || !OwnerAbility)
		return;

	auto* EffectModule = OwnerAbility->GetEffectModule();
	UAbilitySystemComponent* SourceASC = OwnerAbility->GetAbilitySystemComponentFromActorInfo();
	if (!EffectModule || !SourceASC)
		return;

	AActor* TargetActor = const_cast<AActor*>(Target);
	if (bApply)
		EffectModule->ApplyEffectsToActor(SourceASC, TargetActor);
	else
		EffectModule->RemoveEffectsFromActor(SourceASC, TargetActor);

	UE_LOG(LogPWGASCoreAbilityModule, Verbose, TEXT("[AuraModule] %s aura effect: %s"), bApply ? TEXT("Applied") : TEXT("Removed"), *Target->GetName());
}

void UPWAbilityModule_Aura::DrawDebugAura()
{
	const APawn* Pawn = GetPawn();
	if (!AuraSphere || !Pawn)
		return;

	const FVector Center = AuraSphere->GetComponentLocation();
	const float Radius = AuraSphere->GetScaledSphereRadius();
	DebugSphere(Center, Radius, PWModuleDebug::Radius, PWAuraModuleDebug::RedrawLifetime);
	DebugCircle(Center - FVector(0.f, 0.f, Pawn->GetSimpleCollisionHalfHeight()), Radius, PWModuleDebug::Radius, PWAuraModuleDebug::RedrawLifetime);
	DebugText(Center + FVector(0.f, 0.f, Radius), FString::Printf(TEXT("Aura %.0f, %d inside"), Radius, OverlappingActors.Num()), PWModuleDebug::Radius, PWAuraModuleDebug::RedrawLifetime);
	for (const AActor* Actor : OverlappingActors)
		if (Actor)
			DebugLine(Center, Actor->GetActorLocation(), PWModuleDebug::Target, PWAuraModuleDebug::RedrawLifetime);
}
