// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Range.h"

#include "Debug/Logging.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "PWGasCore/Public/Targeting/Types/PWTargetPolicyBase.h"

float UPWAbilityModule_Range::GetRangeValue() const
{
	return OwnerAbility ? Range.GetValueAtLevel(OwnerAbility->GetAbilityLevel()) : Range.GetValueAtLevel(1);
}

void UPWAbilityModule_Range::ApplyRangePolicy(const AActor* Origin, FPWTargetingResult& InOut) const
{
	if (!RangePolicy)
	{
		pwlogability(Error, "ApplyRangePolicy failed: RangePolicy is null in %s", *GetNameSafe(this));
		return;
	}

	if (!Origin)
	{
		pwlogability(Warning, "ApplyRangePolicy: Origin actor is null for %s", *GetNameSafe(this));
		return;
	}

	if (!OwnerAbility)
	{
		pwlogability(Error, "ApplyRangePolicy failed: OwnerAbility is null for %s", *GetNameSafe(this));
		return;
	}

	RangePolicy->Initialize(OwnerAbility);

	const float RangeValue = GetRangeValue();
	if (RangeValue <= 0.f)
	{
		pwlogability(Warning, "ApplyRangePolicy: Range value <= 0 in %s", *GetNameSafe(this));
		return;
	}

	RangePolicy->Apply(Origin, RangeValue, InOut);
}

void UPWAbilityModule_Range::Initialize(UPWModularGameplayAbility* InOwner)
{
	Super::Initialize(InOwner);
	if (RangePolicy) 
		RangePolicy->Initialize(InOwner);
}

void UPWAbilityModule_Range::ExtendParams(UPWSpellParams* Params) const
{
	if (!Params)
		return;

	Params->More->AddFloat(PWParamKeys::Range.ToString(), GetRangeValue());
}

bool UPWAbilityModule_Range::GetValue(const FName Key, float& Out) const
{
	if (Key != PWParamKeys::Range)
		return false;

	Out = GetRangeValue();
	return true;
}
