// Copyright Epic Games, Inc. All Rights Reserved.

#include "GAS/AbilityTasks/PWAbilityTask_PlayMontageAddTagAndWait.h"

#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemLog.h"
#include "AbilitySystemGlobals.h"
#include "GAS/Tags/GASCoreTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(PWAbilityTask_PlayMontageAddTagAndWait)

UPWAbilityTask_PlayMontageAddTagAndWait::UPWAbilityTask_PlayMontageAddTagAndWait(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), AnimRootMotionTranslationScale(0), StartTimeSeconds(0)
{
	Rate = 1.f;
	bStopWhenAbilityEnds = true;
}

void UPWAbilityTask_PlayMontageAddTagAndWait::OnMontageBlendingOut(UAnimMontage* Montage, const bool bInterrupted) const
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
			AbilitySystemComponent.Get()->RemoveLooseGameplayTags(TagsToAdd);
			OnInterrupted.Broadcast();
		}
	}
	else
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			AbilitySystemComponent.Get()->RemoveLooseGameplayTags(TagsToAdd);
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
			AbilitySystemComponent.Get()->RemoveLooseGameplayTags(TagsToAdd);
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
			AbilitySystemComponent.Get()->RemoveLooseGameplayTags(TagsToAdd);
			OnCompleted.Broadcast();
		}
	}

	EndTask();
}

UPWAbilityTask_PlayMontageAddTagAndWait* UPWAbilityTask_PlayMontageAddTagAndWait::CreatePlayMontageAddTagAndWaitProxy(UGameplayAbility* OwningAbility, FGameplayTagContainer TagsToAdd,
                                                                                                                  FName TaskInstanceName, UAnimMontage* MontageToPlay, float Rate, FName StartSection, bool bStopWhenAbilityEnds, float AnimRootMotionTranslationScale, float StartTimeSeconds)
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

				ASC->AddLooseGameplayTags(TagsToAdd);
			}
		}
		else
		{
			ABILITY_LOG(Warning, TEXT("UAbilityTask_PlayMontageAndWait call to PlayMontage failed!"));
		}
	}
	else
	{
		ABILITY_LOG(Warning, TEXT("UAbilityTask_PlayMontageAndWait called on invalid AbilitySystemComponent"));
	}

	if (!bPlayedMontage)
	{
		ABILITY_LOG(Warning, TEXT("UAbilityTask_PlayMontageAndWait called in Ability %s failed to play montage %s; Task Instance Name %s."), *Ability->GetName(), *GetNameSafe(MontageToPlay), *InstanceName.ToString());
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			AbilitySystemComponent.Get()->RemoveLooseGameplayTags(TagsToAdd);
			OnCancelled.Broadcast();
		}
	}

	SetWaitingOnAvatar();
}

void UPWAbilityTask_PlayMontageAddTagAndWait::ExternalCancel()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		AbilitySystemComponent.Get()->RemoveLooseGameplayTags(TagsToAdd);
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
		AbilitySystemComponent.Get()->RemoveLooseGameplayTags(TagsToAdd);
		OnCancelled.Broadcast();
	}

	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	ASC->CurrentMontageStop();

	ExternalCancel();
}
