// copyright mkurtt96

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Ability/PWSpellParamsInterface.h"
#include "GameFramework/Actor.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "PWTestTypes.generated.h"

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UPWTestEffect();
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestPlainAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UPWTestPlainAbility();
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestModularAbility : public UPWModularGameplayAbility
{
	GENERATED_BODY()

public:
	UPWTestModularAbility();
};

UCLASS(NotBlueprintable, NotPlaceable)
class APWTestSpellActor : public AActor, public IPWSpellParamsInterface
{
	GENERATED_BODY()

public:
	APWTestSpellActor();

	UPROPERTY()
	TObjectPtr<USpellParams> SpellParams;

	virtual void SetSpellParams_Implementation(USpellParams* Params) override { SpellParams = Params; }
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestInstantEffectAbility : public UPWModularGameplayAbility
{
	GENERATED_BODY()

public:
	UPWTestInstantEffectAbility();
};
