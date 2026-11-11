// copyright mkurtt96

#pragma once

#include "CoreMinimal.h"
#include "PWActionModule.h"
#include "PWAbilityModule_InstantEffect.generated.h"

struct FGameplayAbilityTargetDataHandle;
/**
 * 
 */
UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced)
class PWGASCORE_API UPWAbilityModule_InstantEffect : public UPWActionModule
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Module")
	virtual void ApplyEffects();
	UFUNCTION(BlueprintCallable, Category="Module")
	virtual void ApplyEffectsAtLocation(const FVector& Location);
	UFUNCTION(BlueprintCallable, Category="Module")
	virtual void ApplyEffectsAtSocket(FName SocketName);
	UFUNCTION(BlueprintCallable, Category="Module")
	virtual void RemoveEffects();
	
	virtual void GetRequiredDataModules_Implementation(TArray<TSubclassOf<UPWDataModule>>& OutRequiredModules) override;
	
	
protected:

	UPROPERTY()
	TObjectPtr<class UPWAbilityTask_TargetFromSource> ActiveTargetTask;
	/** Called by ability task when targeting completes (client and server). */
	UFUNCTION()
	void OnTargetDataReady(const FGameplayAbilityTargetDataHandle& DataHandle, const struct FPWTargetingResult& Result);
	
	virtual void ApplyEffectsToTargets(const FPWTargetingResult& Result);

	TArray<AActor*> Targets;
};
