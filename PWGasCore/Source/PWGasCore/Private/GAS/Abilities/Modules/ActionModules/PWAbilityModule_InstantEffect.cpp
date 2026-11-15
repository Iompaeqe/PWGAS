// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/Abilities/Modules/ActionModules/PWAbilityModule_InstantEffect.h"

#include "AbilitySystemGlobals.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "GAS/Abilities/Modules/ControlModules/PWAbilityModule_Targeting.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Effect.h"
#include "GAS/AbilityTasks/PWAbilityTask_TargetFromSource.h"
#include "Libraries/PWGasFunctions.h"
#include "GameFramework/Pawn.h"
#include "PWGasCore/Public/Targeting/Types/PWTargetPolicyBase.h"
#include "PWGASCoreLogChannels.h"

class UPWAbilityModule_Targeting;

namespace PWInstantEffectModuleDebug
{
	static FString ActorName(const AActor* Actor)
	{
		return Actor ? Actor->GetActorNameOrLabel() : FString(TEXT("none"));
	}
}

void UPWAbilityModule_InstantEffect::ApplyEffects()
{
	if (!OwnerAbility) return;

	UWorld* World = OwnerAbility->GetWorld();
	if (!World) return;

	auto* Effect = OwnerAbility->GetEffectModule();
	if (!Effect)
	{
		UE_LOG(LogPWGASCoreAbilityModule, Warning, TEXT("[%s] InstantEffectAction has no EffectModule!"), *GetNameSafe(this));
		return;
	}

	// Use Targeting module
	if (OwnerAbility->GetTargetingModule())
	{
		ActiveTargetTask = UPWAbilityTask_TargetFromSource::TargetFromSource(OwnerAbility);
		if (!ActiveTargetTask)
		{
			if (IsDebugging())
				DebugPrint(TEXT("could not start the targeting task, nothing applied"), FLinearColor::Red);
			return;
		}

		ActiveTargetTask->ValidData.AddDynamic(
			this, &UPWAbilityModule_InstantEffect::OnTargetDataReady);

		StartAction();
		if (IsDebugging())
			DebugPrint(TEXT("action started: waiting for the Targeting module's target data"));
		ActiveTargetTask->ReadyForActivation();
		return;
	}

	// No Targeting - Use Self
	// A resolver with no targeting gathers around the caster.
	if (const UPWAbilityModule_Resolver* ResolverModule = OwnerAbility->GetResolverModule())
	{
		if (IsDebugging())
			DebugPrint(FString::Printf(TEXT("no Targeting module: %s gathers targets around the caster"), *GetNameSafe(ResolverModule->Resolver)));
		ApplyEffectsToTargets(ResolverModule->ResolveAroundCaster());
		return;
	}

	UAbilitySystemComponent* SourceASC = OwnerAbility->GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC) return;

	Targets.Reset();
	Targets.Add(GetPawn());

	if (IsDebugging())
	{
		DebugPrint(FString::Printf(TEXT("no Targeting or Resolver module: applying to the caster %s"), *PWInstantEffectModuleDebug::ActorName(GetPawn())));
		if (const APawn* Pawn = GetPawn())
			DebugPoint(Pawn->GetActorLocation(), PWModuleDebug::Target);
	}

	FScopedPredictionWindow Prediction(SourceASC);
	for (const TWeakObjectPtr<AActor>& WeakTarget : Targets)
	{
		AActor* Target = WeakTarget.Get();
		if (!Target) continue;
		Effect->ApplyEffectsToActor(SourceASC, Target);
	}
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

	if (IsDebugging())
		DebugPrint(FString::Printf(TEXT("removing %s effects from %d target(s)"), bInfiniteOnly ? TEXT("the infinite") : TEXT("all"), Targets.Num()));

	for (const TWeakObjectPtr<AActor>& WeakTarget : Targets)
	{
		AActor* Target = WeakTarget.Get();
		if (!Target) continue;
		Effect->RemoveEffectsFromActor(SourceASC, Target, bInfiniteOnly);
	}
}

