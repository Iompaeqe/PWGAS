// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Ability/PWSpellParamsInterface.h"
#include "GameFramework/Actor.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "GAS/Abilities/Derived/PWGameplayAbility_Confirmed.h"
#include "GAS/Abilities/Derived/PWGameplayAbility_Instant.h"
#include "GAS/Abilities/Derived/PWGameplayAbility_Passive.h"
#include "PWGasCore/Public/Targeting/Types/PWTargetPolicyBase.h"
#include "GAS/Abilities/Derived/PWGameplayAbility_Channeling.h"
#include "GAS/Abilities/Derived/PWGameplayAbility_Charged.h"
#include "PWAttributeSetBase.h"
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
	mutable float LastRadius = 0.f;

	virtual void Resolve_Implementation(const AActor* Actor, const FPWTargetingResult& Target, TArray<AActor*>& OutActors) const override
	{
		LastLocation = Target.Location;
		LastRadius = GetFloat(PWParamKeys::Radius);
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

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestConfirmedAbility : public UPWGameplayAbility_Confirmed
{
	GENERATED_BODY()

public:
	UPWTestConfirmedAbility();
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestChargedAbility : public UPWGameplayAbility_Charged
{
	GENERATED_BODY()

public:
	UPWTestChargedAbility();
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestChannelingAbility : public UPWGameplayAbility_Channeling
{
	GENERATED_BODY()

public:
	UPWTestChannelingAbility();
};

UCLASS(NotBlueprintable)
class UPWTestAttributeSet : public UPWAttributeSetBase
{
	GENERATED_BODY()

public:
	UPWTestAttributeSet();

	UPROPERTY(ReplicatedUsing = OnRep_Health) FGameplayAttributeData Health; UFUNCTION() void OnRep_Health(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, Health, OldValue); } ATTRIBUTE_ACCESSORS(ThisClass, Health);
	UPROPERTY(ReplicatedUsing = OnRep_HealthMax) FGameplayAttributeData HealthMax; UFUNCTION() void OnRep_HealthMax(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(ThisClass, HealthMax, OldValue); } ATTRIBUTE_ACCESSORS(ThisClass, HealthMax);
	UPROPERTY() FGameplayAttributeData Cooldown; ATTRIBUTE_ACCESSORS(ThisClass, Cooldown);
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestPeriodicDamageEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UPWTestPeriodicDamageEffect();
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestAuraAbility : public UPWModularGameplayAbility
{
	GENERATED_BODY()

public:
	UPWTestAuraAbility();
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestInstantHitEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UPWTestInstantHitEffect();
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestTimedHitEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UPWTestTimedHitEffect();
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestLeveledAbility : public UPWModularGameplayAbility
{
	GENERATED_BODY()

public:
	UPWTestLeveledAbility();
};

UCLASS(NotBlueprintable, HideDropdown)
class UPWTestInterruptibleAbility : public UPWGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UPWTestInterruptibleAbility();
};
