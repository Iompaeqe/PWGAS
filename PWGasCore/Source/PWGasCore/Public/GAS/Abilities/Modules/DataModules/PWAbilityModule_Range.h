// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWDataModule.h"
#include "ScalableFloat.h"
#include "Targeting/Types/PWTargetPolicyBase.h"
#include "PWAbilityModule_Range.generated.h"

struct FPWTargetingResult;

UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWAbilityModule_Range : public UPWDataModule
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly)
	TObjectPtr<UPWRangePolicy> RangePolicy;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FScalableFloat Range = 1200.f;

	UFUNCTION(BlueprintPure, Category="Module")
	float GetRangeValue() const;

	UFUNCTION(BlueprintCallable, Category="Module")
	void ApplyRangePolicy(const AActor* Origin, FPWTargetingResult& InOut) const;

	virtual void Initialize(UPWModularGameplayAbility* InOwner) override;
	virtual void ExtendParams(USpellParams* Params) const override;
	
};