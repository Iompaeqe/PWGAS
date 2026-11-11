// copyright mkurtt96

#include "PWTestTypes.h"

UPWTestEffect::UPWTestEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
}

UPWTestPlainAbility::UPWTestPlainAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

UPWTestModularAbility::UPWTestModularAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

APWTestSpellActor::APWTestSpellActor()
{
	SetRootComponent(CreateDefaultSubobject<USceneComponent>("Root"));
}

UPWTestInstantEffectAbility::UPWTestInstantEffectAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	ActionModule = CreateDefaultSubobject<UPWAbilityModule_InstantEffect>("InstantEffect");

	UPWAbilityModule_Effect* Effect = CreateDefaultSubobject<UPWAbilityModule_Effect>("Effect");
	Effect->Effects.Add({UPWTestEffect::StaticClass(), 1});
	DataModules.Add(Effect);
}
