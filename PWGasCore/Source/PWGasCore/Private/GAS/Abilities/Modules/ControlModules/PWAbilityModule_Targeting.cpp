// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/Abilities/Modules/ControlModules/PWAbilityModule_Targeting.h"

#include "Components/PrimitiveComponent.h"
#include "Debug/Logging.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Range.h"
#include "GAS/Abilities/Modules/ControlModules/PWAbilityModule_Resolver.h"
#include "PWGasCore/Public/Targeting/Types/PWTargetPolicyBase.h"

static bool GetSingleHitFromHandle(const FGameplayAbilityTargetDataHandle& Handle, FHitResult& OutHit)
{
	if (Handle.Num() == 0)
		return false;

	const FGameplayAbilityTargetData* BaseData = Handle.Get(0);
	if (!BaseData)
		return false;

	if (BaseData->HasHitResult())
	{
		if (const FHitResult* HitPtr = BaseData->GetHitResult())
		{
			OutHit = *HitPtr;
			return true;
		}
	}

	return false;
}

static FPWTargetingResult BuildResultFromHandle(const FGameplayAbilityTargetDataHandle& DataHandle, const FGameplayAbilityActorInfo& ActorInfo)
{
	FPWTargetingResult Result;

	FHitResult Hit;
	if (GetSingleHitFromHandle(DataHandle, Hit))
	{
		Result.Hit = Hit;
		Result.bHasHit = Hit.bBlockingHit;

		if (Hit.bBlockingHit)
		{
			if (!Hit.ImpactPoint.IsNearlyZero())
				Result.Location = Hit.ImpactPoint;
			else if (!Hit.Location.IsNearlyZero())
				Result.Location = Hit.Location;
		}
		else
		{
			if (!Hit.Location.IsNearlyZero())
				Result.Location = Hit.Location;
			else if (!Hit.TraceEnd.IsNearlyZero())
				Result.Location = Hit.TraceEnd;
		}

		if (!Hit.TraceEnd.IsNearlyZero() && !Hit.TraceStart.Equals(Hit.TraceEnd))
			Result.Rotation = (Hit.TraceEnd - Hit.TraceStart).Rotation();
		else if (ActorInfo.AvatarActor.IsValid() && !Result.Location.IsNearlyZero())
		{
			const FVector Origin = ActorInfo.AvatarActor->GetActorLocation();
			Result.Rotation = (Result.Location - Origin).Rotation();
		}
	}
	else if (ActorInfo.AvatarActor.IsValid())
	{
		Result.Location = ActorInfo.AvatarActor->GetActorLocation();
		Result.Rotation = ActorInfo.AvatarActor->GetActorRotation();
	}

	return Result;
}

static FString DescribeTargetingHit(const FHitResult& Hit)
{
	if (!Hit.bBlockingHit)
		return TEXT("no blocking hit");
	return FString::Printf(TEXT("blocking hit %s / %s at %s"), *GetNameSafe(Hit.GetActor()), *GetNameSafe(Hit.GetComponent()), *Hit.ImpactPoint.ToCompactString());
}

bool UPWAbilityModule_Targeting::ComputeTarget(FPWTargetingResult& Out)
{
	if (!OwnerAbility)
	{
		pwlogability(Error, "UPWAbilityTargetingModule::ComputeTarget failed: OwnerAbility is null");
		return false;
	}

	InitializePolicies();

	if (TargetingSource)
	{
		// A player aims with its camera or cursor, any other caster through its pawn and AI controller.
		APlayerController* PC = OwnerAbility->GetPC();
		const bool bSampled = PC ? TargetingSource->Sample(PC, Out) : TargetingSource->SampleForPawn(OwnerAbility->GetPawn(), Out);
		if (IsDebugging())
		{
			const FString Through = PC
				? FString::Printf(TEXT("the player controller %s"), *GetNameSafe(PC))
				: FString::Printf(TEXT("the pawn %s (no player controller, AI path)"), *GetNameSafe(OwnerAbility->GetPawn()));
			if (bSampled)
				DebugPrint(FString::Printf(TEXT("%s aimed through %s at %s"), *GetNameSafe(TargetingSource->GetClass()), *Through, *Out.Location.ToCompactString()));
			else
				DebugPrint(FString::Printf(TEXT("%s found no target through %s"), *GetNameSafe(TargetingSource->GetClass()), *Through), FLinearColor::Yellow);
		}
		if (!bSampled)
			return false;
	}
	else
	{
		// No source: the caster is the target point, so a resolver alone gathers targets around it.
		const AActor* Avatar = OwnerAbility->GetAvatarActorFromActorInfo();
		if (!Avatar)
			return false;
		Out.Location = Avatar->GetActorLocation();
		Out.Rotation = Avatar->GetActorRotation();
		if (IsDebugging())
			DebugPrint(FString::Printf(TEXT("no targeting source, aimed at the caster %s"), *GetNameSafe(Avatar)));
	}

	FinalizeTargetingResult(Out);

	pwlogability(Verbose, "ComputeTarget succeeded: %d targets found for %s", Out.HitActors.Num(), *OwnerAbility->GetName());
	return true;
}

