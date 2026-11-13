// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "GAS/Abilities/Modules/ControlModules/PWAbilityModule_Resolver.h"

#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "PWGasCore/Public/Targeting/Types/PWTargetPolicyBase.h"
#include "Targeting/Data/PWTargetingData.h"
#include "Ability/PWSpellParams.h"

void UPWAbilityModule_Resolver::ResolveTargets(FPWTargetingResult& InOut) const
{
	InOut.HitActors.Reset();

	const AActor* Avatar = OwnerAbility ? OwnerAbility->GetAvatarActorFromActorInfo() : nullptr;
	if (!Resolver || !Avatar)
		return;

	Resolver->Initialize(OwnerAbility);

	TArray<AActor*> Found;
	Resolver->Resolve(Avatar, InOut, Found);
	for (AActor* Actor : Found)
		if (IsValid(Actor))
			InOut.HitActors.AddUnique(Actor);
}

FPWTargetingResult UPWAbilityModule_Resolver::ResolveAroundCaster() const
{
	FPWTargetingResult Result;
	if (const AActor* Avatar = OwnerAbility ? OwnerAbility->GetAvatarActorFromActorInfo() : nullptr)
	{
		Result.Location = Avatar->GetActorLocation();
		Result.Rotation = Avatar->GetActorRotation();
	}
	ResolveTargets(Result);
	return Result;
}

void UPWAbilityModule_Resolver::GetRequiredDataModules_Implementation(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules)
{
	if (Resolver)
		Resolver->GetRequiredDataModules(OutRequiredModules);
}

void UPWAbilityModule_Resolver::ExtendParams(UPWSpellParams* Params) const
{
	if (Params)
		Params->TargetResolver = Resolver;
}
