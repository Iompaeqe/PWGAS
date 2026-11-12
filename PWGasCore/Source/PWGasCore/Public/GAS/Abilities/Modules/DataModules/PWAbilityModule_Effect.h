// copyright mkurtt96

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> EffectClass = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StackCount = 1;
};

UCLASS(BlueprintType)
class PWGASCORE_API UEffectSpellParamsExtension : public USpellParamsExtension
{
	GENERATED_BODY()

public:
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Bitmask, BitmaskEnum="/Script/PWSharedTypes.ERelationMask"))
	int32 TargetMask = (int32)ERelationMask::All;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FEffectEntry> Effects;

	TMap<TWeakObjectPtr<UAbilitySystemComponent>, TArray<FActiveGameplayEffectHandle>> ActiveEffectHandles;
	
	void ApplyEffects(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC);
	void RemoveEffects(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC, bool bInfiniteOnly = false);

	virtual void ExtendParams(USpellParams* Params) const override;
};
