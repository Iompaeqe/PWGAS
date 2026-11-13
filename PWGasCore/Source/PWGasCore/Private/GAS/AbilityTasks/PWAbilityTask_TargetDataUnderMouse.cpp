// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/AbilityTasks/PWAbilityTask_TargetDataUnderMouse.h"

#include "AbilitySystemComponent.h"

class UPWAbilityTask_TargetDataUnderMouse;

UPWAbilityTask_TargetDataUnderMouse* UPWAbilityTask_TargetDataUnderMouse::CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility, ECollisionChannel TraceChannel)
{
	UPWAbilityTask_TargetDataUnderMouse* MyObj = NewAbilityTask<UPWAbilityTask_TargetDataUnderMouse>(OwningAbility);
	MyObj->TraceChannel = TraceChannel;
	return MyObj;
}


void UPWAbilityTask_TargetDataUnderMouse::Activate()
{
	const bool bIsLocal = Ability->GetCurrentActorInfo()->IsLocallyControlled();
	if (bIsLocal)
	{
		SendMouseCursorData();
	}
	else
	{
		FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
		FPredictionKey ActivationKey = GetActivationPredictionKey();
		AbilitySystemComponent.Get()->AbilityTargetDataSetDelegate(SpecHandle, ActivationKey).AddUObject(this, &ThisClass::OnTargetDataReplicatedCallback);
		if (! AbilitySystemComponent.Get()->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationKey))
		{
			SetWaitingOnRemotePlayerData();
		}
	}
}

void UPWAbilityTask_TargetDataUnderMouse::SendMouseCursorData()
{
	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get());
	
	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	FHitResult Hit;
	
	PC->GetHitResultUnderCursor(TraceChannel, false, Hit);
	
	if (!Hit.bBlockingHit)
	{
		FVector CamLoc;
		FRotator CamRot;
		PC->GetPlayerViewPoint(CamLoc, CamRot);

		const float MaxRange = 100000.f;
		Hit.ImpactPoint = CamLoc + CamRot.Vector() * MaxRange;
		Hit.ImpactNormal = CamRot.Vector() * -1.f;
		Hit.TraceEnd = Hit.ImpactPoint;
		Hit.bBlockingHit = false;
	}
	
	FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit();
	Data->HitResult = Hit;

	FGameplayAbilityTargetDataHandle DataHandle;
	DataHandle.Add(Data);

	AbilitySystemComponent->ServerSetReplicatedTargetData(
		GetAbilitySpecHandle(),
		GetActivationPredictionKey(),
		DataHandle,
		FGameplayTag(),
		AbilitySystemComponent->ScopedPredictionKey);

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
	
}

void UPWAbilityTask_TargetDataUnderMouse::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag)
{
	AbilitySystemComponent.Get()->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(),GetActivationPredictionKey());
	
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}
