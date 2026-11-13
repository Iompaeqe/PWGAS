// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/Abilities/Library/PWModuleFunctions.h"

#include "Ability/PWSpellParams.h"
#include "GAS/Abilities/Modules/ActionModules/PWAbilityModule_Actor.h"

UPWEffectSpellParams* UPWModuleFunctions::GetEffectParams(const UPWSpellParams* Params)
{
	return Params ? Params->FindExtension<UPWEffectSpellParams>() : nullptr;
}

UPWProjectileSpellParams* UPWModuleFunctions::GetProjectileParams(const UPWSpellParams* Params)
{
	return Params ? Params->FindExtension<UPWProjectileSpellParams>() : nullptr;
}

UPWActorSpellParams* UPWModuleFunctions::GetActorParams(const UPWSpellParams* Params)
{
	return Params ? Params->FindExtension<UPWActorSpellParams>() : nullptr;
}


float UPWModuleFunctions::GetRange(const UPWSpellParams* Params)
{
	return Params ? Params->More->GetFloat("Range") : 0.f;
}

float UPWModuleFunctions::GetRadius(const UPWSpellParams* Params)
{
	return Params ? Params->More->GetFloat("Radius") : 0.f;
}
