// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "Settings/PWGASCoreSettings.h"

#include "GAS/Abilities/PWAbilityInfo.h"

FName UPWGASCoreSettings::GetCategoryName() const
{
	return TEXT("Game"); 
}

FPWAbilityInfo UPWGASCoreSettings::GetAbilityByTag(const FGameplayTag& AbilityTag) const
{
	UPWAbilityRegistry* Abilities = AbilityRegistry.LoadSynchronous();
	return Abilities ? Abilities->GetAbilityByTag(AbilityTag) : FPWAbilityInfo();
}

TSoftObjectPtr<UPWAbilityRegistry> UPWGASCoreSettings::GetAbilityRegistry(const FGameplayTag& AbilityTag) const
{
	return AbilityRegistry;
}
