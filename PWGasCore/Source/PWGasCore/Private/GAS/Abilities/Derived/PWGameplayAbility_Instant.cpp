// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "GAS/Abilities/Derived/PWGameplayAbility_Instant.h"
#include "GAS/Abilities/Modules/ActionModules/PWActionModule.h"
#include "PWGASCoreLogChannels.h"

void UPWGameplayAbility_Instant::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (GIsEditor)
	{
		UE_LOG(LogPWGASCoreAbility, Verbose, TEXT("[%s] Instant Ability Activated."), *GetNameSafe(this));
	}
	
	if (bAutoCommit && !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const bool bHasGraph = bHasBlueprintActivate || bHasBlueprintActivateFromEvent;
	if (!bHasGraph && ActionModule)
	{
		ActionModule->OnActionFinished.RemoveAll(this);
		ActionModule->OnActionFinished.AddUObject(this, &ThisClass::OnActionFinished);
	}

	bActivating = true;
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	bActivating = false;

	if (!bHasGraph && (!ActionModule || !ActionModule->IsActionPending()))
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPWGameplayAbility_Instant::OnActionFinished()
{
	if (!bActivating && IsActive())
		K2_EndAbility();
}