// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "Targeting/Types/PWTargetPolicyBase.h"
#include "AIController.h"
#include "AITypes.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Actor.h"
#include "Targeting/Data/PWTargetingData.h"
#include "Ability/PWSpellParams.h"
#include "PWMultiDataArray.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "GAS/Abilities/Modules/DataModules/PWDataModule.h"

void UPWTargetPolicyBase::Initialize_Implementation(UPWModularGameplayAbility* InOwner)
{
	OwnerAbility = InOwner;
}

void UPWTargetPolicyBase::GetRequiredDataModules_Implementation(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules)
{
}

bool UPWTargetingSource::Sample_Implementation(APlayerController* PC, FPWTargetingResult& Out) const
{
	return false;
}

bool UPWTargetingSource::SampleForPawn_Implementation(const APawn* Pawn, FPWTargetingResult& Out) const
{
	if (!Pawn)
		return false;

	const FVector Origin = Pawn->GetActorLocation();
	const AAIController* AI = Cast<AAIController>(Pawn->GetController());

	if (AActor* Focus = AI ? AI->GetFocusActor() : nullptr)
	{
		Out.Location = Focus->GetActorLocation();
		Out.Hit = FHitResult(Focus, Cast<UPrimitiveComponent>(Focus->GetRootComponent()), Out.Location, (Origin - Out.Location).GetSafeNormal());
		Out.Hit.bBlockingHit = true;
		Out.bHasHit = true;
	}
	else
	{
		const FVector FocalPoint = AI ? AI->GetFocalPoint() : FAISystem::InvalidLocation;
		Out.Location = FAISystem::IsValidLocation(FocalPoint) ? FocalPoint : Origin + Pawn->GetActorForwardVector() * 1000.f;
		Out.Hit = FHitResult();
		Out.Hit.Location = Out.Location;
		Out.Hit.ImpactPoint = Out.Location;
		Out.bHasHit = false;
	}

	Out.Hit.TraceStart = Origin;
	Out.Hit.TraceEnd = Out.Location;
	Out.Rotation = (Out.Location - Origin).Rotation();
	return true;
}

bool UPWTargetingSource::SampleFrom_Implementation(const APawn* Pawn, const FVector& Origin, FPWTargetingResult& Out) const
{
	Out.Location = Origin;
	return true;
}

void UPWRangePolicy::Apply_Implementation(const AActor* Actor, float MaxRange, FPWTargetingResult& InOut) const
{
}

void UPWTargetResolver::Resolve_Implementation(const AActor* Actor, const FPWTargetingResult& Target, TArray<AActor*>& OutActors) const
{
}

float UPWTargetPolicyBase::GetFloat(const FName Key) const
{
	if (ActiveParams)
		return ActiveParams->More ? ActiveParams->More->GetFloat(Key.ToString()) : 0.f;

	float Value = 0.f;
	if (OwnerAbility)
		for (const UPWDataModule* Module : OwnerAbility->DataModules)
			if (Module && Module->GetValue(Key, Value))
				break;
	return Value;
}

void UPWTargetResolver::GetImpactTargets(const AActor* SpellActor, const UPWSpellParams* Params, const FHitResult& Hit, TArray<AActor*>& OutTargets) const
{
	if (!SpellActor)
		return;

	TGuardValue<const UPWSpellParams*> Guard(ActiveParams, Params);

	FPWTargetingResult Target;
	Target.Location = SpellActor->GetActorLocation();
	Target.Rotation = SpellActor->GetActorRotation();
	Target.Hit = Hit;
	Target.bHasHit = Hit.GetActor() != nullptr;

	// The caster may be gone by the time the spell lands.
	const AActor* Caster = Params && Params->SourceAvatar ? Params->SourceAvatar.Get() : SpellActor;

	TArray<AActor*> Found;
	Resolve(Caster, Target, Found);
	for (AActor* Actor : Found)
		if (IsValid(Actor))
			OutTargets.AddUnique(Actor);
}
