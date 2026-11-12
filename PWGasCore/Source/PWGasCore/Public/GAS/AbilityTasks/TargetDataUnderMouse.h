// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "TargetDataUnderMouse.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMouseTargetDataDelegate, const FGameplayAbilityTargetDataHandle&, DataHandle);

UCLASS()
class PWGASCORE_API UTargetDataUnderMouse : public UAbilityTask
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable)
	FMouseTargetDataDelegate ValidData;
	
	ECollisionChannel TraceChannel;

	UFUNCTION(BlueprintCallable, Category="Ability|Task", meta = (DiplayName = "TargetDataUnderMouse", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = true))
	static UTargetDataUnderMouse* CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility, ECollisionChannel TraceChannel = ECC_Visibility);

	

private:
	virtual void Activate() override;
	void SendMouseCursorData();
	void OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag);
	
};
