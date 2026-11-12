// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "PWControlModule.h"
#include "PWAbilityModule_Targeting.generated.h"

struct FGameplayAbilityTargetDataHandle;
class UPWTargetingSource;
class UPWTargetResolver;
struct FPWTargetingResult;
UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWAbilityModule_Targeting : public UPWControlModule
{
	GENERATED_BODY()
	
public:
	// Where the ability aims: the cursor, the screen center or the caster; empty targets the caster.
	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly)
	TObjectPtr<UPWTargetingSource> TargetingSource;
	// Moved to the Target Resolver module. Old abilities are converted when they load.
	UPROPERTY(Instanced, meta=(DeprecatedProperty, DeprecationMessage="Use the Target Resolver module."))
	TObjectPtr<UPWTargetResolver> TargetResolver_DEPRECATED;

	UFUNCTION(BlueprintCallable, Category="Module")
	bool ComputeTarget(FPWTargetingResult& Out);
	UFUNCTION(BlueprintCallable, Category="Module")
	void MakeTargetingResultFromTargetData(FGameplayAbilityTargetDataHandle DataHandle, FPWTargetingResult& Out);

	
	void InitializePolicies() const;
	void FinalizeTargetingResult(FPWTargetingResult& InOut) const;

	virtual void GetRequiredDataModules_Implementation(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules) override;
};
