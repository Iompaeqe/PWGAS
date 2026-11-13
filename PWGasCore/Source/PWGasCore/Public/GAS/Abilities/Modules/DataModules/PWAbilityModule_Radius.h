// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWDataModule.h"
#include "ScalableFloat.h"
#include "PWAbilityModule_Radius.generated.h"

UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWAbilityModule_Radius : public UPWDataModule
{
	GENERATED_BODY()

public:
	// Area radius by ability level, shared by targeting, previews and spawn patterns.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FScalableFloat Radius = 400.f;

	UFUNCTION(BlueprintCallable, Category="Module")
	float GetRadiusValue() const;

	virtual void ExtendParams(UPWSpellParams* Params) const override;
	virtual bool GetValue(FName Key, float& Out) const override;
	virtual void Initialize(UPWModularGameplayAbility* InOwner) override;
};