void UPWAbilityModule_InstantEffect::OnAbilityActivated_Implementation()
{
	if (bApplyOnActivated && IsDebugging())
		DebugPrint(OwnerAbility->DefersAction() ? TEXT("Apply On Activated: waiting for the ability to run its action") : TEXT("Apply On Activated: applying now"));
	if (bApplyOnActivated && !OwnerAbility->DefersAction())
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

	FPWTargetingResult Result;
	Result.Location = Location;

	if (IsDebugging())
	{
		DebugPoint(Location, PWModuleDebug::Position);
		DebugText(Location + FVector(0.f, 0.f, 30.f), TEXT("effect location"), PWModuleDebug::Position);
	}

	const UPWAbilityModule_Targeting* Targeting = OwnerAbility->GetTargetingModule();
	if (!Targeting)
	{
		if (const UPWAbilityModule_Resolver* ResolverModule = OwnerAbility->GetResolverModule())
		{
			if (IsDebugging())
				DebugPrint(FString::Printf(TEXT("applying at %s: %s gathers targets there"), *Location.ToCompactString(), *GetNameSafe(ResolverModule->Resolver)));
			ResolverModule->ResolveTargets(Result);
			ApplyEffectsToTargets(Result);
		}
		else if (IsDebugging())
			DebugPrint(FString::Printf(TEXT("nothing applied at %s: no Targeting or Resolver module to find targets"), *Location.ToCompactString()), FLinearColor::Yellow);
		return;
	}
	if (IsDebugging())
		DebugPrint(FString::Printf(TEXT("applying at %s: sampled by the Targeting source %s"), *Location.ToCompactString(), *GetNameSafe(Targeting->TargetingSource)));
	Targeting->InitializePolicies();
	if (Targeting->TargetingSource)
		Targeting->TargetingSource->SampleFrom(GetPawn(), Location, Result);
	Targeting->FinalizeTargetingResult(Result);
	if (IsDebugging() && !Result.Location.Equals(Location, 1.f))
		DebugLine(Location, Result.Location, PWModuleDebug::Position);
	ApplyEffectsToTargets(Result);
}

void UPWAbilityModule_InstantEffect::ApplyEffectsAtSocket(const FName SocketName)
{
	const APawn* Pawn = GetPawn();
	if (!Pawn) return;

	const UMeshComponent* Mesh = UPWGasFunctions::FindSocketMesh(Pawn, SocketName);
	if (IsDebugging())
	{
		if (Mesh)
		{
			DebugAxes(Mesh->GetSocketTransform(SocketName));
			DebugText(Mesh->GetSocketLocation(SocketName) + FVector(0.f, 0.f, 60.f), SocketName.ToString(), PWModuleDebug::Position);
			DebugPrint(FString::Printf(TEXT("socket %s on %s"), *SocketName.ToString(), *Mesh->GetName()));
		}
		else
			DebugPrint(FString::Printf(TEXT("socket %s not found on %s, using its location"), *SocketName.ToString(), *PWInstantEffectModuleDebug::ActorName(Pawn)), FLinearColor::Yellow);
	}
	ApplyEffectsAtLocation(Mesh ? Mesh->GetSocketLocation(SocketName) : Pawn->GetActorLocation());
}

void UPWAbilityModule_InstantEffect::OnTargetDataReady(const FGameplayAbilityTargetDataHandle& DataHandle, const struct FPWTargetingResult& Result)
{
	ApplyEffectsToTargets(Result);
	FinishAction();
	if (IsDebugging())
		DebugPrint(TEXT("action finished"));
}

void UPWAbilityModule_InstantEffect::ApplyEffectsToTargets(const FPWTargetingResult& Result)
{
	if (!OwnerAbility) return;

	auto* Effect = OwnerAbility->GetEffectModule();
	if (!Effect) return;

	UAbilitySystemComponent* SourceASC = OwnerAbility->GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC) return;

	Targets.Reset();
	for (AActor* Hit : Result.HitActors)
		Targets.Add(Hit);
	if (IsDebugging())
		DebugTargets(Result);
	if (Targets.Num() == 0)
		return;

	FScopedPredictionWindow Prediction(SourceASC);

	for (const TWeakObjectPtr<AActor>& WeakTarget : Targets)
	{
		AActor* Target = WeakTarget.Get();
		if (!Target) continue;
		Effect->ApplyEffectsToActor(SourceASC, Target);
	}
}

void UPWAbilityModule_InstantEffect::DebugTargets(const FPWTargetingResult& Result) const
{
	const APawn* Pawn = GetPawn();
	const FVector From = Pawn ? Pawn->GetActorLocation() : Result.Location;
	DebugPoint(Result.Location, PWModuleDebug::Position);

	TArray<FString> Names;
	for (const TWeakObjectPtr<AActor>& WeakTarget : Targets)
	{
		const AActor* Target = WeakTarget.Get();
		if (!Target)
			continue;
		Names.Add(PWInstantEffectModuleDebug::ActorName(Target));
		DebugArrow(From, Target->GetActorLocation(), PWModuleDebug::Target);
		DebugPoint(Target->GetActorLocation(), PWModuleDebug::Target);
	}

	const TCHAR* FoundBy = OwnerAbility->GetTargetingModule() ? TEXT("the Targeting module") : TEXT("the Resolver");
	if (Names.IsEmpty())
		DebugPrint(FString::Printf(TEXT("no targets from %s at %s, nothing applied"), FoundBy, *Result.Location.ToCompactString()), FLinearColor::Yellow);
	else
		DebugPrint(FString::Printf(TEXT("%d target(s) from %s at %s: %s"), Names.Num(), FoundBy, *Result.Location.ToCompactString(), *FString::Join(Names, TEXT(", "))), FLinearColor::Green);
}
