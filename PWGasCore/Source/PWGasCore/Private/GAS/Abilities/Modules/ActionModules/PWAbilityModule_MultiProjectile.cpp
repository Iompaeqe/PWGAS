// copyright mkurtt96

#include "GAS/Abilities/Modules/ActionModules/PWAbilityModule_MultiProjectile.h"

#include "Algo/RandomShuffle.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"


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

	BuildSpreadOrder();
	CurrentSpreadIndex = 0;
	CastId = FGuid::NewGuid();

	SpreadSpawnOneProjectile(Target);

	// Schedule remaining projectiles if there's a delay
	if (SpreadDelay > 0.f && ProjectileCount > 1)
	{
		OwnerAbility->GetWorld()->GetTimerManager().SetTimer(
			SpreadTimerHandle,
			FTimerDelegate::CreateWeakLambda(this, [this, Target]() { SpreadSpawnOneProjectile(Target); }),
			SpreadDelay,
			true
		);
	}
	else
	{
		for (int32 i = 1; i < ProjectileCount; ++i)
		{
			SpreadSpawnOneProjectile(Target);
		}
	}
}

void UPWAbilityModule_MultiProjectile::SpreadSpawnOneProjectile(const FPWTargetingResult& Target)
{
	if (CurrentSpreadIndex >= CachedSpreadOrder.Num())
		return;

	FPWTargetingResult SpreadTarget = Target;
	
	if (ProjectileCount > 1)
	{
		int32 SpreadIndex = CachedSpreadOrder[CurrentSpreadIndex];
		FRotator SpreadRotation = ComputeSpreadRotation(SpreadIndex);
		const FVector Origin = OwnerAbility->GetAvatarActorFromActorInfo()->GetActorLocation();
		const FVector ToTarget = Target.Location - Origin;
		SpreadTarget.Rotation = (ToTarget.Rotation().Quaternion() * SpreadRotation.Quaternion()).Rotator();
		SpreadTarget.Location = Origin + SpreadTarget.Rotation.Vector() * ToTarget.Size();
	}

	Super::SpawnActorWithTargetingResult(SpreadTarget);
	CurrentSpreadIndex++;

	// Clear timer if we've spawned all projectiles
	if (CurrentSpreadIndex >= ProjectileCount)
		OwnerAbility->GetWorld()->GetTimerManager().ClearTimer(SpreadTimerHandle);
}

USpellParams* UPWAbilityModule_MultiProjectile::MakeSpellParams_Implementation(const FPWTargetingResult& Target)
{
	USpellParams* Params = Super::MakeSpellParams_Implementation(Target);
	if (Params && ProjectileCount > 1)
		Params->CastId = CastId;
	return Params;
}

void UPWAbilityModule_MultiProjectile::BuildSpreadOrder()
{
	CachedSpreadOrder.Empty(ProjectileCount);
	
	switch (SpreadPattern)
	{
	case EProjectileSpreadPattern::LeftToRight: Build_LeftToRight(); break;
	case EProjectileSpreadPattern::RightToLeft: Build_RightToLeft(); break;
	case EProjectileSpreadPattern::CenterOut: Build_CenterOut(); break;
	case EProjectileSpreadPattern::OutIn: Build_OutIn(); break;
	case EProjectileSpreadPattern::CenterFanOutRight: Build_CenterFanOutRight(); break;
	case EProjectileSpreadPattern::CenterFanOutLeft: Build_CenterFanOutLeft(); break;
	case EProjectileSpreadPattern::OutFanInRight: Build_OutFanInRight(); break;
	case EProjectileSpreadPattern::OutFanInLeft: Build_OutFanInLeft(); break;
	case EProjectileSpreadPattern::Random2D: Build_Random(false); break;
	case EProjectileSpreadPattern::Random3D: Build_Random(true); break;
	default: Build_LeftToRight(); break;
	}
}

FRotator UPWAbilityModule_MultiProjectile::ComputeSpreadRotation(int32 SpreadIndex) const
{
	const float HalfSpread = SpreadAngle * 0.5f;
	float OffsetYaw = 0.f;

	if (ProjectileCount > 1)
		OffsetYaw = FMath::Lerp(-HalfSpread, +HalfSpread, SpreadIndex / float(ProjectileCount - 1));

	FRotator Rot(0, OffsetYaw, 0);
	if (SpreadPattern == EProjectileSpreadPattern::Random3D)
		Rot.Pitch = FMath::FRandRange(-HalfSpread, +HalfSpread);

	return Rot;
}

