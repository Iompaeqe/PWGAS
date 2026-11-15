// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWDataModule.h"
#include "ScalableFloat.h"
#include "PWAbilityModule_Radius.generated.h"

// Area radius by ability level, shared by targeting, previews and spawn patterns.
// Graphs change it for one cast with Set Radius; every activation goes back to the configured radius.
UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced, meta=(DisplayName="PWM Radius"))
class PWGASCORE_API UPWAbilityModule_Radius : public UPWDataModule
{
	GENERATED_BODY()

public:
	// The configured radius by ability level. Every activation starts from it.
	UPROPERTY(EditAnywhere, Category="Radius")
	FScalableFloat Radius = 400.f;

	// The radius in use at the ability's level: what this cast set, otherwise the configured one.
	UFUNCTION(BlueprintPure, Category="Module")
	float GetRadius() const;

	// The configured radius at the ability's level, whatever this cast set.
	UFUNCTION(BlueprintPure, Category="Module")
	float GetBaseRadius() const;

	// The radius in use, read at any level.
	UFUNCTION(BlueprintPure, Category="Module")
	float GetRadiusAtLevel(int32 Level) const;

	// The configured radius at any level.
	UFUNCTION(BlueprintPure, Category="Module")
	float GetBaseRadiusAtLevel(int32 Level) const;

	// Uses this radius at every level for the rest of the cast. The next activation goes back to the configured one.
	UFUNCTION(BlueprintCallable, Category="Module")
	void SetRadius(float NewRadius);

	// Uses this value and curve for the rest of the cast, so it still scales with level. The next activation goes back to the configured one.
	UFUNCTION(BlueprintCallable, Category="Module")
	void SetRadiusCurve(const FScalableFloat& NewRadius);

	virtual void ResetCurrentValues() override;
	// Draws the radius as a sphere and a ground circle at Center, labelled with what uses it. Only with this module's Show Debug on.
	void DebugDrawAt(const FVector& Center, const FString& UsedFor, float Duration = PWModuleDebug::Duration) const;
	virtual void OnAbilityActivated_Implementation() override;

	virtual void ExtendParams(UPWSpellParams* Params) const override;
	virtual bool GetValue(FName Key, float& Out) const override;

private:
	UPROPERTY(Transient)
	FPWModuleValue CurrentRadius;
};
