// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/Abilities/Modules/ControlModules/PWAbilityModule_Targeting.h"

#include "Debug/Logging.h"
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
		APlayerController* PC = OwnerAbility->GetPC();
		if (!PC)
		{
			pwlogability(Error, "ComputeTarget failed: PlayerController is null for %s", *OwnerAbility->GetName());
			return false;
		}
		if (!TargetingSource->Sample(PC, Out))
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
}

void UPWAbilityModule_Targeting::MakeTargetingResultFromTargetData(const FGameplayAbilityTargetDataHandle DataHandle, FPWTargetingResult& Out)
{
	Out = FPWTargetingResult{};

	if (!OwnerAbility)
		return;

	InitializePolicies();

	const FGameplayAbilityActorInfo& ActorInfo = OwnerAbility->GetActorInfo();

	Out = BuildResultFromHandle(DataHandle, ActorInfo);

	// Now run the **same** post-processing as the local path
	FinalizeTargetingResult(Out);
}

void UPWAbilityModule_Targeting::GetRequiredDataModules_Implementation(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules)
{
}
