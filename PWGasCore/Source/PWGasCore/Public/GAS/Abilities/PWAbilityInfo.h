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
	// The ability's identity tag, used to grant it and look it up.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	FGameplayTag Tag = FGameplayTag();

	// The gameplay ability class granted for this tag.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	TSubclassOf<UGameplayAbility> AbilityClass = nullptr;
	
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentLevel = 0;
	// Highest level the ability can be upgraded to.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	int32 MaxLevel = 1;
	
	// Display name shown in UI.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	FText Name = FText();
	// Description of the current level; {_Level0}, {_Level1}, {_CD0} and {_CD1} are replaced with level and cooldown values.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (MultiLine=true), Category = "UI")
	FText Description = FText();
	// Description shown when previewing the next level, with the same placeholders as Description.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (MultiLine=true), Category = "UI")
	FText NextDescription = FText();
	// Material used as the ability's icon in UI.
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
