// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/Abilities/Modules/ActionModules/PWAbilityModule_Projectile.h"

#include "GAS/Abilities/PWModularGameplayAbility.h"

USpellParams* UPWAbilityModule_Projectile::MakeSpellParams_Implementation(const FPWTargetingResult& Target)
{
	if (!OwnerAbility)
		return nullptr;

	USpellParams* Params = OwnerAbility->MakeSpellParams();
	UProjectileSpellParamsExtension* ProjectileParams = Params->AddExtension<UProjectileSpellParamsExtension>();
	ProjectileParams->SpawnLocation = Target.Location;
	ProjectileParams->SpawnRotation = (Target.Location - OwnerAbility->GetPawn()->GetActorLocation()).Rotation();
	ProjectileParams->bAttachToOwner = bAttachToOwner;
	ProjectileParams->AttachSocket = SpawnSocket;
	
	ProjectileParams->InitialSpeed = ProjectileConfig.InitialSpeed;
	ProjectileParams->GravityZOverride = ProjectileConfig.GravityZOverride;
	ProjectileParams->ProjectileRadius = ProjectileConfig.ProjectileRadius;

	ExtendParamsWithData(Params);

	return Params;
}

AActor* UPWAbilityModule_Projectile::SpawnProjectileWithTargetingResult(const FPWTargetingResult& Target)
{
	return SpawnActorWithTargetingResult(Target);
}

void UPWAbilityModule_Projectile::SpawnProjectile()
{
	SpawnActor();
}

const FPWProjectileSimConfig& UPWAbilityModule_Projectile::GetProjectileConfig() const
{ return ProjectileConfig; }
