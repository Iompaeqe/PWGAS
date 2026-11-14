// Copyright Epic Games, Inc. All Rights Reserved.

#include "GAS/AbilityTasks/PWAbilityTask_PlayMontageAddTagAndWait.h"

#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemLog.h"
#include "AbilitySystemGlobals.h"
#include "GAS/Tags/GASCoreTags.h"
#include "Engine/World.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PWAbilityTask_PlayMontageAddTagAndWait)

UPWAbilityTask_PlayMontageAddTagAndWait::UPWAbilityTask_PlayMontageAddTagAndWait(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), AnimRootMotionTranslationScale(0), StartTimeSeconds(0)
{
	Rate = 1.f;
	bStopWhenAbilityEnds = true;
}

void UPWAbilityTask_PlayMontageAddTagAndWait::OnMontageBlendingOut(UAnimMontage* Montage, const bool bInterrupted)
{
	if (Ability && Ability->GetCurrentMontage() == MontageToPlay)
	{
		if (Montage == MontageToPlay)
		{
			if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
			{
				ASC->ClearAnimatingAbility(Ability);
			}

			// Reset AnimRootMotionTranslationScale
			ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
			if (Character && (Character->GetLocalRole() == ROLE_Authority ||
				(Character->GetLocalRole() == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
			{
				Character->SetAnimRootMotionTranslationScale(1.f);
			}
		}
	}

	FGameplayAbilitySpec* AbilitySpec = Ability->GetCurrentAbilitySpec();

	if (bInterrupted)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			RemoveTags();
			OnInterrupted.Broadcast();
		}
	}
	else
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			RemoveTags();
			OnBlendOut.Broadcast();
		}
	}
}

void UPWAbilityTask_PlayMontageAddTagAndWait::OnMontageInterrupted()
{
	if (StopPlayingMontage())
	{
		// Let the BP handle the interrupt as well
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			RemoveTags();
			OnInterrupted.Broadcast();
		}
	}
}

void UPWAbilityTask_PlayMontageAddTagAndWait::OnMontageEnded(UAnimMontage* Montage, const bool bInterrupted)
{
	if (!bInterrupted)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			RemoveTags();
			OnCompleted.Broadcast();
		}
	}

	EndTask();
}

UPWAbilityTask_PlayMontageAddTagAndWait* UPWAbilityTask_PlayMontageAddTagAndWait::CreatePlayMontageAddTagAndWaitProxy(UGameplayAbility* OwningAbility, FGameplayTagContainer TagsToAdd,
                                                                                                                  FName TaskInstanceName, UAnimMontage* MontageToPlay, float Rate, FName StartSection, bool bStopWhenAbilityEnds, float AnimRootMotionTranslationScale, float StartTimeSeconds, float NoMontageDelay)
{
	UAbilitySystemGlobals::NonShipping_ApplyGlobalAbilityScaler_Rate(Rate);

	UPWAbilityTask_PlayMontageAddTagAndWait* MyObj = NewAbilityTask<UPWAbilityTask_PlayMontageAddTagAndWait>(OwningAbility, TaskInstanceName);
	MyObj->MontageToPlay = MontageToPlay;
	MyObj->TagsToAdd = TagsToAdd;
	MyObj->Rate = Rate;
	MyObj->StartSection = StartSection;
	MyObj->AnimRootMotionTranslationScale = AnimRootMotionTranslationScale;
	MyObj->bStopWhenAbilityEnds = bStopWhenAbilityEnds;
	MyObj->StartTimeSeconds = StartTimeSeconds;
	MyObj->NoMontageDelay = NoMontageDelay;

	return MyObj;
}

void UPWAbilityTask_PlayMontageAddTagAndWait::Activate()
{
	if (Ability == nullptr)
	{
		return;
	}

	bool bPlayedMontage = false;

	if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
	{
		const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
		UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			if (ASC->PlayMontage(Ability, Ability->GetCurrentActivationInfo(), MontageToPlay, Rate, StartSection, StartTimeSeconds) > 0.f)
			{
				// Playing a montage could potentially fire off a callback into game code which could kill this ability! Early out if we are  pending kill.
				if (ShouldBroadcastAbilityTaskDelegates() == false)
				{
					return;
				}

				InterruptedHandle = Ability->OnGameplayAbilityCancelled.AddUObject(this, &UPWAbilityTask_PlayMontageAddTagAndWait::OnMontageInterrupted);

				BlendingOutDelegate.BindUObject(this, &UPWAbilityTask_PlayMontageAddTagAndWait::OnMontageBlendingOut);
				AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, MontageToPlay);

				MontageEndedDelegate.BindUObject(this, &UPWAbilityTask_PlayMontageAddTagAndWait::OnMontageEnded);
				AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageToPlay);

				ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
				if (Character && (Character->GetLocalRole() == ROLE_Authority ||
					(Character->GetLocalRole() == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
				{
					Character->SetAnimRootMotionTranslationScale(AnimRootMotionTranslationScale);
				}

				bPlayedMontage = true;

				AddTags();
			}
		}

		// No montage, or nothing to play it on, such as a static mesh: the cast still happens after the delay.
		if (!bPlayedMontage)
			StartWithoutMontage();
	}
	else
	{
		ABILITY_LOG(Warning, TEXT("UAbilityTask_PlayMontageAndWait called on invalid AbilitySystemComponent"));
		if (ShouldBroadcastAbilityTaskDelegates())
			OnCancelled.Broadcast();
	}

	SetWaitingOnAvatar();
}

