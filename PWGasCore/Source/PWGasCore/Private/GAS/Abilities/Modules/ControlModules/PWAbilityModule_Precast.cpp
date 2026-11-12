// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "PWGasCore/Public/GAS/Abilities/Modules/ControlModules/PWAbilityModule_Precast.h"

#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "GAS/Abilities/Modules/ActionModules/PWAbilityModule_Projectile.h"
#include "GAS/Abilities/Modules/ControlModules/PWAbilityModule_Targeting.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Radius.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Range.h"
#include "Precast/PWPrecastControllerComponent.h"

void UPWAbilityModule_Precast::StartPrecast()
{
	if (!OwnerAbility) return;
	
	const APlayerController* PC = GetPC();
	if (!PC || !PC->IsLocalController()) return;
	
	if (UPWPrecastControllerComponent* C = GetPrecastController())
	{
		UPWTargetingSource* TargetingSource = nullptr;
		UPWRangePolicy* RangePolicy = nullptr;
		UPWTargetResolver* TargetResolver = nullptr;
		FName OriginSocket = NAME_None;
		FPWProjectileSimConfig ProjectileConfig;
		ProjectileConfig.InitialSpeed = 0.f;

		if (const UPWAbilityModule_Targeting* TargetingModule = OwnerAbility->GetTargetingModule())
		{
			TargetingModule->InitializePolicies();
			TargetingSource = TargetingModule->TargetingSource;
		}

		if (const UPWAbilityModule_Resolver* ResolverModule = OwnerAbility->GetResolverModule())
		{
			TargetResolver = ResolverModule->Resolver;
			if (TargetResolver)
				TargetResolver->Initialize(OwnerAbility);
		}
		if (const UPWAbilityModule_Range* RangeModule = OwnerAbility->GetRangeModule())
		{
			RangePolicy = RangeModule->RangePolicy;
			PreviewRange = RangeModule->GetRangeValue();
		}
		
		if (const UPWAbilityModule_Projectile* ProjectileModule = OwnerAbility->GetProjectileModule())
		{
			OriginSocket = ProjectileModule->SpawnSocket;
			ProjectileConfig = ProjectileModule->ProjectileConfig;
		}

		if (const UPWAbilityModule_Radius* RadiusModule = OwnerAbility->GetRadiusModule())
		{
			PreviewRadius = RadiusModule->GetRadiusValue();
		}

		C->StartPrecast(
			TargetingSource,
			RangePolicy,
			TargetResolver,
			Visualizer,
			PreviewRadius,
			PreviewRange,
			OriginSocket,
			ProjectileConfig
		);
		C->SetActionModule(OwnerAbility->GetActionModule<UPWActionModule>());

		if (ReleaseAnimationIndex >= 0)
		{
			UAnimMontage* Montage = nullptr;
			float Rate = 1.f;
			OwnerAbility->GetAnimMontageFromActor(Montage, Rate, ReleaseAnimationIndex);
			C->SetReleasePose(Montage, ReleaseTime);
		}
		bRunning = true;
	}
}

void UPWAbilityModule_Precast::StopPrecast()
{
	const APlayerController* PC = GetPC();
	if (!PC || !PC->IsLocalController()) return;
	
	if (UPWPrecastControllerComponent* C = GetPrecastController())
	{
		CachedResult = C->GetTargetingResult();
		C->StopPrecast();
	}
	bRunning = false;
}

void UPWAbilityModule_Precast::OnAbilityEnded_Implementation()
{
	if (bRunning)
		StopPrecast();
}

void UPWAbilityModule_Precast::UpdateProjectileSimConfig(const FPWProjectileSimConfig& Cfg)
{
	if (UPWPrecastControllerComponent* C = GetPrecastController())
		C->UpdateProjectileSimConfig(Cfg);
}

UPWPrecastControllerComponent* UPWAbilityModule_Precast::GetPrecastController() const
{
	if (const APlayerController* PC = GetPC())
		return PC->FindComponentByClass<UPWPrecastControllerComponent>();
	return nullptr;
}
