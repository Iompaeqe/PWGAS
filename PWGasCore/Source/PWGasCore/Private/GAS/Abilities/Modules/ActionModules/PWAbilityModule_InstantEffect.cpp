// copyright mkurtt96


#include "GAS/Abilities/Modules/ActionModules/PWAbilityModule_InstantEffect.h"

#include "AbilitySystemGlobals.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "GAS/Abilities/Modules/ControlModules/PWAbilityModule_Targeting.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Effect.h"
#include "GAS/AbilityTasks/PWAbilityTask_TargetFromSource.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "PWGasCore/Public/Targeting/Types/PWTargetPolicyBase.h"

class UPWAbilityModule_Targeting;

void UPWAbilityModule_InstantEffect::ApplyEffects()
{
	if (!OwnerAbility) return;

	UWorld* World = OwnerAbility->GetWorld();
	if (!World) return;

	auto* Effect = OwnerAbility->GetEffectModule();
	if (!Effect)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] InstantEffectAction has no EffectModule!"), *GetNameSafe(this));
		return;
	}

	// Use Targeting module
	if (OwnerAbility->GetTargetingModule())
	{
		ActiveTargetTask = UPWAbilityTask_TargetFromSource::TargetFromSource(OwnerAbility);
		if (!ActiveTargetTask)
			return;

		ActiveTargetTask->ValidData.AddDynamic(
			this, &UPWAbilityModule_InstantEffect::OnTargetDataReady);

		StartAction();
		ActiveTargetTask->ReadyForActivation();
		return;
	}

	// No Targeting - Use Self
	UAbilitySystemComponent* SourceASC = OwnerAbility->GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC) return;

	Targets.Reset();
	Targets.Add(GetPawn());

	FScopedPredictionWindow Prediction(SourceASC);
	for (AActor* Target : Targets)
	{
		if (!Target) continue;
		UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);
		if (TargetASC)
			Effect->ApplyEffects(SourceASC, TargetASC);
	}

	UE_LOG(LogTemp, Log, TEXT("[%s] Applied instant effect to %d targets."), *GetNameSafe(this), Targets.Num());
}

void UPWAbilityModule_InstantEffect::RemoveEffects()
{
	RemoveTargetEffects(false);
}

void UPWAbilityModule_InstantEffect::RemoveTargetEffects(const bool bInfiniteOnly)
{
	auto* Effect = OwnerAbility->GetEffectModule();
	if (!Effect) return;

	UAbilitySystemComponent* SourceASC = OwnerAbility->GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC) return;

	for (AActor* Target : Targets)
	{
		if (!Target) continue;
		UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);
		if (TargetASC)
			Effect->RemoveEffects(SourceASC, TargetASC, bInfiniteOnly);
	}
}

void UPWAbilityModule_InstantEffect::OnAbilityActivated_Implementation()
{
	if (bApplyOnActivated)
		ApplyEffects();
}

void UPWAbilityModule_InstantEffect::OnAbilityEnded_Implementation()
{
	if (bApplyOnActivated)
		RemoveTargetEffects(true);
}

void UPWAbilityModule_InstantEffect::GetRequiredDataModules_Implementation(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules)
{
	OutRequiredModules.Add(UPWAbilityModule_Effect::StaticClass());
}

void UPWAbilityModule_InstantEffect::ApplyEffectsAtLocation(const FVector& Location)
{
	if (!OwnerAbility) return;

	const UPWAbilityModule_Targeting* Targeting = OwnerAbility->GetTargetingModule();
	if (!Targeting) return;

	FPWTargetingResult Result;
	Result.Location = Location;
	Targeting->InitializePolicies();
	if (Targeting->TargetingSource)
		Targeting->TargetingSource->SampleFrom(GetPawn(), Location, Result);
	Targeting->FinalizeTargetingResult(Result);
	ApplyEffectsToTargets(Result);
}

void UPWAbilityModule_InstantEffect::ApplyEffectsAtSocket(const FName SocketName)
{
	const APawn* Pawn = GetPawn();
	if (!Pawn) return;

	const USkeletalMeshComponent* Mesh = Pawn->FindComponentByClass<USkeletalMeshComponent>();
	const bool bHasSocket = Mesh && Mesh->DoesSocketExist(SocketName);
	ApplyEffectsAtLocation(bHasSocket ? Mesh->GetSocketLocation(SocketName) : Pawn->GetActorLocation());
}

void UPWAbilityModule_InstantEffect::OnTargetDataReady(const FGameplayAbilityTargetDataHandle& DataHandle, const struct FPWTargetingResult& Result)
{
	ApplyEffectsToTargets(Result);
	FinishAction();
}

void UPWAbilityModule_InstantEffect::ApplyEffectsToTargets(const FPWTargetingResult& Result)
{
	if (!OwnerAbility) return;

	auto* Effect = OwnerAbility->GetEffectModule();
	if (!Effect) return;

	UAbilitySystemComponent* SourceASC = OwnerAbility->GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC) return;

	Targets = Result.HitActors;
	if (Targets.Num() == 0)
		return;

	FScopedPredictionWindow Prediction(SourceASC);

	for (AActor* Target : Targets)
	{
		if (!Target) continue;
		UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);
		if (TargetASC)
			Effect->ApplyEffects(SourceASC, TargetASC);
	}
}
