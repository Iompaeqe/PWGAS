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

UPWTestCostEffect::UPWTestCostEffect()
{
	FGameplayModifierInfo Cost;
	Cost.ModifierOp = EGameplayModOp::AddBase;
	Cost.ModifierMagnitude = FScalableFloat(-25.f);
	Modifiers.Add(Cost);
}

UPWTestCostAbility::UPWTestCostAbility()
{
	CostGameplayEffectClass = UPWTestCostEffect::StaticClass();
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

UPWTestPassiveEffectAbility::UPWTestPassiveEffectAbility()
{
	UPWAbilityModule_Effect* Effect = CreateDefaultSubobject<UPWAbilityModule_Effect>("Effect");
	Effect->Effects.Add({UPWTestEffect::StaticClass(), 1});
	DataModules.Add(Effect);
}
