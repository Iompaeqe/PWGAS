// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/AbilityTasks/PWAbilityTask_WaitInputEvent.h"

#include "AbilitySystemComponent.h"
#include "GAS/Tags/GASCoreTags.h"

UPWAbilityTask_WaitInputEvent* UPWAbilityTask_WaitInputEvent::WaitInputEvent(
	UGameplayAbility* OwningAbility, FGameplayTag InputTag, EPWInputEventType EventType)
{
	UPWAbilityTask_WaitInputEvent* Task = NewAbilityTask<UPWAbilityTask_WaitInputEvent>(OwningAbility);
	Task->WatchedInputTag = InputTag;
	Task->WatchedEventType = EventType;
	Task->ownerability = OwningAbility;
	return Task;
}

void UPWAbilityTask_WaitInputEvent::Activate()
{
	if (!Ability) return;

	const FGameplayAbilityActorInfo* Info = Ability->GetCurrentActorInfo();
	if (!Info || !Info->AbilitySystemComponent.IsValid()) return;

	UAbilitySystemComponent* ASC = Info->AbilitySystemComponent.Get();

	EventHandle = ASC->GenericGameplayEventCallbacks.FindOrAdd(PWTags::Event::Input).AddUObject(this, &ThisClass::OnGameplayEventReceived);
}

void UPWAbilityTask_WaitInputEvent::OnDestroy(bool bInOwnerFinished)
{
	if (AbilitySystemComponent.IsValid())
		if (FGameplayEventMulticastDelegate* Delegate = AbilitySystemComponent->GenericGameplayEventCallbacks.Find(PWTags::Event::Input))
			Delegate->Remove(EventHandle);

	Super::OnDestroy(bInOwnerFinished);
}

void UPWAbilityTask_WaitInputEvent::OnGameplayEventReceived(const FGameplayEventData* Payload)
{
	if (!Payload)
		return;

	const FGameplayTagContainer& Tags = Payload->InstigatorTags;
	if (!Tags.HasTagExact(WatchedInputTag))
		return;

	const bool bPressed = Payload->EventMagnitude > 0.5f;
	const EPWInputEventType ReceivedType = bPressed ? EPWInputEventType::Pressed : EPWInputEventType::Released;

	if (ReceivedType != WatchedEventType)
		return;

	EventReceived.Broadcast(WatchedInputTag);
	EndTask();
 }