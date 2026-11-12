// Copyright (c) 2025 Metehan Goksel Kurtulan (Iompaeqe)

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PWAbilityInfo.generated.h"

class UGameplayAbility;

USTRUCT(BlueprintType)
struct FPWAbilityInfo
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	FGameplayTag Tag = FGameplayTag();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	TSubclassOf<UGameplayAbility> AbilityClass = nullptr;
	
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentLevel = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	int32 MaxLevel = 1;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	FText Name = FText();
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (MultiLine=true), Category = "UI")
	FText Description = FText();
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (MultiLine=true), Category = "UI")
	FText NextDescription = FText();
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TObjectPtr<const UMaterialInstance> IconMaterial = nullptr;
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag InputTag = FGameplayTag();
	UPROPERTY(BlueprintReadOnly)
	bool bToBeRemoved = false;
	
public:
	bool IsValid() const
	{
		return Tag.IsValid();
	}
};
