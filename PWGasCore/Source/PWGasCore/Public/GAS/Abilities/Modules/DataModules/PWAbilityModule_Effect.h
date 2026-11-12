// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWDataModule.h"
#include "Ability/SpellParams.h"
#include "PWAbilityModule_Effect.generated.h"

struct FPWTargetingResult;
class UGameplayEffect;
class UAbilitySystemComponent;

USTRUCT(BlueprintType)
struct FEffectEntry
{
	GENERATED_BODY()

	// Gameplay effect applied to the target.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> EffectClass = nullptr;
	
	// Stacks applied at once.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StackCount = 1;
};

UCLASS(BlueprintType)
class PWGASCORE_API UEffectSpellParamsExtension : public USpellParamsExtension
{
	GENERATED_BODY()

public:
	// Effects carried to the spawned actor for it to apply on hit.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FEffectEntry> Effects;
	UFUNCTION(BlueprintCallable)
	void ApplyEffectsToTarget(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC, int32 AbilityLevel = 1) const;
};

UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWAbilityModule_Effect : public UPWDataModule
{
	GENERATED_BODY()

public:
	// Relations that receive the effects; anyone outside the mask is skipped.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Bitmask, BitmaskEnum="/Script/PWSharedTypes.ERelationMask"))
	int32 TargetMask = (int32)ERelationMask::All;
	// Effects this ability applies to its targets.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FEffectEntry> Effects;

	TMap<TWeakObjectPtr<UAbilitySystemComponent>, TArray<FActiveGameplayEffectHandle>> ActiveEffectHandles;
	
	void ApplyEffects(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC);
	void RemoveEffects(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC, bool bInfiniteOnly = false);

	// Uses the target's ability system component when it has one, otherwise its IAffectable receiver.
	void ApplyEffectsToActor(UAbilitySystemComponent* SourceASC, AActor* Target);
	void RemoveEffectsFromActor(UAbilitySystemComponent* SourceASC, AActor* Target, bool bInfiniteOnly = false);

	virtual void ExtendParams(USpellParams* Params) const override;
};
