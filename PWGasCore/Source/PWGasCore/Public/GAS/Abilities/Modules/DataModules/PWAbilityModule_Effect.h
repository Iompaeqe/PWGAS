// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWDataModule.h"
#include "Ability/PWSpellParams.h"
#include "PWAbilityModule_Effect.generated.h"

struct FPWTargetingResult;
class UGameplayEffect;
class UAbilitySystemComponent;

USTRUCT(BlueprintType)
struct FPWEffectEntry
{
	GENERATED_BODY()

	// Gameplay effect applied to the target.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> EffectClass = nullptr;
	
	// Stacks applied at once.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StackCount = 1;
};

USTRUCT(BlueprintType)
struct FPWEffectGroup
{
	GENERATED_BODY()

	// Relations that receive these effects; anyone outside the mask is skipped.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Bitmask, BitmaskEnum="/Script/PWSharedTypes.EPWRelationMask"))
	int32 TargetMask = (int32)EPWRelationMask::All;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPWEffectEntry> Effects;
};

UCLASS(BlueprintType)
class PWGASCORE_API UPWEffectSpellParams : public UPWSpellParamsExtension
{
	GENERATED_BODY()

public:
	// Effects carried to the spawned actor for it to apply on hit, each group with its own mask.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPWEffectGroup> Groups;
	UFUNCTION(BlueprintCallable)
	void ApplyEffectsToTarget(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC, int32 AbilityLevel = 1) const;
};

UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWAbilityModule_Effect : public UPWDataModule
{
	GENERATED_BODY()

public:
	// Effects this ability applies, grouped by who receives them: for example damage for enemies and neutrals, burn for enemies only.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FPWEffectGroup> EffectGroups;

	UPROPERTY()
	int32 TargetMask_DEPRECATED = (int32)EPWRelationMask::All;
	UPROPERTY()
	TArray<FPWEffectEntry> Effects_DEPRECATED;

	TMap<TWeakObjectPtr<UAbilitySystemComponent>, TArray<FActiveGameplayEffectHandle>> ActiveEffectHandles;
	
	void ApplyEffects(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC);
	void RemoveEffects(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC, bool bInfiniteOnly = false);

	// Uses the target's ability system component when it has one, otherwise its IPWAffectable receiver.
	void ApplyEffectsToActor(UAbilitySystemComponent* SourceASC, AActor* Target);
	void RemoveEffectsFromActor(UAbilitySystemComponent* SourceASC, AActor* Target, bool bInfiniteOnly = false);

	// Every relation at least one group applies to.
	int32 GetCombinedMask() const;

	virtual void ExtendParams(UPWSpellParams* Params) const override;
	virtual void PostLoad() override;
};
