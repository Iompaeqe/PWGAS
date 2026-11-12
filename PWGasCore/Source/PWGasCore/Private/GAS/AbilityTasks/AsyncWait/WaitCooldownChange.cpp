// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/AbilityTasks/AsyncWait/WaitCooldownChange.h"

#include "AbilitySystemComponent.h"

UWaitCooldownChange* UWaitCooldownChange::WaitForCooldownChange(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTag& InCooldownTag)
{
	UWaitCooldownChange* Task = NewObject<UWaitCooldownChange>();
	Task->ASC = AbilitySystemComponent;
	Task->CooldownTag = InCooldownTag;

	if (!IsValid(Task->ASC) || !InCooldownTag.IsValid())
	{
		Task->EndTask();
		return nullptr;
	}

	AbilitySystemComponent->RegisterGameplayTagEvent(Task->CooldownTag).AddUObject(Task, &ThisClass::CooldownTagChanged);
	AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(Task, &ThisClass::CooldownEffectApplied);
	
	
	return Task;
}

void UWaitCooldownChange::EndTask() 
{
	if (!IsValid(ASC)) return;
		
	ASC->RegisterGameplayTagEvent(CooldownTag).RemoveAll(this);

	SetReadyToDestroy();
	MarkAsGarbage();
}

void UWaitCooldownChange::CooldownTagChanged(FGameplayTag GameplayTag, int NewCount) const
{
	if (NewCount == 0)
	{
		CooldownEnded.Broadcast(0.f, 1.f);
	}
}

void UWaitCooldownChange::CooldownEffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle) const
{
	FGameplayTagContainer AssetTags;
	Spec.GetAllAssetTags(AssetTags);
	FGameplayTagContainer GrantedTags;
	Spec.GetAllGrantedTags(GrantedTags);

	if (AssetTags.HasTagExact(CooldownTag) || GrantedTags.HasTagExact(CooldownTag))
	{
		const FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTag.GetSingleTagContainer());
		if(TArray<float> TimesRemaining = AbilitySystemComponent->GetActiveEffectsTimeRemaining(Query); TimesRemaining.Num() > 0)
		{
			float TimeRemaining = TimesRemaining[0];
			for (int i = 0; i < TimesRemaining.Num(); i++)
			{
				if (TimesRemaining[i] > TimeRemaining)
				{
					TimeRemaining = TimesRemaining[i];
				}
			}

			TArray<float> Cooldowns = AbilitySystemComponent->GetActiveEffectsDuration(Query);
			if(Cooldowns.Num() > 0)
			{
				float Cooldown = Cooldowns[0];
				for (int i = 0; i < Cooldowns.Num(); i++)
				{
					if (Cooldowns[i] > Cooldown)
					{
						Cooldown = Cooldowns[i];
					}
				}
				if (TimeRemaining > 0.01)
					CooldownStarted.Broadcast(TimeRemaining, Cooldown);
			}
		}
	}
}

