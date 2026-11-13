// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Radius.h"

#include "Ability/SpellParams.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"

float UPWAbilityModule_Radius::GetRadiusValue() const
{
	return OwnerAbility ? Radius.GetValueAtLevel(OwnerAbility->GetAbilityLevel()) : Radius.GetValueAtLevel(1);
}

void UPWAbilityModule_Radius::ExtendParams(USpellParams* Params) const
{
	if (!Params) return;

	Params->More->AddFloat(PWParamKeys::Radius.ToString(), GetRadiusValue());
}

bool UPWAbilityModule_Radius::GetValue(const FName Key, float& Out) const
{
	if (Key != PWParamKeys::Radius)
		return false;

	Out = GetRadiusValue();
	return true;
}

void UPWAbilityModule_Radius::Initialize(UPWModularGameplayAbility* InOwner)
{
	Super::Initialize(InOwner);
	
}
