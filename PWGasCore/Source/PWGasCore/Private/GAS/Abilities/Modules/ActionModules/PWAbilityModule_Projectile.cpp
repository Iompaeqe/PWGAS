// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/Abilities/Modules/ActionModules/PWAbilityModule_Projectile.h"

#include "GAS/Abilities/PWModularGameplayAbility.h"

UPWSpellParams* UPWAbilityModule_Projectile::MakeSpellParams_Implementation(const FPWTargetingResult& Target)
{
	if (!OwnerAbility)
		return nullptr;

	UPWSpellParams* Params = OwnerAbility->MakeSpellParams();
	UPWProjectileSpellParams* ProjectileParams = Params->AddExtension<UPWProjectileSpellParams>();
	ProjectileParams->SpawnLocation = Target.Location;
	ProjectileParams->SpawnRotation = BuildSpawnTransform(Target).Rotator();
	ProjectileParams->bAttachToOwner = bAttachToOwner;
	ProjectileParams->AttachSocket = SpawnSocket;
	
	ProjectileParams->InitialSpeed = ProjectileConfig.InitialSpeed;
	ProjectileParams->GravityZOverride = ProjectileConfig.GravityZOverride;

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
