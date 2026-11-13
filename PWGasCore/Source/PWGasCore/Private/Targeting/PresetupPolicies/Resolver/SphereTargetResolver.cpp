// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "Targeting/PresetupPolicies/Resolver/SphereTargetResolver.h"

#include "Engine/OverlapResult.h"
#include "Ability/SpellParams.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Radius.h"

void USphereTargetResolver::Resolve_Implementation(const AActor* Actor, const FPWTargetingResult& Target, TArray<AActor*>& OutActors) const
{
	const float Radius = GetFloat(PWParamKeys::Radius);
	if (!Actor || !Actor->GetWorld() || Radius <= 0.f) return;

	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);

	FCollisionObjectQueryParams ObjectParams;
	for (const TEnumAsByte<ECollisionChannel> Channel : ObjectChannels)
		ObjectParams.AddObjectTypesToQuery(Channel);

	if (Actor->GetWorld()->OverlapMultiByObjectType(Overlaps, Target.Location, FQuat::Identity, ObjectParams, Sphere))
	{
		for (auto& R : Overlaps)
			if (AActor* A = R.GetActor())
				OutActors.AddUnique(A);
	}
}

void USphereTargetResolver::GetRequiredDataModules_Implementation(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules)
{
	Super::GetRequiredDataModules_Implementation(OutRequiredModules);

	OutRequiredModules.Add(UPWAbilityModule_Radius::StaticClass());
}
