// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PWGasFunctions.generated.h"

enum class EGameplayEffectDurationType : uint8;
struct FGameplayAttribute;
class UPWAbilitySystemComponent;
class UPWSpellParams;

USTRUCT(BlueprintType)
struct FPWEffectModifierValue
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite)
	EGameplayEffectDurationType DurationType = EGameplayEffectDurationType::Instant;
	UPROPERTY(BlueprintReadWrite)
	float Duration = 0.0f;
	UPROPERTY(BlueprintReadWrite)
	float Period = 0.0f;
	UPROPERTY(BlueprintReadWrite)
	float Value = 0.0f;
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> EffectClass = nullptr;
};

UCLASS()
class PWGASCORE_API UPWGasFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static bool ApplyEffects(UPWAbilitySystemComponent* ASC, const UPWSpellParams* SpellParams);
	// With a target, only the effect groups whose mask includes it are read.
	static TArray<FPWEffectModifierValue> GetModifierValuesFromEffects(const UPWSpellParams* SpellParams, const FGameplayAttribute& Attribute, const AActor* Target = nullptr);
};