void UPWAbilityModule_MultiProjectile::Build_LeftToRight()
{
	for (int32 i = 0; i < ProjectileCount; ++i)
		CachedSpreadOrder.Add(i);
}

void UPWAbilityModule_MultiProjectile::Build_RightToLeft()
{
	for (int32 i = ProjectileCount - 1; i >= 0; --i)
		CachedSpreadOrder.Add(i);
}

void UPWAbilityModule_MultiProjectile::Build_CenterOut()
{
	const int32 Mid = ProjectileCount / 2;
	if (ProjectileCount % 2 == 1)
		CachedSpreadOrder.Add(Mid);
	for (int32 i = 1; i <= Mid; ++i)
	{
		int32 L = Mid - i;
		int32 R = Mid + i;
		if (R < ProjectileCount) CachedSpreadOrder.Add(R);
		if (L >= 0) CachedSpreadOrder.Add(L);
	}
}

void UPWAbilityModule_MultiProjectile::Build_OutIn()
{
	const int32 Mid = ProjectileCount / 2;
	for (int32 i = Mid; i > 0; --i)
	{
		int32 L = Mid - i;
		int32 R = ProjectileCount - L - 1;
		if (L >= 0) CachedSpreadOrder.Add(L);
		if (R < ProjectileCount) CachedSpreadOrder.Add(R);
	}
	if (ProjectileCount % 2 == 1)
		CachedSpreadOrder.Add(Mid);
}

void UPWAbilityModule_MultiProjectile::Build_CenterFanOutRight()
{
	const int32 Mid = ProjectileCount / 2;
	if (ProjectileCount % 2 == 1)
		CachedSpreadOrder.Add(Mid);
	for (int32 i = 1; i <= Mid; ++i)
	{
		int32 R = Mid + i;
		int32 L = Mid - i;
		if (R < ProjectileCount) CachedSpreadOrder.Add(R);
		if (L >= 0) CachedSpreadOrder.Add(L);
	}
}

void UPWAbilityModule_MultiProjectile::Build_CenterFanOutLeft()
{
	const int32 Mid = ProjectileCount / 2;
	if (ProjectileCount % 2 == 1)
		CachedSpreadOrder.Add(Mid);
	for (int32 i = 1; i <= Mid; ++i)
	{
		int32 L = Mid - i;
		int32 R = Mid + i;
		if (L >= 0) CachedSpreadOrder.Add(L);
		if (R < ProjectileCount) CachedSpreadOrder.Add(R);
	}
}

void UPWAbilityModule_MultiProjectile::Build_OutFanInRight()
{
	for (int32 i = 0; i < ProjectileCount / 2; ++i)
	{
		int32 R = ProjectileCount - 1 - i;
		int32 L = i;
		if (R < ProjectileCount) CachedSpreadOrder.Add(R);
		if (L >= 0) CachedSpreadOrder.Add(L);
	}
	if (ProjectileCount % 2 == 1)
		CachedSpreadOrder.Add(ProjectileCount / 2);
}

void UPWAbilityModule_MultiProjectile::Build_OutFanInLeft()
{
	for (int32 i = 0; i < ProjectileCount / 2; ++i)
	{
		int32 L = i;
		int32 R = ProjectileCount - 1 - i;
		if (L >= 0) CachedSpreadOrder.Add(L);
		if (R < ProjectileCount) CachedSpreadOrder.Add(R);
	}
	if (ProjectileCount % 2 == 1)
		CachedSpreadOrder.Add(ProjectileCount / 2);
}

void UPWAbilityModule_MultiProjectile::Build_Random(bool bThreeDimensional)
{
	for (int32 i = 0; i < ProjectileCount; ++i)
		CachedSpreadOrder.Add(i);
	Algo::RandomShuffle(CachedSpreadOrder);

	if (bThreeDimensional)
	{
		for (int32& Idx : CachedSpreadOrder)
			Idx += FMath::RandRange(-2, 2); // variation on pitch later
	}
}
