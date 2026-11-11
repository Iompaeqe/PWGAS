// copyright mkurtt96


#include "GAS/Abilities/Derived/PWGameplayAbility_Passive.h"

#include "AbilitySystemComponent.h"
#include "GAS/Tags/GASCoreTags.h"


UPWGameplayAbility_Passive::UPWGameplayAbility_Passive()
{
	FGameplayTagContainer GameplayTags = GetAssetTags();
	GameplayTags.AddTag(PWTags::Ability::Activation::OnGranted);
	GameplayTags.AddTag(PWTags::Ability::Deactivation::OnRemoved);
	SetAssetTags(GameplayTags);
}

void UPWGameplayAbility_Passive::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	bIsActivePassive = true;

	if (bEnablePassiveTick && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			PassiveTickTimer,
			this,
			&UPWGameplayAbility_Passive::HandlePassiveTick,
			PassiveTickInterval,
			true
		);
	}
}

void UPWGameplayAbility_Passive::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	bIsActivePassive = false;

	if (UWorld* World = GetWorld())
		World->GetTimerManager().ClearTimer(PassiveTickTimer);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPWGameplayAbility_Passive::HandlePassiveTick()
{
	OnPassiveTick();
}
