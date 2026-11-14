// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "Settings/PWGASCoreSettings.h"

FName UPWGASCoreSettings::GetCategoryName() const
{
	return TEXT("Game");
}

TSoftObjectPtr<UPWAbilityRegistry> UPWGASCoreSettings::GetAbilityRegistry(const FGameplayTag& AbilityTag) const
{
	return AbilityRegistry;
}
