// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "GAS/Abilities/Modules/ActionModules/PWAbilityModule_MultiProjectile.h"

#include "Algo/RandomShuffle.h"
#include "Algo/Reverse.h"
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
		Super::SpawnActorWithTargetingResult(MakeSpreadTarget(Target, SpreadIndex));
	CurrentSpreadStep++;

	if (CurrentSpreadStep >= SpreadSteps.Num())
		OwnerAbility->GetWorld()->GetTimerManager().ClearTimer(SpreadTimerHandle);
}

FPWTargetingResult UPWAbilityModule_MultiProjectile::MakeSpreadTarget(const FPWTargetingResult& Target, const int32 SpreadIndex) const
{
	const FVector Origin = OwnerAbility->GetAvatarActorFromActorInfo()->GetActorLocation();
	const FVector ToTarget = Target.Location - Origin;

	FPWTargetingResult SpreadTarget = Target;
	SpreadTarget.Rotation = (ToTarget.Rotation().Quaternion() * ComputeSpreadRotation(SpreadIndex).Quaternion()).Rotator();
	SpreadTarget.Location = Origin + SpreadTarget.Rotation.Vector() * ToTarget.Size();
	return SpreadTarget;
}

void UPWAbilityModule_MultiProjectile::GetPreviewAimPoints(const FPWTargetingResult& Target, TArray<FVector>& OutAimPoints) const
{
	if (ProjectileCount <= 1 || !OwnerAbility->GetAvatarActorFromActorInfo())
		return;

	for (int32 SpreadIndex = 0; SpreadIndex < ProjectileCount; ++SpreadIndex)
		OutAimPoints.Add(MakeSpreadTarget(Target, SpreadIndex).Location);
}

USpellParams* UPWAbilityModule_MultiProjectile::MakeSpellParams_Implementation(const FPWTargetingResult& Target)
{
	USpellParams* Params = Super::MakeSpellParams_Implementation(Target);
	if (Params && ProjectileCount > 1)
		Params->CastId = CastId;
	return Params;
}

void UPWAbilityModule_MultiProjectile::BuildSpreadSteps()
{
	SpreadSteps.Reset();

	const auto FromCenter = [this](const int32 Index) { return FMath::Abs(2 * Index - (ProjectileCount - 1)); };
	const auto IsCenter = [&](const int32 Index) { return FromCenter(Index) == (ProjectileCount - 1) % 2; };

	TArray<int32> Order;
	for (int32 i = 0; i < ProjectileCount; ++i)
		Order.Add(i);

	const auto SortFromCenter = [&](const bool bOutward, const bool bRightFirst)
	{
		Order.Sort([&](const int32 A, const int32 B)
		{
			if (FromCenter(A) != FromCenter(B))
				return bOutward ? FromCenter(A) < FromCenter(B) : FromCenter(A) > FromCenter(B);
			return bRightFirst ? A > B : A < B;
		});
	};

	switch (SpreadPattern)
	{
	case EProjectileSpreadPattern::RightToLeft: Algo::Reverse(Order); break;
	case EProjectileSpreadPattern::CenterToSides:
	case EProjectileSpreadPattern::CenterOut:
	case EProjectileSpreadPattern::CenterFanOutRight: SortFromCenter(true, true); break;
	case EProjectileSpreadPattern::CenterFanOutLeft: SortFromCenter(true, false); break;
	case EProjectileSpreadPattern::SidesToCenter:
	case EProjectileSpreadPattern::OutIn:
	case EProjectileSpreadPattern::OutFanInRight: SortFromCenter(false, true); break;
	case EProjectileSpreadPattern::OutFanInLeft: SortFromCenter(false, false); break;
	case EProjectileSpreadPattern::Random2D:
	case EProjectileSpreadPattern::Random3D: Algo::RandomShuffle(Order); break;
	default: break;
	}

	for (const int32 Index : Order)
	{
		bool bSameStep = false;
		if (SpreadSteps.Num() > 0)
		{
			const int32 Previous = SpreadSteps.Last().Last();
			switch (SpreadPattern)
			{
			case EProjectileSpreadPattern::CenterToSides:
			case EProjectileSpreadPattern::SidesToCenter: bSameStep = IsCenter(Index) == IsCenter(Previous); break;
			case EProjectileSpreadPattern::CenterOut:
			case EProjectileSpreadPattern::OutIn: bSameStep = FromCenter(Index) == FromCenter(Previous); break;
			default: break;
			}
		}

		if (!bSameStep)
			SpreadSteps.AddDefaulted();
		SpreadSteps.Last().Add(Index);
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
