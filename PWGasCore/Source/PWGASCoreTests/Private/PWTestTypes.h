// copyright mkurtt96

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Ability/PWSpellParamsInterface.h"
#include "GameFramework/Actor.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "GAS/Abilities/Derived/PWGameplayAbility_Instant.h"
#include "GAS/Abilities/Derived/PWGameplayAbility_Passive.h"
#include "PWGasCore/Public/Targeting/Types/PWTargetPolicyBase.h"
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
class UPWTestCostEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UPWTestCostEffect();
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestCostAbility : public UPWGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UPWTestCostAbility();
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
class UPWTestPassiveEffectAbility : public UPWGameplayAbility_Passive
{
	GENERATED_BODY()

public:
	UPWTestPassiveEffectAbility();
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestDurationEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UPWTestDurationEffect();
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestInstantBuffAbility : public UPWGameplayAbility_Instant
{
	GENERATED_BODY()

public:
	UPWTestInstantBuffAbility();
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestTargetResolver : public UPWTargetResolver
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<TObjectPtr<AActor>> Actors;

	mutable FVector LastLocation = FVector::ZeroVector;

	virtual void Resolve_Implementation(const AActor* Actor, const FPWTargetingResult& Target, TArray<AActor*>& OutActors) const override
	{
		LastLocation = Target.Location;
		OutActors.Append(Actors);
	}
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestInstantEffectAbility : public UPWModularGameplayAbility
{
	GENERATED_BODY()

public:
	UPWTestInstantEffectAbility();
};
