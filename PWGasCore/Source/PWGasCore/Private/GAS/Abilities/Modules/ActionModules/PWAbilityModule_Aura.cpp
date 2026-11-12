// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "PWGasCore/Public/GAS/Abilities/Modules/ActionModules/PWAbilityModule_Aura.h"

#include "AbilitySystemGlobals.h"
#include "Components/SphereComponent.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Effect.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Range.h"
#include "PWGASCoreLogChannels.h"


void UPWAbilityModule_Aura::OnAbilityActivated_Implementation()
{
	APawn* OwnerPawn = GetPawn();
	if (!OwnerPawn || !OwnerAbility)
		return;

	const FGameplayAbilityActorInfo* Info = OwnerAbility->GetCurrentActorInfo();
	if (!Info || !Info->IsNetAuthority()) return;
	
	UWorld* World = OwnerPawn->GetWorld();
	if (!World)
		return;

	// --- Create sphere component dynamically ---
	AuraSphere = NewObject<USphereComponent>(OwnerPawn);
	if (!AuraSphere)
		return;

	AuraSphere->RegisterComponent();
	AuraSphere->AttachToComponent(OwnerPawn->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	float Radius = 500.f;
	if (const auto* Range = OwnerAbility->GetRangeModule())
		Radius = Range->GetRangeValue();
	
	AuraSphere->SetSphereRadius(Radius);
	AuraSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
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

	if (bDrawDebug)
		DrawDebugSphere(World, OwnerPawn->GetActorLocation(), Radius, 16, FColor::Green, false, 10.f);

	UE_LOG(LogPWGASCoreAbilityModule, Log, TEXT("[AuraModule] Aura activated with radius %.1f"), Radius);
}

void UPWAbilityModule_Aura::OnAbilityEnded_Implementation()
{
	if (AuraSphere)
	{
		AuraSphere->DestroyComponent();
		AuraSphere = nullptr;
		UE_LOG(LogPWGASCoreAbilityModule, Log, TEXT("[AuraModule] Aura ended, component destroyed."));
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

	ApplyAuraEffect(OtherActor, false);
	OverlappingActors.Remove(OtherActor);
}

void UPWAbilityModule_Aura::EnterAura(AActor* Actor)
{
	if (!Actor || Actor == GetPawn() || OverlappingActors.Contains(Actor))
		return;

	OverlappingActors.Add(Actor);
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
