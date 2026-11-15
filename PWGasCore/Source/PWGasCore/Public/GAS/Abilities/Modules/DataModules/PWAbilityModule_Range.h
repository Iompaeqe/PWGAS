// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWDataModule.h"
#include "ScalableFloat.h"
#include "Targeting/Types/PWTargetPolicyBase.h"
#include "PWAbilityModule_Range.generated.h"

struct FPWTargetingResult;

// Maximum cast distance by ability level, and how a target beyond it is handled.
// Graphs change the range for one cast with Set Range; every activation goes back to the configured range.
UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced, meta=(DisplayName="PWM Range"))
class PWGASCORE_API UPWAbilityModule_Range : public UPWDataModule
{
	GENERATED_BODY()

public:
	// How a target point beyond the range is handled, for example clamped to max range.
	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category="Range")
	TObjectPtr<UPWRangePolicy> RangePolicy;

	// The configured maximum cast distance by ability level. Every activation starts from it.
	UPROPERTY(EditDefaultsOnly, Category="Range")
	FScalableFloat Range = 1200.f;

	// The range in use at the ability's level: what this cast set, otherwise the configured one.
	UFUNCTION(BlueprintPure, Category="Module")
	float GetRange() const;

	// The configured range at the ability's level, whatever this cast set.
	UFUNCTION(BlueprintPure, Category="Module")
	float GetBaseRange() const;

	// The range in use, read at any level.
	UFUNCTION(BlueprintPure, Category="Module")
	float GetRangeAtLevel(int32 Level) const;

	// The configured range at any level.
	UFUNCTION(BlueprintPure, Category="Module")
	float GetBaseRangeAtLevel(int32 Level) const;

	// Uses this range at every level for the rest of the cast. The next activation goes back to the configured one.
	UFUNCTION(BlueprintCallable, Category="Module")
	void SetRange(float NewRange);

	// Uses this value and curve for the rest of the cast, so it still scales with level. The next activation goes back to the configured one.
	UFUNCTION(BlueprintCallable, Category="Module")
	void SetRangeCurve(const FScalableFloat& NewRange);

	virtual void ResetCurrentValues() override;

	UFUNCTION(BlueprintCallable, Category="Module")
	void ApplyRangePolicy(const AActor* Origin, FPWTargetingResult& InOut) const;
	// Draws the range as a ground circle around Origin, labelled with what uses it. Only with this module's Show Debug on.
	void DebugDrawFrom(const FVector& Origin, const FString& UsedFor, float Duration = PWModuleDebug::Duration) const;
	virtual void OnAbilityActivated_Implementation() override;

	virtual void Initialize(UPWModularGameplayAbility* InOwner) override;
	virtual void ExtendParams(UPWSpellParams* Params) const override;
	virtual bool GetValue(FName Key, float& Out) const override;

private:
	UPROPERTY(Transient)
	FPWModuleValue CurrentRange;
};