void UPWAbilityTask_PlayMontageAddTagAndWait::AddTags()
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!ASC || bTagsAdded)
		return;
	ASC->AddLooseGameplayTags(TagsToAdd);
	bTagsAdded = true;
}

void UPWAbilityTask_PlayMontageAddTagAndWait::RemoveTags()
{
	// Several paths end a montage; the tags must only come off once, or a tag something else also holds would drop with them.
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!ASC || !bTagsAdded)
		return;
	ASC->RemoveLooseGameplayTags(TagsToAdd);
	bTagsAdded = false;
}

void UPWAbilityTask_PlayMontageAddTagAndWait::StartWithoutMontage()
{
	if (!ShouldBroadcastAbilityTaskDelegates())
		return;

	ABILITY_LOG(Log, TEXT("%s plays without a montage; the trigger comes after %.2fs."), *Ability->GetName(), NoMontageDelay);
	AddTags();

	FTimerManager& Timers = GetWorld()->GetTimerManager();
	if (NoMontageDelay > 0.f)
		Timers.SetTimer(NoMontageTimer, this, &ThisClass::FinishWithoutMontage, NoMontageDelay, false);
	else
		NoMontageTimer = Timers.SetTimerForNextTick(this, &ThisClass::FinishWithoutMontage);
}

void UPWAbilityTask_PlayMontageAddTagAndWait::FinishWithoutMontage()
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (ASC && ShouldBroadcastAbilityTaskDelegates())
	{
		// What the montage's notify would have sent.
		FGameplayEventData Payload;
		Payload.EventTag = PWTags::MontageEvent::Trigger;
		Payload.Instigator = GetAvatarActor();
		{
			FScopedPredictionWindow PredictionWindow(ASC, true);
			ASC->HandleGameplayEvent(Payload.EventTag, &Payload);
		}

		RemoveTags();
		if (ShouldBroadcastAbilityTaskDelegates())
			OnCompleted.Broadcast();
	}
	EndTask();
}

void UPWAbilityTask_PlayMontageAddTagAndWait::ExternalCancel()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		RemoveTags();
		OnCancelled.Broadcast();
	}
	Super::ExternalCancel();
}

void UPWAbilityTask_PlayMontageAddTagAndWait::OnDestroy(const bool AbilityEnded)
{
	// Note: Clearing montage end delegate isn't necessary since its not a multicast and will be cleared when the next montage plays.
	// (If we are destroyed, it will detect this and not do anything)

	// This delegate, however, should be cleared as it is a multicast
	if (Ability)
	{
		Ability->OnGameplayAbilityCancelled.Remove(InterruptedHandle);
		if (AbilityEnded && bStopWhenAbilityEnds)
		{
			StopPlayingMontage();
		}
	}
	if (UWorld* World = GetWorld())
		World->GetTimerManager().ClearTimer(NoMontageTimer);

	// Stopping the montage above unbinds its end callbacks, so the tags come off here instead.
	RemoveTags();
	Super::OnDestroy(AbilityEnded);
}

bool UPWAbilityTask_PlayMontageAddTagAndWait::StopPlayingMontage() const
{
	if (Ability == nullptr)
	{
		return false;
	}

	const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
	if (ActorInfo == nullptr)
	{
		return false;
	}

	UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
	if (AnimInstance == nullptr)
	{
		return false;
	}

	// Check if the montage is still playing
	// The ability would have been interrupted, in which case we should automatically stop the montage
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (ASC && Ability)
	{
		if (ASC->GetAnimatingAbility() == Ability
			&& ASC->GetCurrentMontage() == MontageToPlay)
		{
			// Unbind delegates so they don't get called as well
			FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(MontageToPlay);
			if (MontageInstance)
			{
				MontageInstance->OnMontageBlendingOutStarted.Unbind();
				MontageInstance->OnMontageEnded.Unbind();
			}

			ASC->CurrentMontageStop();
			return true;
		}
	}

	return false;
}

FString UPWAbilityTask_PlayMontageAddTagAndWait::GetDebugString() const
{
	UAnimMontage* PlayingMontage = nullptr;
	if (Ability)
	{
		const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
		UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();

		if (AnimInstance != nullptr)
		{
			PlayingMontage = AnimInstance->Montage_IsActive(MontageToPlay) ? ToRawPtr(MontageToPlay) : AnimInstance->GetCurrentActiveMontage();
		}
	}

	return FString::Printf(TEXT("PlayMontageAndWait. MontageToPlay: %s  (Currently Playing): %s"), *GetNameSafe(MontageToPlay), *GetNameSafe(PlayingMontage));
}

void UPWAbilityTask_PlayMontageAddTagAndWait::StopMontage()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		RemoveTags();
		OnCancelled.Broadcast();
	}

	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	ASC->CurrentMontageStop();

	ExternalCancel();
}
