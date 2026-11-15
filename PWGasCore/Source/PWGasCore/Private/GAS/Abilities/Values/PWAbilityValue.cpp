// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#include "GAS/Abilities/Values/PWAbilityValue.h"

#include "Engine/BlueprintGeneratedClass.h"
#include "GAS/Abilities/PWModularGameplayAbility.h"
#include "GAS/Abilities/Modules/ActionModules/PWAbilityModule_Projectile.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Radius.h"
#include "GAS/Abilities/Modules/DataModules/PWAbilityModule_Range.h"

namespace PWAbilityValueText
{
	FString Inner(const UPWAbilityValue* Value, UPWModularGameplayAbility* Ability)
	{
		return Value ? Value->Describe(Ability) : TEXT("nothing");
	}
}

FString UPWAbilityValue::Describe(UPWModularGameplayAbility* Ability) const
{
	return FString::Printf(TEXT("%s %g"), *GetClass()->GetDisplayNameText().ToString(), Evaluate(Ability));
}

FString UPWAbilityValue_Constant::Describe(UPWModularGameplayAbility* Ability) const
{
	return FString::Printf(TEXT("%g"), Value);
}

float UPWAbilityValue_RadiusModule::Evaluate_Implementation(UPWModularGameplayAbility* Ability) const
{
	const UPWAbilityModule_Radius* Module = Ability ? Ability->GetRadiusModule() : nullptr;
	return Module ? Module->GetRadius() : 0.f;
}

float UPWAbilityValue_RangeModule::Evaluate_Implementation(UPWModularGameplayAbility* Ability) const
{
	const UPWAbilityModule_Range* Module = Ability ? Ability->GetRangeModule() : nullptr;
	return Module ? Module->GetRange() : 0.f;
}

float UPWAbilityValue_ProjectileSpeed::Evaluate_Implementation(UPWModularGameplayAbility* Ability) const
{
	const UPWAbilityModule_Projectile* Module = Ability ? Ability->GetProjectileModule() : nullptr;
	return Module ? Module->GetProjectileSpeed() : 0.f;
}

float UPWAbilityValue_AbilityLevel::Evaluate_Implementation(UPWModularGameplayAbility* Ability) const
{
	return Ability ? Ability->GetAbilityLevel() : 1.f;
}

float UPWAbilityValue_Variable::Evaluate_Implementation(UPWModularGameplayAbility* Ability) const
{
	const FNumericProperty* Property = Ability ? FindFProperty<FNumericProperty>(Ability->GetClass(), Variable) : nullptr;
	if (!Property)
		return 0.f;
	const void* Address = Property->ContainerPtrToValuePtr<void>(Ability);
	return Property->IsFloatingPoint() ? Property->GetFloatingPointPropertyValue(Address) : Property->GetSignedIntPropertyValue(Address);
}

FString UPWAbilityValue_Variable::Describe(UPWModularGameplayAbility* Ability) const
{
	const bool bFound = Ability && FindFProperty<FNumericProperty>(Ability->GetClass(), Variable);
	return FString::Printf(TEXT("%s %g%s"), *Variable.ToString(), Evaluate(Ability), bFound ? TEXT("") : TEXT(" (no such number variable)"));
}

TArray<FString> UPWAbilityValue_Variable::GetVariableNames() const
{
	TArray<FString> Names;
	const UPWModularGameplayAbility* Ability = GetTypedOuter<UPWModularGameplayAbility>();
	if (!Ability)
		return Names;
	for (TFieldIterator<FNumericProperty> It(Ability->GetClass()); It; ++It)
	{
		if (Cast<UBlueprintGeneratedClass>(It->GetOwnerClass()))
			Names.Add(It->GetName());
	}
	return Names;
}

UPWAbilityValue_Math::UPWAbilityValue_Math()
{
	A = CreateDefaultSubobject<UPWAbilityValue_Constant>(TEXT("A"));
	B = CreateDefaultSubobject<UPWAbilityValue_Constant>(TEXT("B"));
}

void UPWAbilityValue_Math::SetStartValue(const float Value)
{
	for (UPWAbilityValue* Side : {A.Get(), B.Get()})
	{
		if (UPWAbilityValue_Constant* Constant = Cast<UPWAbilityValue_Constant>(Side))
			Constant->Value = Value;
	}
}

float UPWAbilityValue_Math::Evaluate_Implementation(UPWModularGameplayAbility* Ability) const
{
	return Combine(A ? A->Evaluate(Ability) : 0.f, B ? B->Evaluate(Ability) : 0.f);
}

FString UPWAbilityValue_Math::Describe(UPWModularGameplayAbility* Ability) const
{
	return FString::Printf(TEXT("(%s %s %s)"), *PWAbilityValueText::Inner(A, Ability), GetSymbol(), *PWAbilityValueText::Inner(B, Ability));
}
