// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GAS/Abilities/PWAbilityRegistry.h"
#include "PWGASCoreSettings.generated.h"

UCLASS(config=Game, defaultconfig)
class PWGASCORE_API UPWGASCoreSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	static const UPWGASCoreSettings* Get(){ return GetDefault<UPWGASCoreSettings>(); }
	virtual FName GetCategoryName() const override;

	// The project's registry. Look abilities up with the registry's Find Ability instead, which also sees a world's own registry.
	UFUNCTION(BlueprintCallable)
	TSoftObjectPtr<UPWAbilityRegistry> GetAbilityRegistry(const FGameplayTag& AbilityTag) const;

protected:
	// Registry asset that maps ability tags to ability classes for the whole game. A game mode can bring its own as a PW Ability Registry Provider.
	UPROPERTY(EditAnywhere, config, Category="GAS Core Registry")
	TSoftObjectPtr<UPWAbilityRegistry> AbilityRegistry;

	
};
