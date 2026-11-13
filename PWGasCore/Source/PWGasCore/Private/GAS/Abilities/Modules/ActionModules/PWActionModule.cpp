// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/Abilities/Modules/ActionModules/PWActionModule.h"

#include "GAS/Abilities/PWModularGameplayAbility.h"


USpellParams* UPWActionModule::MakeSpellParams_Implementation(const FPWTargetingResult& Target)
{
	return nullptr;
}

void UPWActionModule::StartAction()
{
	bActionPending = true;
}

void UPWActionModule::FinishAction()
{
	bActionPending = false;
	OnActionFinished.Broadcast();
}

void UPWActionModule::ExtendParamsWithData(USpellParams* Params) const
{
	for (TObjectPtr Data : OwnerAbility->DataModules)
	{
		Data.Get()->ExtendParams(Params);
	}
	for (const UPWControlModule* Control : OwnerAbility->ControlModules)
		if (Control)
			Control->ExtendParams(Params);
}
