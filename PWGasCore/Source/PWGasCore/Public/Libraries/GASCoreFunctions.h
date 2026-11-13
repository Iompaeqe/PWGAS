// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GASCoreFunctions.generated.h"

enum class EGameplayEffectDurationType : uint8;
struct FGameplayAttribute;
class UPWAbilitySystemComponent;
class USpellParams;

USTRUCT(BlueprintType)
struct FEffectModifierValue
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
class PWGASCORE_API UGASCoreFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static bool ApplyEffects(UPWAbilitySystemComponent* ASC, const USpellParams* SpellParams);
	// The object that takes effects for an actor without an ability system component, or null.
	static UObject* FindAffectable(AActor* Target);
	// With a target, only the effect groups whose mask includes it are read.
	static TArray<FEffectModifierValue> GetModifierValuesFromEffects(const USpellParams* SpellParams, const FGameplayAttribute& Attribute, const AActor* Target = nullptr);
};
