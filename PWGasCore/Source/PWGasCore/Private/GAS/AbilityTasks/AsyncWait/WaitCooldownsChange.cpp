// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/AbilityTasks/AsyncWait/WaitCooldownsChange.h"

#include "AbilitySystemComponent.h"
#include "GAS/Tags/GASCoreTags.h"
#include "GAS/Tags/PWTagFunctions.h"
#include "PWGASCoreLogChannels.h"

UWaitCooldownsChange* UWaitCooldownsChange::WaitForCooldownsChange(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTagContainer InCooldownTags)
{
	UWaitCooldownsChange* Task = NewObject<UWaitCooldownsChange>();
	Task->ASC = AbilitySystemComponent;
	Task->CooldownTags = InCooldownTags;

	if (!IsValid(Task->ASC) || InCooldownTags.IsEmpty())
	{
		Task->EndTask();
		return nullptr;
	}

	for (const FGameplayTag Tag : Task->CooldownTags)
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(Tag).AddUObject(Task, &ThisClass::CooldownTagChanged);
		AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(Task, &ThisClass::CooldownEffectApplied);
	}
	
	
	return Task;
}

void UWaitCooldownsChange::EndTask() 
{
	if (!IsValid(ASC)) return;

	for (const FGameplayTag Tag : CooldownTags)
		ASC->RegisterGameplayTagEvent(Tag).RemoveAll(this);

	SetReadyToDestroy();
	MarkAsGarbage();
}

void UWaitCooldownsChange::CooldownTagChanged(FGameplayTag GameplayTag, int NewCount) const
{
	if (NewCount == 0)
	{
		CooldownEnded.Broadcast(GameplayTag, 0.f, 1.f);
	}
}

void UWaitCooldownsChange::CooldownEffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle) const
{
	FGameplayTagContainer Tags;
	Spec.GetAllAssetTags(Tags);
	FGameplayTagContainer GrantedTags;
	Spec.GetAllGrantedTags(GrantedTags);
	Tags.AppendTags(GrantedTags);

	if (Tags.HasAnyExact(CooldownTags))
	{
		const FGameplayTag CooldownTag = UPWTagFunctions::FindTagWithSuffix(Tags, PWTags::Ability::Skill::GetCooldownSuffix());
		if (!CooldownTag.IsValid())
		{
			UE_LOG(LogPWGASCoreAbility, Warning, TEXT("No cooldown tag found in applied GE tags!"));
			return;
		}

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

			if(TArray<float> Cooldowns = AbilitySystemComponent->GetActiveEffectsDuration(Query); Cooldowns.Num() > 0)
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
					CooldownStarted.Broadcast(CooldownTag, TimeRemaining, Cooldown);
			}
		}
	}
}

