// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "GAS/Abilities/Modules/ActionModules/PWAbilityModule_MultiProjectile.h"

#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "GAS/Abilities/Modules/ActionModules/SpawnPatterns/PWSpreadPattern_Fan.h"


AActor* UPWAbilityModule_MultiProjectile::SpawnActorWithTargetingResult(const FPWTargetingResult& Target)
{
	if (ProjectileCount <= 1)
	{
		return Super::SpawnActorWithTargetingResult(Target);
	}
	
	SpreadSpawnProjectiles(Target);
	return nullptr;
}

void UPWAbilityModule_MultiProjectile::SpreadSpawnProjectiles(const FPWTargetingResult& Target)
{
	if (ProjectileCount <= 1)
	{
		Super::SpawnActorWithTargetingResult(Target);
		return;
	}

	BuildSpreadSteps();
	CurrentSpreadStep = 0;
	CastId = FGuid::NewGuid();

	SpreadSpawnNextStep(Target);

	if (SpreadDelay > 0.f && SpreadSteps.Num() > 1)
	{
		OwnerAbility->GetWorld()->GetTimerManager().SetTimer(
			SpreadTimerHandle,
			FTimerDelegate::CreateWeakLambda(this, [this, Target]() { SpreadSpawnNextStep(Target); }),
			SpreadDelay,
			true
		);
	}
	else
	{
		while (CurrentSpreadStep < SpreadSteps.Num())
			SpreadSpawnNextStep(Target);
	}
}

void UPWAbilityModule_MultiProjectile::SpreadSpawnNextStep(const FPWTargetingResult& Target)
{
	if (!SpreadSteps.IsValidIndex(CurrentSpreadStep))
		return;

	for (const int32 SpreadIndex : SpreadSteps[CurrentSpreadStep])
		Super::SpawnActorWithTargetingResult(MakeSpreadTarget(Target, Shots[SpreadIndex].Offset));
	CurrentSpreadStep++;

	if (CurrentSpreadStep >= SpreadSteps.Num())
		OwnerAbility->GetWorld()->GetTimerManager().ClearTimer(SpreadTimerHandle);
}

FPWTargetingResult UPWAbilityModule_MultiProjectile::MakeSpreadTarget(const FPWTargetingResult& Target, const FRotator& Offset) const
{
	const FVector Origin = OwnerAbility->GetAvatarActorFromActorInfo()->GetActorLocation();
	const FVector ToTarget = Target.Location - Origin;

	FPWTargetingResult SpreadTarget = Target;
	SpreadTarget.Rotation = (ToTarget.Rotation().Quaternion() * Offset.Quaternion()).Rotator();
	SpreadTarget.Location = Origin + SpreadTarget.Rotation.Vector() * ToTarget.Size();
	return SpreadTarget;
}

void UPWAbilityModule_MultiProjectile::GetPreviewAimPoints(const FPWTargetingResult& Target, TArray<FVector>& OutAimPoints) const
{
	if (ProjectileCount <= 1 || !OwnerAbility->GetAvatarActorFromActorInfo())
		return;

	for (const FPWSpreadShot& Shot : BuildShots())
		OutAimPoints.Add(MakeSpreadTarget(Target, Shot.Offset).Location);
}

USpellParams* UPWAbilityModule_MultiProjectile::MakeSpellParams_Implementation(const FPWTargetingResult& Target)
{
	USpellParams* Params = Super::MakeSpellParams_Implementation(Target);
	if (Params && ProjectileCount > 1)
		Params->CastId = CastId;
	return Params;
}

TArray<FPWSpreadShot> UPWAbilityModule_MultiProjectile::BuildShots() const
{
	const UPWSpreadPattern* UsedPattern = Pattern ? Pattern.Get() : GetDefault<UPWSpreadPattern_Fan>();
	return UsedPattern->BuildShots(ProjectileCount, SpreadAngle);
}

void UPWAbilityModule_MultiProjectile::BuildSpreadSteps()
{
	Shots = BuildShots();
	SpreadSteps.Reset();

	TArray<int32> Order;
	for (int32 i = 0; i < Shots.Num(); ++i)
		Order.Add(i);
	Order.StableSort([this](const int32 A, const int32 B) { return Shots[A].Step < Shots[B].Step; });

	for (const int32 Index : Order)
	{
		if (SpreadSteps.Num() == 0 || Shots[SpreadSteps.Last().Last()].Step != Shots[Index].Step)
			SpreadSteps.AddDefaulted();
		SpreadSteps.Last().Add(Index);
	}
}
