// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/Abilities/Library/PWModuleFunctions.h"

#include "Ability/SpellParams.h"
#include "GAS/Abilities/Modules/ActionModules/PWAbilityModule_Actor.h"

UEffectSpellParamsExtension* UPWModuleFunctions::GetEffectParams(const USpellParams* Params)
{
	return Params ? Params->FindExtension<UEffectSpellParamsExtension>() : nullptr;
}

UProjectileSpellParamsExtension* UPWModuleFunctions::GetProjectileParams(const USpellParams* Params)
{
	return Params ? Params->FindExtension<UProjectileSpellParamsExtension>() : nullptr;
}

UActorSpellParamsExtension* UPWModuleFunctions::GetActorParams(const USpellParams* Params)
{
	return Params ? Params->FindExtension<UActorSpellParamsExtension>() : nullptr;
}


float UPWModuleFunctions::GetRange(const USpellParams* Params)
{
	return Params ? Params->More->GetFloat("Range") : 0.f;
}

float UPWModuleFunctions::GetRadius(const USpellParams* Params)
{
	return Params ? Params->More->GetFloat("Radius") : 0.f;
}
