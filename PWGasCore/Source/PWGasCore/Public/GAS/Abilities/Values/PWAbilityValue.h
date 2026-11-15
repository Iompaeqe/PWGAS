// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PWAbilityValue.generated.h"

class UPWModularGameplayAbility;

// A number read from the ability, picked from a dropdown: a module's value, the ability level, a variable of the ability, a constant, or math on those.
// Blueprint subclasses add their own options by overriding Evaluate.
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories)
class PWGASCORE_API UPWAbilityValue : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category="Ability Value")
	float Evaluate(UPWModularGameplayAbility* Ability) const;

	// How the number was worked out, for debug prints, e.g. (Radius Module 400 + -65).
	virtual FString Describe(UPWModularGameplayAbility* Ability) const;

protected:
	virtual float Evaluate_Implementation(UPWModularGameplayAbility* Ability) const { return 0.f; }
};

// The number as typed.
UCLASS(meta=(DisplayName="Constant", InlineProperty="Value"))
class PWGASCORE_API UPWAbilityValue_Constant : public UPWAbilityValue
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Value")
	float Value = 0.f;

	virtual FString Describe(UPWModularGameplayAbility* Ability) const override;

protected:
	virtual float Evaluate_Implementation(UPWModularGameplayAbility* Ability) const override { return Value; }
};

// The radius in use from the ability's Radius module; 0 without one.
UCLASS(meta=(DisplayName="Radius Module", ValueBadge="MODULE", ValueLabel="Radius"))
class PWGASCORE_API UPWAbilityValue_RadiusModule : public UPWAbilityValue
{
	GENERATED_BODY()

protected:
	virtual float Evaluate_Implementation(UPWModularGameplayAbility* Ability) const override;
};

// The range in use from the ability's Range module; 0 without one.
UCLASS(meta=(DisplayName="Range Module", ValueBadge="MODULE", ValueLabel="Range"))
class PWGASCORE_API UPWAbilityValue_RangeModule : public UPWAbilityValue
{
	GENERATED_BODY()

protected:
	virtual float Evaluate_Implementation(UPWModularGameplayAbility* Ability) const override;
};

// The launch speed in use from the ability's Projectile module; 0 without one.
UCLASS(meta=(DisplayName="Projectile Module Speed", ValueBadge="MODULE", ValueLabel="Projectile Speed"))
class PWGASCORE_API UPWAbilityValue_ProjectileSpeed : public UPWAbilityValue
{
	GENERATED_BODY()

protected:
	virtual float Evaluate_Implementation(UPWModularGameplayAbility* Ability) const override;
};

UCLASS(meta=(DisplayName="Ability Level", ValueBadge="ABILITY", ValueLabel="Level"))
class PWGASCORE_API UPWAbilityValue_AbilityLevel : public UPWAbilityValue
{
	GENERATED_BODY()

protected:
	virtual float Evaluate_Implementation(UPWModularGameplayAbility* Ability) const override;
};

// A number variable of the ability, float or integer, read from the running ability. New variables show in the list after compiling.
UCLASS(meta=(DisplayName="Blueprint Variable", ValueBadge="VAR", InlineProperty="Variable"))
class PWGASCORE_API UPWAbilityValue_Variable : public UPWAbilityValue
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Value", meta=(GetOptions="GetVariableNames"))
	FName Variable;

	virtual FString Describe(UPWModularGameplayAbility* Ability) const override;

	UFUNCTION()
	TArray<FString> GetVariableNames() const;

protected:
	virtual float Evaluate_Implementation(UPWModularGameplayAbility* Ability) const override;
};

// Two values combined. Both start as constants that leave the other side as it is, so typing one number is enough.
UCLASS(Abstract)
class PWGASCORE_API UPWAbilityValue_Math : public UPWAbilityValue
{
	GENERATED_BODY()

public:
	UPWAbilityValue_Math();

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category="Value")
	TObjectPtr<UPWAbilityValue> A;

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category="Value")
	TObjectPtr<UPWAbilityValue> B;

	virtual FString Describe(UPWModularGameplayAbility* Ability) const override;

protected:
	virtual float Evaluate_Implementation(UPWModularGameplayAbility* Ability) const override;
	virtual float Combine(float InA, float InB) const { return 0.f; }
	virtual const TCHAR* GetSymbol() const { return TEXT("?"); }
	// Starts both sides at this constant.
	void SetStartValue(float Value);
};

UCLASS(meta=(DisplayName="Add"))
class PWGASCORE_API UPWAbilityValue_Add : public UPWAbilityValue_Math
{
	GENERATED_BODY()

protected:
	virtual float Combine(const float InA, const float InB) const override { return InA + InB; }
	virtual const TCHAR* GetSymbol() const override { return TEXT("+"); }
};

UCLASS(meta=(DisplayName="Subtract"))
class PWGASCORE_API UPWAbilityValue_Subtract : public UPWAbilityValue_Math
{
	GENERATED_BODY()

protected:
	virtual float Combine(const float InA, const float InB) const override { return InA - InB; }
	virtual const TCHAR* GetSymbol() const override { return TEXT("-"); }
};

UCLASS(meta=(DisplayName="Multiply"))
class PWGASCORE_API UPWAbilityValue_Multiply : public UPWAbilityValue_Math
{
	GENERATED_BODY()

public:
	UPWAbilityValue_Multiply() { SetStartValue(1.f); }

protected:
	virtual float Combine(const float InA, const float InB) const override { return InA * InB; }
	virtual const TCHAR* GetSymbol() const override { return TEXT("*"); }
};

// A divided by B; dividing by 0 gives 0.
UCLASS(meta=(DisplayName="Divide"))
class PWGASCORE_API UPWAbilityValue_Divide : public UPWAbilityValue_Math
{
	GENERATED_BODY()

public:
	UPWAbilityValue_Divide() { SetStartValue(1.f); }

protected:
	virtual float Combine(const float InA, const float InB) const override { return FMath::IsNearlyZero(InB) ? 0.f : InA / InB; }
	virtual const TCHAR* GetSymbol() const override { return TEXT("/"); }
};
