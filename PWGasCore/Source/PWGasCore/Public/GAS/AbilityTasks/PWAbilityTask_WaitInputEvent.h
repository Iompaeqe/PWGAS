// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "GAS/Data/GASCoreEnums.h"
#include "PWAbilityTask_WaitInputEvent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPWInputEventDelegate, const FGameplayTag&, InputTag);

/**
 * Waits for a simple gameplay input event (Pressed/Released) with a specific input tag.
 * The event is sent from the AbilitySystemComponent via SendInputEvent.
 */
UCLASS()
class PWGASCORE_API UPWAbilityTask_WaitInputEvent : public UAbilityTask
{
	GENERATED_BODY()

public:
	/** Called when the matching input event is received */
	UPROPERTY(BlueprintAssignable)
	FPWInputEventDelegate EventReceived;

	/** Waits for an input event with given tag and type (Pressed or Released). */
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta=(GameplayTagFilter = "Input", DisplayName="Wait Input Event", HidePin="OwningAbility", DefaultToSelf="OwningAbility", BlueprintInternalUseOnly="true"))
	static UPWAbilityTask_WaitInputEvent* WaitInputEvent(UGameplayAbility* OwningAbility, FGameplayTag InputTag, EPWInputEventType EventType = EPWInputEventType::Pressed);

	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

protected:
	void OnGameplayEventReceived(const FGameplayEventData* Payload);

	UGameplayAbility* ownerability;
	FDelegateHandle EventHandle;
	
	/** Tag to listen for (e.g. Input.Mouse.Left) */
	UPROPERTY()
	FGameplayTag WatchedInputTag;

	/** Whether we’re waiting for Pressed or Released */
	UPROPERTY()
	EPWInputEventType WatchedEventType = EPWInputEventType::Pressed;
};