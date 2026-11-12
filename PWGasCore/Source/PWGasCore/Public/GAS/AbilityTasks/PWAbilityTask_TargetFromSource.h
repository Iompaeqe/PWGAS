// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "PWAbilityTask_TargetFromSource.generated.h"

class UPWAbilityModule_Targeting;
class UPWModularGameplayAbility;
class UPWTargetingSource;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPWTargetResultDelegate, const FGameplayAbilityTargetDataHandle&, DataHandle, const FPWTargetingResult&, TargetingResult);

UCLASS()
class PWGASCORE_API UPWAbilityTask_TargetFromSource : public UAbilityTask
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FPWTargetResultDelegate ValidData;

	UFUNCTION(BlueprintCallable, meta=(HidePin="OwningAbility", DefaultToSelf="OwningAbility", BlueprintInternalUseOnly="true"))
	static UPWAbilityTask_TargetFromSource* TargetFromSource(UPWModularGameplayAbility* OwningAbility);

protected:
	UPROPERTY()
	UPWAbilityModule_Targeting* TargetingModule;

	virtual void Activate() override;

	void SendTargetDataLocal();
	void OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle,
	                                    FGameplayTag ActivationTag) const;
};