void UPWAbilityModule_Targeting::InitializePolicies() const
{
	if (TargetingSource)
		TargetingSource->Initialize(OwnerAbility);

	const UPWAbilityModule_Range* RangeModule = OwnerAbility->GetRangeModule();
	if (RangeModule && RangeModule->RangePolicy)
		RangeModule->RangePolicy->Initialize(OwnerAbility);
}

void UPWAbilityModule_Targeting::FinalizeTargetingResult(FPWTargetingResult& InOut) const
{
	const AActor* AvatarActor = OwnerAbility ? OwnerAbility->GetAvatarActorFromActorInfo() : nullptr;

	// --- Range clamp ---
	if (const UPWAbilityModule_Range* RangeModule = OwnerAbility->GetRangeModule())
	{
		if (AvatarActor)
		{
			RangeModule->ApplyRangePolicy(AvatarActor, InOut);
		}
	}

	// --- Resolve targets ---
	InOut.HitActors.Reset();
	if (const UPWAbilityModule_Resolver* ResolverModule = OwnerAbility->GetResolverModule())
		ResolverModule->ResolveTargets(InOut);

	// --- Final rotation & flags ---
	const FVector Origin = AvatarActor ? AvatarActor->GetActorLocation() : FVector::ZeroVector;
	if (!InOut.Location.IsNearlyZero())
	{
		InOut.Rotation = (InOut.Location - Origin).Rotation();
	}

	if (!IsDebugging())
		return;
	const FHitResult& Hit = InOut.Hit;
	if (!Hit.TraceStart.IsNearlyZero() && !Hit.TraceStart.Equals(Hit.TraceEnd))
		DebugLine(Hit.TraceStart, Hit.bBlockingHit ? Hit.ImpactPoint : Hit.TraceEnd, PWModuleDebug::Direction);
	if (Hit.bBlockingHit)
	{
		DebugPoint(Hit.ImpactPoint, PWModuleDebug::Target);
		DebugText(Hit.ImpactPoint + FVector(0.f, 0.f, 40.f), FString::Printf(TEXT("hit %s"), *GetNameSafe(Hit.GetActor())), PWModuleDebug::Target);
	}
	DebugPoint(InOut.Location, PWModuleDebug::Position);
	DebugText(InOut.Location + FVector(0.f, 0.f, 70.f), TEXT("target"), PWModuleDebug::Position);
	if (AvatarActor)
		DebugArrow(Origin, InOut.Location, PWModuleDebug::Direction);
	DebugPrint(FString::Printf(TEXT("target %s, %.0f from the caster, rotation %s, %d hit actors, %s"),
		*InOut.Location.ToCompactString(), FVector::Dist(Origin, InOut.Location), *InOut.Rotation.ToCompactString(), InOut.HitActors.Num(), *DescribeTargetingHit(Hit)));
}

void UPWAbilityModule_Targeting::MakeTargetingResultFromTargetData(const FGameplayAbilityTargetDataHandle DataHandle, FPWTargetingResult& Out)
{
	Out = FPWTargetingResult{};

	if (!OwnerAbility)
		return;

	InitializePolicies();

	const FGameplayAbilityActorInfo& ActorInfo = OwnerAbility->GetActorInfo();

	Out = BuildResultFromHandle(DataHandle, ActorInfo);

	if (IsDebugging())
	{
		FHitResult Hit;
		if (GetSingleHitFromHandle(DataHandle, Hit))
			DebugPrint(FString::Printf(TEXT("from target data (%d entries): trace %s -> %s, %s"), DataHandle.Num(), *Hit.TraceStart.ToCompactString(), *Hit.TraceEnd.ToCompactString(), *DescribeTargetingHit(Hit)));
		else
			DebugPrint(FString::Printf(TEXT("from target data (%d entries): no hit result, aimed at the caster"), DataHandle.Num()), FLinearColor::Yellow);
	}

	// Now run the **same** post-processing as the local path
	FinalizeTargetingResult(Out);
}

void UPWAbilityModule_Targeting::GetRequiredDataModules_Implementation(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules)
{
}
