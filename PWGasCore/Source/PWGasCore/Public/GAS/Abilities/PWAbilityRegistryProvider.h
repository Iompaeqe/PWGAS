// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PWAbilityRegistryProvider.generated.h"

class UPWAbilityRegistry;

UINTERFACE(BlueprintType, meta=(DisplayName="PW Ability Registry Provider"))
class UPWAbilityRegistryProvider : public UInterface
{
	GENERATED_BODY()
};

// A game mode that brings its own ability registry instead of the one in Project Settings.
// It is read from the game mode's class defaults, which every client reaches through the game state, so return a registry set in defaults.
class PWGASCORE_API IPWAbilityRegistryProvider
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Abilities")
	UPWAbilityRegistry* GetAbilityRegistry() const;
};
