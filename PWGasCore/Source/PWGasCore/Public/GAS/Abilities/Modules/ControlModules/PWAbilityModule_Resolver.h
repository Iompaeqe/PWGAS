// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWControlModule.h"
#include "Targeting/Data/PWTargetingData.h"
#include "PWAbilityModule_Resolver.generated.h"

class UPWTargetResolver;

// Decides which actors become targets. With a Targeting module it resolves at the aimed point, without one it resolves around the caster.
UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced, meta=(DisplayName="Target Resolver"))
class PWGASCORE_API UPWAbilityModule_Resolver : public UPWControlModule
{
	GENERATED_BODY()

public:
	// How targets are gathered at the target point: a single actor, a sphere, or your own rule.
	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly)
	TObjectPtr<UPWTargetResolver> Resolver;

	// Fills the result's hit actors with the actors resolved at its location.
	UFUNCTION(BlueprintCallable, Category="Module")
	void ResolveTargets(UPARAM(ref) FPWTargetingResult& InOut) const;

	// Resolves around the caster, for abilities with no Targeting module.
	UFUNCTION(BlueprintCallable, Category="Module")
	FPWTargetingResult ResolveAroundCaster() const;

	virtual void GetRequiredDataModules_Implementation(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules) override;
};
