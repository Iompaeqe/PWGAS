// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "Targeting/PWTeamTargetFilter.h"
#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Components/MeshComponent.h"
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
	// Abilities saved before tag filters stored a number: Self 1, Ally 2, Enemy 4, Neutral 8. Things with no team used to count as Neutral, so that flag becomes Neutral plus Unaffiliated.
	static FPWTeamTargetFilter FilterFromLegacyMask(int32 Mask);

	// With a target, only the effect groups whose filter accepts it are read.
	static TArray<FPWEffectModifierValue> GetModifierValuesFromEffects(const UPWSpellParams* SpellParams, const FGameplayAttribute& Attribute, const AActor* Target = nullptr);

	// The actor's mesh that has this socket, skeletal or static. Skeletal meshes are checked first, so a character's body wins over what it carries.
	UFUNCTION(BlueprintPure, Category="PW|Sockets")
	static UMeshComponent* FindSocketMesh(const AActor* Actor, FName Socket);
};
