// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/Abilities/PWAbilityDataAsset.h"
#include "GAS/Abilities/PWGameplayAbilityBase.h"
#include "Misc/DataValidation.h"

#if WITH_EDITOR
EDataValidationResult UPWAbilityDataAsset::IsDataValid(FDataValidationContext& Context) const
{
	if (!Ability.Tag.IsValid())
		Context.AddError(FText::FromString(TEXT("Ability tag is not set.")));
	if (!Ability.AbilityClass)
		Context.AddError(FText::FromString(TEXT("Ability class is not set.")));

	const UPWGameplayAbilityBase* AbilityDefault = Ability.AbilityClass ? Cast<UPWGameplayAbilityBase>(Ability.AbilityClass.GetDefaultObject()) : nullptr;
	if (AbilityDefault && AbilityDefault->AbilityTag != Ability.Tag)
		Context.AddError(FText::FromString(FString::Printf(TEXT("Tag [%s] does not match the ability tag [%s] of %s."), *Ability.Tag.ToString(), *AbilityDefault->AbilityTag.ToString(), *Ability.AbilityClass->GetName())));

	return Context.GetNumErrors() > 0 ? EDataValidationResult::Invalid : EDataValidationResult::Valid;
}
#endif
