// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "GAS/Abilities/PWAbilitySet.h"
#include "GAS/Abilities/PWAbilityInfo.h"
#include "Misc/DataValidation.h"
#include "Settings/PWGASCoreSettings.h"

#if WITH_EDITOR
EDataValidationResult UPWAbilitySet::IsDataValid(FDataValidationContext& Context) const
{
	const FString Registry = UPWGASCoreSettings::Get()->GetAbilityRegistry(FGameplayTag()).GetAssetName();
	for (const FPWAbilitySetEntry& Entry : Abilities)
		if (Entry.AbilityTag.IsValid() && !UPWGASCoreSettings::Get()->GetAbilityByTag(Entry.AbilityTag).IsValid())
			Context.AddWarning(FText::FromString(FString::Printf(TEXT("Ability Tag [%s] is not in %s."), *Entry.AbilityTag.ToString(), Registry.IsEmpty() ? TEXT("the ability registry") : *Registry)));

	return EDataValidationResult::Valid;
}
#endif
