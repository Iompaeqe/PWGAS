// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "GAS/Effects/PWGameplayEffect_DynamicCooldown.h"

#include "GAS/Tags/GASCoreTags.h"

UPWGameplayEffect_DynamicCooldown::UPWGameplayEffect_DynamicCooldown()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	FSetByCallerFloat Duration;
	Duration.DataTag = PWTags::Ability::SetByCaller::Cooldown;
	DurationMagnitude = FGameplayEffectModifierMagnitude(Duration);
}
