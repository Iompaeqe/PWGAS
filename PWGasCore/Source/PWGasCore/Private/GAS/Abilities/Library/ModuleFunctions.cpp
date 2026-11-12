// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/Abilities/Library/ModuleFunctions.h"

#include "Ability/SpellParams.h"
#include "GAS/Abilities/Modules/ActionModules/PWAbilityModule_Actor.h"

UEffectSpellParamsExtension* UModuleFunctions::GetEffectParams(const USpellParams* Params)
{
	return Params ? Params->FindExtension<UEffectSpellParamsExtension>() : nullptr;
}

UProjectileSpellParamsExtension* UModuleFunctions::GetProjectileParams(const USpellParams* Params)
{
	return Params ? Params->FindExtension<UProjectileSpellParamsExtension>() : nullptr;
}

UActorSpellParamsExtension* UModuleFunctions::GetActorParams(const USpellParams* Params)
{
	return Params ? Params->FindExtension<UActorSpellParamsExtension>() : nullptr;
}


float UModuleFunctions::GetRange(const USpellParams* Params)
{
	return Params ? Params->More->GetFloat("Range") : 0.f;
}

float UModuleFunctions::GetRadius(const USpellParams* Params)
{
	return Params ? Params->More->GetFloat("Radius") : 0.f;
}
