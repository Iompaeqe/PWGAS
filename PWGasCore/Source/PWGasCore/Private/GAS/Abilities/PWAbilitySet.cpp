// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "GAS/Abilities/PWAbilitySet.h"
#include "GAS/Abilities/PWAbilityRegistry.h"
#include "Misc/DataValidation.h"

#if WITH_EDITOR
EDataValidationResult UPWAbilitySet::IsDataValid(FDataValidationContext& Context) const
{
	for (const FPWAbilitySetEntry& Entry : Abilities)
		if (Entry.AbilityTag.IsValid() && !UPWAbilityRegistry::IsInAnyRegistry(Entry.AbilityTag))
			Context.AddWarning(FText::FromString(FString::Printf(TEXT("Ability Tag [%s] is in no ability registry."), *Entry.AbilityTag.ToString())));

	return EDataValidationResult::Valid;
}
#endif
