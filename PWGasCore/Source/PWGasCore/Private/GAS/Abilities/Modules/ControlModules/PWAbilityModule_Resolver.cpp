// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "GAS/Abilities/Modules/ControlModules/PWAbilityModule_Resolver.h"

#include "GameFramework/Actor.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Radius.h"
#include "GAS/Abilities/Modules/DataModules/PWDataModule.h"
#include "PWGasCore/Public/Targeting/Types/PWTargetPolicyBase.h"
#include "Targeting/Data/PWTargetingData.h"
#include "Ability/PWSpellParams.h"

void UPWAbilityModule_Resolver::ResolveTargets(FPWTargetingResult& InOut) const
{
	ResolveAt(InOut, TEXT("at the target point"));
}

FPWTargetingResult UPWAbilityModule_Resolver::ResolveAroundCaster() const
{
	FPWTargetingResult Result;
	if (const AActor* Avatar = OwnerAbility ? OwnerAbility->GetAvatarActorFromActorInfo() : nullptr)
	{
		Result.Location = Avatar->GetActorLocation();
		Result.Rotation = Avatar->GetActorRotation();
	}
	ResolveAt(Result, TEXT("around the caster"));
	return Result;
}

void UPWAbilityModule_Resolver::ResolveAt(FPWTargetingResult& InOut, const TCHAR* Where) const
{
	InOut.HitActors.Reset();

	const AActor* Avatar = OwnerAbility ? OwnerAbility->GetAvatarActorFromActorInfo() : nullptr;
	if (!Resolver || !Avatar)
	{
		if (IsDebugging())
			DebugPrint(FString::Printf(TEXT("nothing resolved %s: %s"), Where, Resolver ? TEXT("no caster") : TEXT("no resolver set")), FLinearColor::Yellow);
		return;
	}

	Resolver->Initialize(OwnerAbility);

	TArray<AActor*> Found;
	Resolver->Resolve(Avatar, InOut, Found);
	for (AActor* Actor : Found)
		if (IsValid(Actor))
			InOut.HitActors.AddUnique(Actor);

	DebugResolve(InOut, Where);
}

void UPWAbilityModule_Resolver::DebugResolve(const FPWTargetingResult& Result, const TCHAR* Where) const
{
	const UPWAbilityModule_Radius* RadiusModule = OwnerAbility->GetRadiusModule();
	if (!IsDebugging() && !(RadiusModule && RadiusModule->IsDebugging()))
		return;

	TArray<TSubclassOf<UPWDataModule>> Required;
	Resolver->GetRequiredDataModules(Required);
	const bool bUsesRadius = Required.ContainsByPredicate([](const TSubclassOf<UPWDataModule>& Class)
	{
		return Class.Get() && Class.Get()->IsChildOf(UPWAbilityModule_Radius::StaticClass());
	});
	const FVector Center = Result.Location;
	if (bUsesRadius && RadiusModule)
		RadiusModule->DebugDrawAt(Center, TEXT("targets"));
	if (!IsDebugging())
		return;

	const float Radius = bUsesRadius ? Resolver->GetFloat(PWParamKeys::Radius) : 0.f;
	if (Radius > 0.f)
	{
		DebugSphere(Center, Radius, PWModuleDebug::Radius);
		DebugCircle(Center, Radius, PWModuleDebug::Radius);
	}
	DebugPoint(Center, PWModuleDebug::Position);

	TArray<FString> Names;
	for (const AActor* Actor : Result.HitActors)
	{
		const FVector Location = Actor->GetActorLocation();
		DebugLine(Center, Location, PWModuleDebug::Target);
		DebugPoint(Location, PWModuleDebug::Target);
		DebugText(Location + FVector(0.f, 0.f, 60.f), GetNameSafe(Actor), PWModuleDebug::Target);
		Names.Add(GetNameSafe(Actor));
	}

	const FString ResolverName = GetNameSafe(Resolver->GetClass());
	const FString RadiusText = Radius > 0.f ? FString::Printf(TEXT(", radius %.0f"), Radius) : FString();
	DebugText(Center + FVector(0.f, 0.f, 40.f), FString::Printf(TEXT("%s: %d found"), *ResolverName, Names.Num()), PWModuleDebug::Text);
	DebugPrint(FString::Printf(TEXT("%s %s %s%s, %d found%s%s"), *ResolverName, Where, *Center.ToCompactString(), *RadiusText, Names.Num(),
		Names.IsEmpty() ? TEXT("") : TEXT(": "), *FString::Join(Names, TEXT(", "))));
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
