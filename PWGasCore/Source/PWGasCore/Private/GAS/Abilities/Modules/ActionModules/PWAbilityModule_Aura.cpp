// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "PWGasCore/Public/GAS/Abilities/Modules/ActionModules/PWAbilityModule_Aura.h"

#include "AbilitySystemGlobals.h"
#include "Components/SphereComponent.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Effect.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Range.h"


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

	if (bDrawDebug)
		DrawDebugSphere(World, OwnerPawn->GetActorLocation(), Radius, 16, FColor::Green, false, 10.f);

	UE_LOG(LogTemp, Log, TEXT("[AuraModule] Aura activated with radius %.1f"), Radius);
}

void UPWAbilityModule_Aura::OnAbilityEnded_Implementation()
{
	if (AuraSphere)
	{
		AuraSphere->DestroyComponent();
		AuraSphere = nullptr;
		UE_LOG(LogTemp, Log, TEXT("[AuraModule] Aura ended, component destroyed."));
	}
}

void UPWAbilityModule_Aura::GetRequiredDataModules_Implementation(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules)
{
	OutRequiredModules.Add(UPWAbilityModule_Range::StaticClass());
	OutRequiredModules.Add(UPWAbilityModule_Effect::StaticClass());
}

void UPWAbilityModule_Aura::HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ApplyAuraEffect(OtherActor, true);
	OverlappingActors.AddUnique(OtherActor);
}

void UPWAbilityModule_Aura::HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ApplyAuraEffect(OtherActor, false);
	OverlappingActors.Remove(OtherActor);
}

void UPWAbilityModule_Aura::ApplyAuraEffect(const AActor* Target, const bool bApply) const
{
	if (!Target || !OwnerAbility)
		return;

	auto* EffectModule = OwnerAbility->GetEffectModule();
	if (!EffectModule)
		return;

	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);
	UAbilitySystemComponent* SourceASC = OwnerAbility->GetAbilitySystemComponentFromActorInfo();

	if (!TargetASC || !SourceASC)
		return;

	if (bApply)
	{
		EffectModule->ApplyEffects(SourceASC, TargetASC);
		UE_LOG(LogTemp, Verbose, TEXT("[AuraModule] Applied aura effect to %s"), *Target->GetName());
	}
	else
	{
		EffectModule->RemoveEffects(SourceASC, TargetASC);
		UE_LOG(LogTemp, Verbose, TEXT("[AuraModule] Removed aura effect from %s"), *Target->GetName());
	}
}
