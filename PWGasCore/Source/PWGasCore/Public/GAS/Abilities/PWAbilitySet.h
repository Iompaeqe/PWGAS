// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PWAbilitySet.generated.h"

USTRUCT(BlueprintType)
struct FPWAbilitySetEntry
{
	GENERATED_BODY()

	// Input the ability is bound to when granted; leave empty for no binding.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability", meta=(Categories="Input"))
	FGameplayTag InputTag;
	// Tag of the ability to grant, as listed in the ability registry.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability", meta=(Categories="Ability.Skill"))
	FGameplayTag AbilityTag;
};

UCLASS(BlueprintType)
class PWGASCORE_API UPWAbilitySet : public UDataAsset
{
	GENERATED_BODY()

public:
	// Abilities granted together, for example a character's starting kit.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	TArray<FPWAbilitySetEntry> Abilities;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
