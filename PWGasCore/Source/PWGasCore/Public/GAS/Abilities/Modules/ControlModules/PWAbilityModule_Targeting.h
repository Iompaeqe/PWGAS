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
	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly)
	TObjectPtr<UPWTargetingSource> TargetingSource;
	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly)
	TObjectPtr<UPWTargetResolver> TargetResolver;

	UFUNCTION(BlueprintCallable, Category="Module")
	bool ComputeTarget(FPWTargetingResult& Out);
	UFUNCTION(BlueprintCallable, Category="Module")
	void MakeTargetingResultFromTargetData(FGameplayAbilityTargetDataHandle DataHandle, FPWTargetingResult& Out);

	
	void InitializePolicies() const;
	void FinalizeTargetingResult(FPWTargetingResult& InOut) const;

	virtual void GetRequiredDataModules_Implementation(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules) override;
};
