// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "PWGameplayAbility_Instant.generated.h"


UCLASS(Blueprintable, BlueprintType)
class PWGASCORE_API UPWGameplayAbility_Instant : public UPWModularGameplayAbility
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability|Commit")
	bool bAutoCommit = true;

protected:
	void OnActionFinished();

	bool bActivating = false;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
