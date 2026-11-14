// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestWorld.h"
#include "PWTestTypes.h"
#include "GAS/AbilityTasks/PWAbilityTask_PlayMontageAddTagAndWait.h"
#include "GAS/Tags/GASCoreTags.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

static UPWAbilityTask_PlayMontageAddTagAndWait* PlayWithoutMontage(UGameplayAbility* Ability, const float Delay)
{
	UPWAbilityTask_PlayMontageAddTagAndWait* Task = UPWAbilityTask_PlayMontageAddTagAndWait::CreatePlayMontageAddTagAndWaitProxy(
		Ability, FGameplayTagContainer(PWTags::ASC::State::Casting::Root), NAME_None, nullptr, 1.f, NAME_None, true, 1.f, 0.f, Delay);
	Task->ReadyForActivation();
	return Task;
}

// The tags other players get; the accessor is protected, so it is read like the replicated property it is.
static bool IsSentToOthers(const UAbilitySystemComponent* ASC, const FGameplayTag& Tag)
{
	const FStructProperty* Property = CastField<FStructProperty>(UAbilitySystemComponent::StaticClass()->FindPropertyByName(TEXT("MinimalReplicationTags")));
	const FMinimalReplicationTagCountMap* Tags = Property ? Property->ContainerPtrToValuePtr<FMinimalReplicationTagCountMap>(ASC) : nullptr;
	return Tags && Tags->TagMap.Contains(Tag);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNoMontageCastTest, "PW.Animation.Montage.NoMontage", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNoMontageCastTest::RunTest(const FString& Parameters)
{
	const FPWTestWorld Test;
	UPWTestModularAbility* Ability = Test.ActivateAbility<UPWTestModularAbility>();
	if (!TestTrue("Ability active", Ability && Ability->IsActive()))
		return false;

	int32 Triggers = 0;
	FGameplayEventMulticastDelegate& TriggerEvent = Test.ASC->GenericGameplayEventCallbacks.FindOrAdd(PWTags::MontageEvent::Trigger);
	const FDelegateHandle Handle = TriggerEvent.AddLambda([&Triggers](const FGameplayEventData*) { ++Triggers; });

	// Timers set before the timer manager has ticked this frame start counting at its next tick, as in a game frame.
	Test.TickTimers(0.f);
	PlayWithoutMontage(Ability, 0.5f);
	TestTrue("The casting tag is on although nothing plays", Test.ASC->HasMatchingGameplayTag(PWTags::ASC::State::Casting::Root));
	TestTrue("and is sent to the other players", IsSentToOthers(Test.ASC, PWTags::ASC::State::Casting::Root));

	Test.TickTimers(0.3f);
	TestEqual("No trigger before the delay", Triggers, 0);

	Test.TickTimers(0.3f);
	TestEqual("The task sends the trigger the montage would have", Triggers, 1);
	TestFalse("The casting tag comes off with it", Test.ASC->HasMatchingGameplayTag(PWTags::ASC::State::Casting::Root));
	TestFalse("for the other players too", IsSentToOthers(Test.ASC, PWTags::ASC::State::Casting::Root));

	PlayWithoutMontage(Ability, 0.5f);
	Ability->K2_CancelAbility();
	Test.TickTimers(1.f);
	TestEqual("A cast cancelled during the delay never fires", Triggers, 1);
	TestFalse("and leaves no casting tag behind", Test.ASC->HasMatchingGameplayTag(PWTags::ASC::State::Casting::Root));

	Test.ASC->GenericGameplayEventCallbacks.FindOrAdd(PWTags::MontageEvent::Trigger).Remove(Handle);
	return true;
}

#endif
