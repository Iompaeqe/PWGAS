// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "PWTestTypes.h"

#include "GAS/Tags/GASCoreTags.h"

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
	Effect->EffectGroups.AddDefaulted_GetRef().Effects.Add({UPWTestEffect::StaticClass(), 1});
	DataModules.Add(Effect);
}

UPWTestPassiveEffectAbility::UPWTestPassiveEffectAbility()
{
	UPWAbilityModule_Effect* Effect = CreateDefaultSubobject<UPWAbilityModule_Effect>("Effect");
	Effect->EffectGroups.AddDefaulted_GetRef().Effects.Add({UPWTestEffect::StaticClass(), 1});
	DataModules.Add(Effect);
}

UPWTestDurationEffect::UPWTestDurationEffect()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FScalableFloat(10.f);
}

UPWTestInstantBuffAbility::UPWTestInstantBuffAbility()
{
	UPWAbilityModule_InstantEffect* InstantEffect = CreateDefaultSubobject<UPWAbilityModule_InstantEffect>("InstantEffect");
	InstantEffect->bApplyOnActivated = true;
	ActionModule = InstantEffect;

	UPWAbilityModule_Effect* Effect = CreateDefaultSubobject<UPWAbilityModule_Effect>("Effect");
	Effect->EffectGroups.AddDefaulted_GetRef().Effects.Add({UPWTestEffect::StaticClass(), 1});
	Effect->EffectGroups.AddDefaulted_GetRef().Effects.Add({UPWTestDurationEffect::StaticClass(), 1});
	DataModules.Add(Effect);
}

UPWTestConfirmedAbility::UPWTestConfirmedAbility()
{
	ConfirmInputs.Add(EPWInputEventType::Pressed, FGameplayTagContainer(PWTags::Input::GlobalCancel));

	UPWAbilityModule_InstantEffect* InstantEffect = CreateDefaultSubobject<UPWAbilityModule_InstantEffect>("InstantEffect");
	InstantEffect->bApplyOnActivated = true;
	ActionModule = InstantEffect;

	UPWAbilityModule_Effect* Effect = CreateDefaultSubobject<UPWAbilityModule_Effect>("Effect");
	Effect->EffectGroups.AddDefaulted_GetRef().Effects.Add({UPWTestDurationEffect::StaticClass(), 1});
	DataModules.Add(Effect);
}

UPWTestChargedAbility::UPWTestChargedAbility()
{
	CooldownGameplayEffectClass = UPWTestDurationEffect::StaticClass();
	ChargeDuration = 1.f;
	ChargeTickInterval = 0.25f;
}

UPWTestChannelingAbility::UPWTestChannelingAbility()
{
	CooldownGameplayEffectClass = UPWTestDurationEffect::StaticClass();
	MaxChannelDuration = 1.f;
	ChannelTickInterval = 0.25f;
}

PW_REPLICATE_ATTRIBUTES(UPWTestAttributeSet)

UPWTestAttributeSet::UPWTestAttributeSet()
{
	AttributeTagRoot = PWTags::Ability::Root;
	AddClamped(GetHealthAttribute(), GetHealthMaxAttribute());
}

UPWTestPeriodicDamageEffect::UPWTestPeriodicDamageEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	Period = 0.25f;

	FGameplayModifierInfo Damage;
	Damage.Attribute = UPWTestAttributeSet::GetHealthAttribute();
	Damage.ModifierOp = EGameplayModOp::AddBase;
	Damage.ModifierMagnitude = FScalableFloat(-10.f);
	Modifiers.Add(Damage);
}

UPWTestAuraAbility::UPWTestAuraAbility()
{
	ActionModule = CreateDefaultSubobject<UPWAbilityModule_Aura>("Aura");
	DataModules.Add(CreateDefaultSubobject<UPWAbilityModule_Range>("Range"));

	UPWAbilityModule_Effect* Effect = CreateDefaultSubobject<UPWAbilityModule_Effect>("Effect");
	Effect->EffectGroups.AddDefaulted_GetRef().Effects.Add({UPWTestPeriodicDamageEffect::StaticClass(), 1});
	DataModules.Add(Effect);
}

static FGameplayModifierInfo MakeHealthHit(const float Amount)
{
	FGameplayModifierInfo Hit;
	Hit.Attribute = UPWTestAttributeSet::GetHealthAttribute();
	Hit.ModifierOp = EGameplayModOp::AddBase;
	Hit.ModifierMagnitude = FScalableFloat(Amount);
	return Hit;
}

UPWTestInstantHitEffect::UPWTestInstantHitEffect()
{
	Modifiers.Add(MakeHealthHit(25.f));
}

UPWTestTimedHitEffect::UPWTestTimedHitEffect()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FScalableFloat(1.f);
	Period = 0.25f;
	Modifiers.Add(MakeHealthHit(10.f));
}

UPWTestLeveledAbility::UPWTestLeveledAbility()
{
	AbilityTag = PWTags::Ability::Skill::Root;

	FGameplayTagContainer Tags = GetAssetTags();
	Tags.AddTag(AbilityTag);
	SetAssetTags(Tags);
}

UPWTestInterruptibleAbility::UPWTestInterruptibleAbility()
{
	CancellationTags.AddTag(PWTags::Ability::Activation::Failed);
}
