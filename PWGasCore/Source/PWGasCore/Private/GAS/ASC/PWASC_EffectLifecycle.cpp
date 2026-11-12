// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)


#include "GAS/ASC/PWASC_EffectLifecycle.h"

#include "GameplayEffect.h"
#include "GAS/ASC/PWAbilitySystemComponent.h"
#include "GAS/Effects/PWGameplayEffect_BuffTag.h"
#include "GAS/EffectContext/PWGameplayEffectContext.h"

void FPWASC_EffectLifecycle::ApplyBuffTag(const FGameplayTag BuffTag) const
{
	FGameplayEffectContextHandle EffectContext = ASC.MakeEffectContext();
	EffectContext.AddSourceObject(&ASC);

	FGameplayEffectSpec Spec(GetDefault<UPWGameplayEffect_BuffTag>(), EffectContext, 1);
	Spec.DynamicGrantedTags.AddTag(BuffTag);
	ASC.ApplyGameplayEffectSpecToSelf(Spec);
}

void FPWASC_EffectLifecycle::RemoveEffectsByTag(const FGameplayTagContainer& TagsToRemove) const
{
	FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(TagsToRemove);

	TArray<FActiveGameplayEffectHandle> HandlesToRemove = ASC.GetActiveEffects(Query);

	for (const FActiveGameplayEffectHandle& Handle : HandlesToRemove)
	{
		ASC.RemoveActiveGameplayEffect(Handle);
	}
}
